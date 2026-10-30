#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayer.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerBacking.hpp"
#include "Krystal.Booey/Rendering/RenderLayerCompositor.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Gfx.Tests/Golden.hpp"
#include "Krystal.Gfx.Tests/TestDevice.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.UI.Tests/ViewTestUtils.hpp"
#include "Krystal.UI/View.hpp"
#include <chrono>
#include <string>

namespace krys::ui::tests
{
  using namespace gfx::tests;

  namespace
  {
    /// @brief The average milliseconds a frame of a page spends in each stage.
    struct FrameCosts
    {
      double Update {0};
      double Paint {0};
      double Submit {0};
      /// @brief How many of the frames measured composited the page.
      int32 Composited {0};
      /// @brief How many of the frames measured repainted part of a layer.
      int32 Repainted {0};
    };

    /// @brief Whether any of the page's layers repainted since @p compositor's tracked repaints were reset.
    KRYS_NODISCARD bool LayersRepainted(View &view, boo::RenderLayerCompositor &compositor) noexcept
    {
      auto *root = view.Frame().View()->GetRenderViewForUpdate()->Layer()->Backing()->GetGraphicsLayer();
      while (root->Parent())
      {
        root = root->Parent();
      }
      auto text = root->LayerTreeAsText({boo::LayerTreeAsTextOptions::IncludeRepaintRects});
      compositor.ResetTrackedRepaintRects();
      return text.find(u8"(repaint rects") != utf8_string::npos;
    }

    /// @brief How a frame paints the page.
    enum class Painting : uint8
    {
      /// @brief As a host does: `Paint`, which repaints what changed.
      Damage,
      /// @brief The whole page, every frame: `Record` and `Replay`.
      Whole
    };

    /// @brief Runs @p frames frames (after ten to warm up) of the page at @p path at 60 Hz, as a host would:
    /// `Update`, then paint into a target, then submit the frame.
    /// @param scrollTo Where the page is scrolled to, after its first frames.
    /// @param scrollStep How far the page scrolls each measured frame, down and back up, as a wheel would.
    KRYS_NODISCARD FrameCosts Measure(const std::filesystem::path &path, uint32 width, uint32 height,
                                      Painting painting = Painting::Damage,
                                      Maybe<boo::IntPoint> scrollTo = null, int32 scrollStep = 0,
                                      int32 frames = 60)
    {
      using namespace std::chrono;
      static const auto fonts = View::AddFontDirectory(std::filesystem::path {KRYS_UI_TESTS_FONTS_DIR});
      (void)fonts;

      auto device = CreateTestDevice(gfx::DeviceDesc {}.TransientMemoryPerFrame);
      auto renderer = gfx::Renderer2D::Create(*device);
      REQUIRE(renderer.has_value());
      auto target =
        device->CreateTexture({.Format = gfx::TextureFormat::RGBA8Unorm,
                               .Width = width,
                               .Height = height,
                               .Usage = gfx::TextureUsage::RenderTarget | gfx::TextureUsage::Sampled,
                               .DebugName = u8"Benchmark"});
      REQUIRE(target.has_value());

      FrameCosts costs;
      {
        View view({static_cast<int32>(width), static_cast<int32>(height)});
        REQUIRE(view.LoadFile(path).has_value());
        WaitForLoad(view);
        auto time = boo::html::EventLoop::Current().Now() + 1s;
        const auto frame = duration_cast<MonotonicTime::duration>(microseconds {16'667});
        for (int32 i = 0; i < frames + 10; ++i)
        {
          time += frame;
          if (i == 2 && scrollTo)
          {
            view.ScrollTo(*scrollTo);
          }
          const auto start = steady_clock::now();
          if (i >= 10 && scrollStep)
          {
            // Down for 20 frames, then back up.
            const auto phase = (i - 10) % 40;
            const auto y = (phase < 20 ? phase : 40 - phase) * scrollStep;
            view.ScrollTo({0, (scrollTo ? scrollTo->Y() : 0) + y});
          }
          (void)view.Update(time);
          const auto updated = steady_clock::now();
          const bool needsPaint = view.NeedsPaint();
          device->BeginFrame();
          auto &commands = device->GetCommandBuffer();
          (*renderer)->Begin(commands, {.Texture = *target,
                                        .Width = width,
                                        .Height = height,
                                        .Format = gfx::TextureFormat::RGBA8Unorm,
                                        .ClearColour = Array<float, 4> {0.f, 0.f, 0.f, 0.f}});
          if (painting == Painting::Damage)
          {
            view.Paint(**renderer);
          }
          else
          {
            auto displayList = view.Record();
            view.Replay(*displayList, **renderer);
          }
          (*renderer)->End();
          const auto painted = steady_clock::now();
          device->Submit(commands);
          device->EndFrame();
          const auto submitted = steady_clock::now();
          auto &compositor = view.Frame().View()->GetRenderViewForUpdate()->Compositor();
          if (i == 9)
          {
            compositor.SetTracksRepaints(true);
            compositor.ResetTrackedRepaintRects();
          }
          if (i >= 10)
          {
            costs.Update += duration<double, std::milli>(updated - start).count() / frames;
            costs.Paint += duration<double, std::milli>(painted - updated).count() / frames;
            costs.Submit += duration<double, std::milli>(submitted - painted).count() / frames;
            costs.Composited += needsPaint ? 1 : 0;
            costs.Repainted += LayersRepainted(view, compositor) ? 1 : 0;
          }
        }
      }
      device->Destroy(*target);
      return costs;
    }

    void Report(const std::string &name, const FrameCosts &costs)
    {
      WARN(name << ": update " << costs.Update << " ms, paint " << costs.Paint << " ms, submit "
                << costs.Submit << " ms, total " << costs.Update + costs.Paint + costs.Submit
                << " ms a frame; composited " << costs.Composited << " frames, repainted layers in "
                << costs.Repainted);
    }
  }

  // Hidden: run it on its own (`[.benchmark]`), in Debug and Release, to compare frame costs.
  TEST_CASE("Benchmark: frame costs", "[UI][GPU][.benchmark]")
  {
    // The editor's page: cards, images, text, transitions on hover and animations that run every frame. At
    // its top, nothing on screen changes after it loads; at its bottom, its animations run.
    const auto editorPage = GoldenDirectory() / ".." / ".." / "Krystal.Editor" / "pages" / "index.html";
    const auto bottom = boo::IntPoint(0, 100'000);
    Report("editor page, top", Measure(editorPage, 1'280u, 800u));
    Report("editor page, animations", Measure(editorPage, 1'280u, 800u, Painting::Damage, bottom));
    Report("editor page, animations, all repainted",
           Measure(editorPage, 1'280u, 800u, Painting::Whole, bottom));
    // A still page: after its first paint, nothing changes.
    Report("editor page, scrolling", Measure(editorPage, 1'280u, 800u, Painting::Damage, null, 12));
    Report("still page", Measure(GoldenDirectory() / "scenes" / "text.html", 360u, 240u));
  }
}
