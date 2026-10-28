#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Animation/DocumentTimeline.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Gfx.Tests/Golden.hpp"
#include "Krystal.Gfx.Tests/TestDevice.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.UI/View.hpp"
#include <string>

namespace krys::ui::tests
{
  using namespace gfx::tests;

  /// @brief Paints a view into a target of its size the way a host does each frame (`View::Paint`, which
  /// composites the page's layers, repainting only what changed in them), and as a full repaint of the page
  /// (`Record` and `Replay`), to compare.
  class RepaintFixture
  {
    KRYS_NON_COPY_MOVABLE(RepaintFixture);

  private:
    UniquePtr<gfx::Device> _device;
    UniquePtr<gfx::Renderer2D> _renderer;
    gfx::TextureHandle _target;
    uint32 _width;
    uint32 _height;

  public:
    RepaintFixture(uint32 width, uint32 height)
        : _device(CreateTestDevice(gfx::DeviceDesc {}.TransientMemoryPerFrame)), _width(width),
          _height(height)
    {
      auto renderer = gfx::Renderer2D::Create(*_device);
      REQUIRE(renderer.has_value());
      _renderer = krys::move(*renderer);
      auto target =
        _device->CreateTexture({.Format = gfx::TextureFormat::RGBA8Unorm,
                                .Width = width,
                                .Height = height,
                                .Usage = gfx::TextureUsage::RenderTarget | gfx::TextureUsage::Sampled,
                                .DebugName = u8"Repaint"});
      REQUIRE(target.has_value());
      _target = *target;
    }

    ~RepaintFixture() noexcept
    {
      _device->Destroy(_target);
    }

    KRYS_NODISCARD gfx::Device &Device() const noexcept
    {
      return *_device;
    }

    /// @brief The page as the host shows it: its layers composited.
    KRYS_NODISCARD GoldenImage Painted(View &view)
    {
      return Draw([&] { view.Paint(*_renderer); });
    }

    /// @brief The page composited where the host draws it: `offset` into the target.
    KRYS_NODISCARD GoldenImage PaintedAt(View &view, Vec2f offset)
    {
      return Draw(
        [&]
        {
          _renderer->Translate(offset.x, offset.y);
          view.Paint(*_renderer);
        });
    }

    /// @brief The whole page painted again.
    KRYS_NODISCARD GoldenImage Repainted(View &view)
    {
      // Animations that run on layers don't update the page's style each frame: resolve it at the current
      // time, so the whole page paints what the layers show.
      if (auto *timeline = view.Document().ExistingTimeline())
      {
        for (const auto &animation : timeline->RelevantAnimations())
        {
          if (auto *effect = animation->GetKeyframeEffect())
          {
            effect->Invalidate();
          }
        }
      }
      (void)view.Document().UpdateLayout();
      auto displayList = view.Record();
      return Draw([&] { view.Replay(*displayList, *_renderer); });
    }

    /// @brief The page as the host shows it, after @p before records the host's own drawing into the
    /// frame's commands.
    KRYS_NODISCARD GoldenImage PaintedAfter(View &view, const Func<void(gfx::CommandBuffer &)> &before)
    {
      return Draw([&] { view.Paint(*_renderer); }, before);
    }

  private:
    template <typename TDraw>
    KRYS_NODISCARD GoldenImage Draw(TDraw &&draw, const Func<void(gfx::CommandBuffer &)> &before = {})
    {
      _device->BeginFrame();
      auto &commands = _device->GetCommandBuffer();
      if (before)
      {
        before(commands);
      }
      _renderer->Begin(commands, {.Texture = _target,
                                  .Width = _width,
                                  .Height = _height,
                                  .Format = gfx::TextureFormat::RGBA8Unorm,
                                  .ClearColour = Array<float, 4> {0.f, 0.f, 0.f, 0.f}});
      draw();
      _renderer->End();
      _device->Submit(commands);
      _device->EndFrame();

      GoldenImage image {.Width = _width, .Height = _height, .Pixels = {}};
      image.Pixels.resize(static_cast<size_t>(_width) * _height);
      _device->ReadTexture(
        _target, gfx::TextureRegion::Whole2D(_width, _height),
        Span<byte>(reinterpret_cast<byte *>(image.Pixels.data()), image.Pixels.size() * 4u));
      return image;
    }
  };

  /// @brief Checks the page the host shows after a change against the whole page painted again: no stale
  /// pixels.
  inline void CheckNoStalePixels(RepaintFixture &fixture, View &view, const std::string &step)
  {
    auto painted = fixture.Painted(view);
    auto repainted = fixture.Repainted(view);
    auto comparison = CompareImages(painted, repainted, 2u);
    INFO(step << ": largest difference " << comparison.LargestDifference << ", " << comparison.PixelsOver
              << " pixels over 2");
    if (comparison.PixelsOver > 0u)
    {
      (void)SavePng(GoldenOutputDirectory() / ("repaint." + step + ".painted.png"), painted);
      (void)SavePng(GoldenOutputDirectory() / ("repaint." + step + ".repainted.png"), repainted);
    }
    CHECK(comparison.PixelsOver == 0u);
  }
}
