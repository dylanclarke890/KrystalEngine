#include "Krystal.Gfx.Tests/Golden.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/DOMRectReadOnly.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/CompositionUnderline.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Gfx.Tests/TestDevice.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.UI.Tests/ViewTestUtils.hpp"
#include "Krystal.UI/View.hpp"
#include <string>
#include <unicode/uloc.h>

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
      WaitForLoad(view);
      // Let the page's resources load, as Chrome's capture does: its images and style sheets load as
      // networking tasks, and its web fonts once laying text out asks for them (their text is invisible
      // until then).
      for (int32 pass = 0; pass < 4; ++pass)
      {
        view.UpdateRendering();
        boo::html::EventLoop::Current().RunUntilIdle();
      }
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

  TEST_CASE("Golden: selection", "[UI][GPU][Golden]")
  {
    // The scene's script selects for Chrome; the page has no script here, so the selection is made the same
    // way through the document. Chrome's golden is only reported: its glyphs are DirectWrite's, and it
    // highlights only the text and line ends where the blocks here fill their selection gaps.
    CheckPage("selection", 360u, 240u, true,
              [](View &view)
              {
                auto &document = view.Document();
                auto *start = document.GetElementById(u8"start")->FirstChild();
                auto *end = document.GetElementById(u8"end")->FirstChild();
                document.Selection().MoveTo(boo::dom::MakeContainerOffsetPosition(ShareRefPtr(start), 14),
                                            boo::dom::MakeContainerOffsetPosition(ShareRefPtr(end), 8),
                                            boo::Affinity::Downstream);
                view.UpdateRendering();
              });
  }

  /// @brief Composes "kanji henkan" at the end of the scene's #field with @p underlines.
  void Compose(View &view, const List<boo::CompositionUnderline> &underlines)
  {
    auto &document = view.Document();
    auto &field = Downcast<boo::html::HTMLElement>(*document.GetElementById(u8"field"));
    field.Focus();
    auto &text = Downcast<boo::dom::Text>(*field.FirstChild());
    document.Selection().MoveTo(
      boo::dom::MakeContainerOffsetPosition(ShareRefPtr(&text), static_cast<uint32>(text.Length())),
      boo::Affinity::Downstream);
    document.GetEditor().SetComposition(u8"kanji henkan", underlines, 12, 12);
    view.UpdateRendering();
  }

  TEST_CASE("Golden: composition", "[UI][GPU][Golden]")
  {
    // Chrome's golden is only reported: a page can't compose text, so it has none.
    using boo::CompositionUnderlineColor;
    CheckPage("composition", 360u, 120u, true,
              [](View &view)
              {
                Compose(view,
                        {{0, 5, CompositionUnderlineColor::TextColor, {}, false},
                         {5, 9, CompositionUnderlineColor::TextColor, {}, true},
                         {9, 12, CompositionUnderlineColor::GivenColor, boo::Color(boo::Color::red), false}});
              });
  }

  TEST_CASE("Golden: composition fill", "[UI][GPU][Golden]")
  {
    // Without underlines, the composition is filled. Chrome's golden is only reported (see above).
    CheckPage("composition-fill", 360u, 120u, true, [](View &view) { Compose(view, {}); });
  }

  TEST_CASE("Golden: decorations", "[UI][GPU][Golden]")
  {
    // Text, so Chrome's golden is only reported (see "Golden: text").
    CheckPage("decorations", 360u, 240u, true);
  }

  TEST_CASE("Golden: animations", "[UI][GPU][Golden]")
  {
    // Each animation is paused partway through by a negative delay, so neither capture depends on the time.
    CheckPage("animations", 360u, 240u);
  }

  TEST_CASE("Golden: web fonts", "[UI][GPU][Golden]")
  {
    // Text, so Chrome's golden is only reported (see "Golden: text"); Ahem's boxes match it.
    CheckPage("webfonts", 360u, 240u, true);
  }

  TEST_CASE("Golden: filters", "[UI][GPU][Golden]")
  {
    CheckPage("filters", 360u, 240u);
  }

  TEST_CASE("Golden: inline SVG", "[UI][GPU][Golden]")
  {
    CheckPage("svg", 360u, 240u);
  }

  TEST_CASE("Golden: SVG instances and embedded content", "[UI][GPU][Golden]")
  {
    CheckPage("svg-content", 360u, 240u);
  }

  TEST_CASE("Golden: SVG images", "[UI][GPU][Golden]")
  {
    CheckPage("svg-images", 360u, 240u, false,
              [](View &view)
              {
                boo::html::EventLoop::Current().RunUntilIdle();
                view.UpdateRendering();
              });
  }

  TEST_CASE("Golden: SVG paint servers", "[UI][GPU][Golden]")
  {
    CheckPage("svg-paint-servers", 360u, 240u);
  }

  TEST_CASE("Golden: backdrop filters", "[UI][GPU][Golden]")
  {
    CheckPage("backdrop", 360u, 240u);
  }

  TEST_CASE("Golden: masks", "[UI][GPU][Golden]")
  {
    CheckPage("masks", 360u, 240u);
  }

  TEST_CASE("Golden: mask borders", "[UI][GPU][Golden]")
  {
    // Chrome's golden is the scene with the prefixed -webkit-mask-box-image, which always keeps the middle.
    CheckPage("mask-border", 240u, 120u, true);
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

  TEST_CASE("Golden: text fields", "[UI][GPU][Golden]")
  {
    // The fields lay out as Chrome's do, but its golden is only reported: its DirectWrite glyphs leave a seam
    // between Ahem's boxes (see "Golden: text").
    CheckPage("textfields", 360u, 240u, true);
  }

  TEST_CASE("Golden: controls", "[UI][GPU][Golden]")
  {
    // The controls are drawn by the Adwaita theme, which isn't Chrome's look, so Chrome's golden is only
    // reported. The scene's script makes a checkbox indeterminate for Chrome; it's done the same way here.
    CheckPage("controls", 360u, 240u, true,
              [](View &view)
              {
                Downcast<boo::html::HTMLInputElement>(*view.Document().GetElementById(u8"mixed"))
                  .SetIndeterminate(true);
                view.UpdateRendering();
              });
  }

  TEST_CASE("Golden: ranges", "[UI][GPU][Golden]")
  {
    // Ranges and progress bars are drawn by the Adwaita theme, so Chrome's golden is only reported; meters
    // are drawn by their shadow trees' gradients, which Chrome doesn't use either.
    CheckPage("ranges", 360u, 280u, true);
  }

  TEST_CASE("Golden: selects", "[UI][GPU][Golden]")
  {
    // Drop-downs and list boxes are drawn by the Adwaita theme, so Chrome's golden is only reported.
    CheckPage("selects", 360u, 280u, true);
  }

  TEST_CASE("Golden: select popup", "[UI][GPU][Golden]")
  {
    // The popup is the view's overlay; Chrome draws its own in a window, so there's no Chrome golden.
    CheckPage("select-popup", 240u, 200u, true,
              [](View &view)
              {
                auto select = view.Document().GetElementById(u8"fruit");
                REQUIRE(select);
                auto box = select->GetBoundingClientRect();
                boo::IntPoint inside(static_cast<int32>(box->X()) + 5, static_cast<int32>(box->Y()) + 5);
                view.MousePressed(inside, pal::MouseButton::Left);
                view.MouseReleased(inside, pal::MouseButton::Left);
                REQUIRE(view.ShownOverlay());
                view.UpdateRendering();
              });
  }

  namespace
  {
    /// @brief Makes the system's language US English while it lives, so form controls show dates the same on
    /// every machine.
    struct USEnglishScope
    {
      std::string Previous;

      USEnglishScope() noexcept : Previous(uloc_getDefault())
      {
        UErrorCode status = U_ZERO_ERROR;
        uloc_setDefault("en_US", &status);
      }

      ~USEnglishScope() noexcept
      {
        UErrorCode status = U_ZERO_ERROR;
        uloc_setDefault(Previous.c_str(), &status);
      }
    };
  }

  TEST_CASE("Golden: dates", "[UI][GPU][Golden]")
  {
    // Dates' fields and colour wells are drawn by the Adwaita theme, and Chrome on Windows lays dates out in
    // the system's regional format, so Chrome's golden is only reported.
    USEnglishScope english;
    CheckPage("dates", 360u, 260u, true);
  }

  TEST_CASE("Golden: date picker", "[UI][GPU][Golden]")
  {
    // The picker is the view's overlay; Chrome draws its own in a window, so there's no Chrome golden.
    USEnglishScope english;
    CheckPage("date-picker", 280u, 260u, true,
              [](View &view)
              {
                auto &input =
                  Downcast<boo::html::HTMLInputElement>(*view.Document().GetElementById(u8"date"));
                input.Click();
                REQUIRE(view.ShownOverlay());
                // The highlight moves to the 12th.
                (void)view.KeyPressed(pal::Key::RightArrow);
                (void)view.KeyPressed(pal::Key::RightArrow);
                (void)view.KeyPressed(pal::Key::RightArrow);
                (void)view.KeyPressed(pal::Key::RightArrow);
                (void)view.KeyPressed(pal::Key::RightArrow);
                view.UpdateRendering();
              });
  }

  TEST_CASE("Golden: color picker", "[UI][GPU][Golden]")
  {
    CheckPage("color-picker", 240u, 200u, true,
              [](View &view)
              {
                auto &input =
                  Downcast<boo::html::HTMLInputElement>(*view.Document().GetElementById(u8"color"));
                input.Click();
                REQUIRE(view.ShownOverlay());
                view.UpdateRendering();
              });
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

  TEST_CASE("Golden: images", "[UI][GPU][Golden]")
  {
    // The page's images load from files as networking tasks, which the event loop runs before it's painted.
    CheckPage("images", 360u, 320u, false,
              [](View &view)
              {
                boo::html::EventLoop::Current().RunUntilIdle();
                view.UpdateRendering();
              });
  }

  TEST_CASE("Golden: img", "[UI][GPU][Golden]")
  {
    CheckPage("img", 360u, 240u, false,
              [](View &view)
              {
                boo::html::EventLoop::Current().RunUntilIdle();
                view.UpdateRendering();
              });
  }
}
