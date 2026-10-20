#include "Krystal.Gfx.Tests/Golden.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Gfx.Tests/TestDevice.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.UI/View.hpp"
#include <string>

namespace krys::ui::tests
{
  using namespace gfx::tests;

  namespace
  {
    /// @brief The page's own golden is checked tightly: any change beyond rounding is a regression.
    constexpr uint32 OwnTolerance = 2u;
    /// @brief Chrome antialiases, blurs and dithers a little differently, so its golden is checked loosely.
    constexpr uint32 ChromeTolerance = 24u;

    /// @brief Paints the page golden `golden/scenes/<name>.html` through a `View` of its size, after @p setup
    /// (input, say) if given.
    KRYS_NODISCARD GoldenImage Render(gfx::Device &device, gfx::Renderer2D &renderer, const std::string &name,
                                      uint32 width, uint32 height, const Func<void(View &)> &setup)
    {
      // Liberation Sans, as the scenes name it.
      static const auto fonts = View::AddFontDirectory(std::filesystem::path {KRYS_UI_TESTS_FONTS_DIR});
      (void)fonts;
      View view({static_cast<int32>(width), static_cast<int32>(height)});
      REQUIRE(view.LoadFile(GoldenDirectory() / "scenes" / (name + ".html")).has_value());
      if (setup)
      {
        view.UpdateRendering();
        setup(view);
      }

      auto target =
        device.CreateTexture({.Format = gfx::TextureFormat::RGBA8Unorm,
                              .Width = width,
                              .Height = height,
                              .Usage = gfx::TextureUsage::RenderTarget | gfx::TextureUsage::Sampled,
                              .DebugName = u8"Golden"});
      REQUIRE(target.has_value());
      device.BeginFrame();
      auto &commands = device.GetCommandBuffer();
      renderer.Begin(commands, {.Texture = *target,
                                .Width = width,
                                .Height = height,
                                .Format = gfx::TextureFormat::RGBA8Unorm,
                                .ClearColour = Array<float, 4> {0.f, 0.f, 0.f, 0.f}});
      view.Paint(renderer);
      renderer.End();
      device.Submit(commands);
      device.EndFrame();

      GoldenImage image {.Width = width, .Height = height, .Pixels = {}};
      image.Pixels.resize(static_cast<size_t>(width) * height);
      device.ReadTexture(*target, gfx::TextureRegion::Whole2D(width, height),
                         Span<byte>(reinterpret_cast<byte *>(image.Pixels.data()), image.Pixels.size() * 4u));
      device.Destroy(*target);
      return image;
    }

    /// @brief Checks a page against its own golden (writing it when updating or missing) and against
    /// Chrome's, writing the actual image and the differences to the output directory either way.
    /// @param chromeReportOnly Whether Chrome's golden is only reported, not checked: where Chrome's
    /// rendering follows its own algorithm rather than the one ported (border image tiling).
    /// @param setup Run on the view before it's painted, if given.
    void CheckPage(const std::string &name, uint32 width, uint32 height, bool chromeReportOnly = false,
                   const Func<void(View &)> &setup = {})
    {
      // Pages draw in many small batches (a tile, a clip, a gradient each), so they get a full frame's
      // memory.
      auto device = CreateTestDevice(gfx::DeviceDesc {}.TransientMemoryPerFrame);
      auto renderer = gfx::Renderer2D::Create(*device);
      REQUIRE(renderer.has_value());
      const auto actual = Render(*device, **renderer, name, width, height, setup);
      const auto output = GoldenOutputDirectory();
      CHECK(SavePng(output / (name + ".actual.png"), actual));

      const auto ownPath = GoldenDirectory() / (name + ".png");
      const auto own = LoadPng(ownPath);
      if (UpdatingGoldens() || !own)
      {
        WARN("Writing the page golden for " << name << "; review it before committing.");
        CHECK(SavePng(ownPath, actual));
      }
      else
      {
        const auto comparison = CompareImages(actual, *own, OwnTolerance);
        INFO(name << " against its golden: largest difference " << comparison.LargestDifference << ", "
                  << comparison.PixelsOver << " pixels over " << OwnTolerance);
        CHECK(comparison.SizesMatch);
        if (comparison.PixelsOver > 0u)
        {
          SavePng(output / (name + ".diff.png"), DifferenceImage(actual, *own));
        }
        CHECK(comparison.PixelsOver == 0u);
      }

      const auto chrome = LoadPng(GoldenDirectory() / (name + ".chrome.png"));
      if (!chrome)
      {
        WARN("No Chrome golden for "
             << name << "; run tools/golden/capture_chrome.py --golden Krystal.UI.Tests/golden.");
        return;
      }
      const auto comparison = CompareImages(actual, *chrome, ChromeTolerance);
      SavePng(output / (name + ".chrome-diff.png"), DifferenceImage(actual, *chrome));
      WARN(name << " against Chrome: largest difference " << comparison.LargestDifference << ", mean "
                << comparison.MeanDifference << ", " << comparison.PixelsOver << " of "
                << comparison.PixelCount << " pixels over " << ChromeTolerance);
      CHECK(comparison.SizesMatch);
      if (chromeReportOnly)
      {
        return;
      }
      // Antialiased curves differ most, so a few pixels may differ by more; on average, images should be
      // within a step or so.
      CHECK(comparison.PixelsOver <= comparison.PixelCount / 200u);
      CHECK(comparison.MeanDifference < 1.5);
    }
  }

  TEST_CASE("Golden: backgrounds", "[UI][GPU][Golden]")
  {
    CheckPage("backgrounds", 360u, 240u);
  }

  TEST_CASE("Golden: layers", "[UI][GPU][Golden]")
  {
    CheckPage("layers", 360u, 240u);
  }

  TEST_CASE("Golden: borders", "[UI][GPU][Golden]")
  {
    CheckPage("borders", 360u, 240u);
  }

  TEST_CASE("Golden: border images", "[UI][GPU][Golden]")
  {
    // Chrome tiles `repeat` and `space` pieces its own way (no joins between repeated tiles, hairline gaps
    // for space); the page follows the ported tiling, so only its own golden is checked.
    CheckPage("border-images", 360u, 200u, true);
  }

  TEST_CASE("Golden: text", "[UI][GPU][Golden]")
  {
    // Chrome on Windows rasterises glyphs with DirectWrite, whose hinting and contrast differ from
    // FreeType's, so its golden is only reported.
    CheckPage("text", 360u, 200u, true);
  }

  TEST_CASE("Golden: decorations", "[UI][GPU][Golden]")
  {
    // Text, so Chrome's golden is only reported (see "Golden: text").
    CheckPage("decorations", 360u, 240u, true);
  }

  TEST_CASE("Golden: tables", "[UI][GPU][Golden]")
  {
    CheckPage("tables", 360u, 240u);
  }

  TEST_CASE("Golden: focus rings", "[UI][GPU][Golden]")
  {
    // Both follow rounded corners, but Chrome draws its own ring (its width and placement), while the page
    // draws a fixed 2 pixel ring in the outline colour, so only its own golden is checked.
    CheckPage("focus", 360u, 200u, true, [](View &view) { (void)view.KeyPressed(pal::Key::Tab); });
  }

  TEST_CASE("Golden: scrollbars", "[UI][GPU][Golden]")
  {
    // Chrome's captures hide scrollbars (and its Windows scrollbars aren't the theme's), so there's no Chrome
    // golden.
    CheckPage("scrollbars", 360u, 240u, true,
              [](View &view)
              {
                view.ScrollTo({20, 40});
                view.Document().GetElementById(u8"scrolled")->SetScrollTop(90);
                view.UpdateRendering();
              });
  }
}
