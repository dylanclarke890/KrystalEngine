#include "Krystal.Gfx.Tests/Golden.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/DOMRectReadOnly.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/CompositionUnderline.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasGradient.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasPattern.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2D.hpp"
#include "Krystal.Booey/HTML/Canvas/EngineTextureRenderingContext.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/ImageData.hpp"
#include "Krystal.Booey/SVG/SVGImageElement.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Gfx.Tests/TestDevice.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.UI.Tests/ViewTestUtils.hpp"
#include "Krystal.UI/EngineTexture.hpp"
#include "Krystal.UI/View.hpp"
#include <numbers>
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

    /// @brief Run on the view, with the device it's painted with, before it's painted; the textures it adds
    /// to the list are destroyed once the page is painted.
    using PageSetup = Func<void(View &, gfx::Device &, List<gfx::TextureHandle> &)>;

    /// @brief Paints the page golden `golden/scenes/<name>.html` through a `View` of its size, after @p setup
    /// (input, say) if given.
    KRYS_NODISCARD GoldenImage Render(gfx::Device &device, gfx::Renderer2D &renderer, const std::string &name,
                                      uint32 width, uint32 height, const PageSetup &setup)
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
      List<gfx::TextureHandle> textures;
      if (setup)
      {
        view.UpdateRendering();
        setup(view, device, textures);
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
      for (auto texture : textures)
      {
        device.Destroy(texture);
      }
      return image;
    }

    /// @brief Checks a page against its own golden (writing it when updating or missing) and against
    /// Chrome's, writing the actual image and the differences to the output directory either way.
    /// @param chromeReportOnly Whether Chrome's golden is only reported, not checked: where Chrome's
    /// rendering follows its own algorithm rather than the one ported (border image tiling).
    /// @param setup Run on the view before it's painted, if given.
    void CheckPageOnDevice(const std::string &name, uint32 width, uint32 height, bool chromeReportOnly,
                           const PageSetup &setup)
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

    /// @brief A 40 by 20 texture of the engine textures scene's pattern: blue on the left, orange on the
    /// right, half transparent at the bottom right.
    KRYS_NODISCARD gfx::TextureHandle CreateEngineTexture(gfx::Device &device, bool premultiplied)
    {
      constexpr uint32 width = 40u;
      constexpr uint32 height = 20u;
      auto texture = device.CreateTexture({.Format = gfx::TextureFormat::BGRA8Unorm,
                                           .Width = width,
                                           .Height = height,
                                           .Usage = gfx::TextureUsage::Sampled,
                                           .DebugName = u8"Golden.EngineTexture"});
      if (!texture)
      {
        return {};
      }
      // ARGB in a 32-bit value is BGRA in memory.
      const uint32 halfOrange = premultiplied ? 0x80'77'3B'19u : 0x80'EE'77'33u;
      List<uint32> pixels(static_cast<size_t>(width) * height);
      for (uint32 y = 0; y < height; ++y)
      {
        for (uint32 x = 0; x < width; ++x)
        {
          pixels[y * width + x] = x < 20u ? 0xFF'33'66'99u : (y < 10u ? 0xFF'EE'77'33u : halfOrange);
        }
      }
      device.WriteTexture(*texture, gfx::TextureRegion::Whole2D(width, height),
                          {reinterpret_cast<const byte *>(pixels.data()), pixels.size() * sizeof(uint32)});
      return *texture;
    }

    void CheckPage(const std::string &name, uint32 width, uint32 height, bool chromeReportOnly = false,
                   const Func<void(View &)> &setup = {})
    {
      PageSetup pageSetup;
      if (setup)
      {
        pageSetup = [&](View &view, gfx::Device &, List<gfx::TextureHandle> &)
        {
          setup(view);
        };
      }
      CheckPageOnDevice(name, width, height, chromeReportOnly, pageSetup);
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

  TEST_CASE("Golden: SVG clipping, masking and markers", "[UI][GPU][Golden]")
  {
    CheckPage("svg-clip-mask-markers", 360u, 240u);
  }

  TEST_CASE("Golden: SVG text", "[UI][GPU][Golden]")
  {
    // Chrome's golden is only reported: its glyphs are DirectWrite's.
    CheckPage("svg-text", 360u, 240u, true);
  }

  TEST_CASE("Golden: SVG filters", "[UI][GPU][Golden]")
  {
    CheckPage("svg-filters", 360u, 240u);
  }

  TEST_CASE("Golden: SVG animation", "[UI][GPU][Golden]")
  {
    // Each animation is frozen, holds a discrete value or is half way through a very long one, so neither
    // capture depends on the time.
    CheckPage("svg-animation", 360u, 240u);
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

  TEST_CASE("Golden: MathML", "[UI][GPU][Golden]")
  {
    // Both draw STIX Two Math, but Chrome's glyphs are DirectWrite's, so its golden is only reported.
    CheckPage("mathml", 360u, 240u, true);
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

  TEST_CASE("Golden: engine textures", "[UI][GPU][Golden]")
  {
    // Each canvas shows a texture of the pattern the scene's script draws for Chrome; half of them declare
    // their alpha unpremultiplied, which must look the same.
    CheckPageOnDevice(
      "engine-textures", 360u, 200u, false,
      [](View &view, gfx::Device &device, List<gfx::TextureHandle> &textures)
      {
        for (auto id : {u8"natural", u8"stretched", u8"contain", u8"cover", u8"turned", u8"faded"})
        {
          const bool premultiplied = id != utf8_stringview(u8"contain") && id != utf8_stringview(u8"turned");
          auto texture = CreateEngineTexture(device, premultiplied);
          REQUIRE(texture.IsValid());
          textures.push_back(texture);
          auto *canvas =
            DynamicDowncast<boo::html::HTMLCanvasElement>(view.Document().GetElementById(id).get());
          REQUIRE(canvas);
          auto context = canvas->GetContext(boo::html::EngineTextureRenderingContext::ContextType);
          REQUIRE(context);
          auto &textureContext = Downcast<boo::html::EngineTextureRenderingContext>(*context);
          textureContext.SetTexture(EngineTexture::Create(
            texture, {40, 20},
            premultiplied ? AlphaPremultiplication::Premultiplied : AlphaPremultiplication::Unpremultiplied));
          textureContext.TextureChanged();
        }
        view.UpdateRendering();
      });
  }

  TEST_CASE("Golden: canvas 2D", "[UI][GPU][Golden]")
  {
    // The calls the scene's script makes for Chrome, in two rendering updates.
    CheckPageOnDevice("canvas-2d", 360u, 200u, false,
                      [](View &view, gfx::Device &, List<gfx::TextureHandle> &)
                      {
                        using boo::dom::DOMString;
                        auto *canvas = DynamicDowncast<boo::html::HTMLCanvasElement>(
                          view.Document().GetElementById(u8"canvas").get());
                        REQUIRE(canvas);
                        auto context = canvas->GetContext2d();
                        REQUIRE(context);

                        context->SetFillStyle(DOMString(u8"#336699"));
                        context->FillRect(10, 10, 60, 40);
                        context->SetFillStyle(DOMString(u8"rgba(238, 119, 51, 0.6)"));
                        context->FillRect(40, 30, 60, 40);
                        context->SetLineWidth(4);
                        context->SetStrokeStyle(DOMString(u8"#224422"));
                        context->StrokeRect(10, 90, 90, 40);

                        auto linear = context->CreateLinearGradient(120, 0, 220, 0).ReleaseValue();
                        std::ignore = linear->AddColorStop(0, u8"#ff0000");
                        std::ignore = linear->AddColorStop(1, u8"#0000ff");
                        context->SetFillStyle(krys::move(linear));
                        context->FillRect(120, 10, 100, 40);
                        auto radial = context->CreateRadialGradient(170, 110, 5, 170, 110, 40).ReleaseValue();
                        std::ignore = radial->AddColorStop(0, u8"#ffffff");
                        std::ignore = radial->AddColorStop(1, u8"#008800");
                        context->SetFillStyle(krys::move(radial));
                        context->BeginPath();
                        std::ignore = context->Arc(170, 110, 40, 0, 2 * std::numbers::pi_v<float>);
                        context->Fill();

                        context->Save();
                        context->Translate(280, 50);
                        context->Rotate(std::numbers::pi / 8);
                        context->BeginPath();
                        context->Rect(-30, -30, 60, 60);
                        context->Clip();
                        context->SetLineWidth(8);
                        context->SetLineJoin(boo::html::CanvasLineJoin::Round);
                        context->SetStrokeStyle(DOMString(u8"#880088"));
                        view.UpdateRendering();

                        context->BeginPath();
                        context->MoveTo(-40, 20);
                        context->LineTo(0, -20);
                        context->LineTo(40, 20);
                        context->Stroke();
                        context->Restore();

                        context->SetLineWidth(6);
                        context->SetLineCap(boo::html::CanvasLineCap::Round);
                        context->SetLineDash({12, 10});
                        context->SetStrokeStyle(DOMString(u8"#cc3333"));
                        context->BeginPath();
                        context->MoveTo(240, 110);
                        context->LineTo(330, 110);
                        context->Stroke();
                        context->SetGlobalAlpha(0.5f);
                        context->SetFillStyle(DOMString(u8"#000000"));
                        context->BeginPath();
                        std::ignore =
                          context->RoundRect(240, 125, 90, 45, boo::html::CanvasPath::RadiusVariant(12.0));
                        context->Fill();
                        context->ClearRect(270, 140, 30, 15);
                        view.UpdateRendering();
                      });
  }

  TEST_CASE("Golden: canvas images", "[UI][GPU][Golden]")
  {
    // The calls the scene's script makes for Chrome once the photo has loaded.
    CheckPageOnDevice(
      "canvas-images", 360u, 200u, false,
      [](View &view, gfx::Device &device, List<gfx::TextureHandle> &)
      {
        using boo::dom::DOMString;
        // The canvases are read back and drawn into each other before the view first paints.
        view.SetDevice(device);
        auto &document = view.Document();
        auto *canvas =
          DynamicDowncast<boo::html::HTMLCanvasElement>(document.GetElementById(u8"canvas").get());
        auto *tileCanvas =
          DynamicDowncast<boo::html::HTMLCanvasElement>(document.GetElementById(u8"tile").get());
        auto *photo = DynamicDowncast<boo::html::HTMLImageElement>(document.GetElementById(u8"photo").get());
        REQUIRE(canvas);
        REQUIRE(tileCanvas);
        REQUIRE(photo);
        REQUIRE(photo->Complete());
        auto context = canvas->GetContext2d();
        auto tile = tileCanvas->GetContext2d();
        REQUIRE(context);
        REQUIRE(tile);

        REQUIRE_FALSE(context->DrawImage(ShareRefPtr(photo), 10, 10).HasException());
        REQUIRE_FALSE(context->DrawImage(ShareRefPtr(photo), 70, 10, 96, 64).HasException());
        REQUIRE_FALSE(context->DrawImage(ShareRefPtr(photo), 12, 8, 24, 16, 180, 10, 48, 32).HasException());

        tile->SetFillStyle(DOMString(u8"#336699"));
        tile->FillRect(0, 0, 16, 16);
        tile->SetFillStyle(DOMString(u8"#ee7733"));
        tile->FillRect(0, 0, 8, 8);
        tile->FillRect(8, 8, 8, 8);
        auto pattern = context->CreatePattern(ShareRefPtr(tileCanvas), u8"repeat");
        REQUIRE_FALSE(pattern.HasException());
        REQUIRE(pattern.Value());
        context->SetFillStyle(Ref<boo::html::CanvasPattern>(pattern.Value()));
        context->FillRect(240, 10, 90, 64);

        tile->SetFillStyle(DOMString(u8"#22aa55"));
        tile->FillRect(0, 0, 16, 16);
        REQUIRE_FALSE(context->DrawImage(ShareRefPtr(tileCanvas), 10, 90, 32, 32).HasException());
        REQUIRE_FALSE(context->DrawImage(ShareRefPtr(canvas), 70, 10, 96, 64, 60, 90, 48, 32).HasException());

        auto pixels = context->CreateImageData(64, 40, {});
        REQUIRE_FALSE(pixels.HasException());
        auto bytes = pixels.Value()->Data().AsUint8ClampedArray();
        for (size_t y = 0; y < 40u; ++y)
        {
          for (size_t x = 0; x < 64u; ++x)
          {
            const auto i = (y * 64u + x) * 4u;
            bytes->Set(i, static_cast<double>(x * 4u));
            bytes->Set(i + 1, static_cast<double>(y * 6u));
            bytes->Set(i + 2, 160);
            bytes->Set(i + 3, x < 32u ? 255 : 128);
          }
        }
        context->PutImageData(*pixels.Value(), 130, 90);

        auto read = context->GetImageData(10, 10, 48, 32, {});
        REQUIRE_FALSE(read.HasException());
        context->PutImageData(*read.Value(), 210, 90);
        view.UpdateRendering();
      });
  }

  TEST_CASE("Golden: canvas text", "[UI][GPU][Golden]")
  {
    // The calls the scene's script makes for Chrome once Ahem has loaded. Its boxes lay out as Chrome's do,
    // but Chrome's golden is only reported: its DirectWrite glyphs leave a seam between them (see "Golden:
    // text").
    CheckPageOnDevice("canvas-text", 360u, 200u, true,
                      [](View &view, gfx::Device &, List<gfx::TextureHandle> &)
                      {
                        using boo::dom::DOMString;
                        auto *canvas = DynamicDowncast<boo::html::HTMLCanvasElement>(
                          view.Document().GetElementById(u8"canvas").get());
                        REQUIRE(canvas);
                        auto context = canvas->GetContext2d();
                        REQUIRE(context);

                        context->SetFont(u8"40px Ahem");
                        context->SetFillStyle(DOMString(u8"#336699"));
                        context->FillText(u8"Fill", 10, 50);
                        context->SetLineWidth(2);
                        context->SetStrokeStyle(DOMString(u8"#cc3333"));
                        context->StrokeText(u8"Line", 10, 100);

                        auto gradient = context->CreateLinearGradient(190, 0, 330, 0).ReleaseValue();
                        std::ignore = gradient->AddColorStop(0, u8"#ff0000");
                        std::ignore = gradient->AddColorStop(1, u8"#0000ff");
                        context->SetFillStyle(krys::move(gradient));
                        context->SetFont(u8"30px Ahem");
                        context->FillText(u8"Grad", 200, 45);

                        context->SetFillStyle(DOMString(u8"#118833"));
                        context->SetFont(u8"20px Ahem");
                        context->SetTextAlign(boo::html::CanvasTextAlign::Center);
                        context->SetTextBaseline(boo::html::CanvasTextBaseline::Middle);
                        context->FillText(u8"Mid", 260, 90);

                        context->SetTextAlign(boo::html::CanvasTextAlign::Start);
                        context->SetTextBaseline(boo::html::CanvasTextBaseline::Alphabetic);
                        context->SetFillStyle(DOMString(u8"#884488"));
                        context->FillText(u8"Squeeze", 10, 150, 70.0);
                        context->SetLetterSpacing(u8"6px");
                        context->FillText(u8"ab", 100, 150);
                        context->SetLetterSpacing(u8"0px");

                        context->SetDirection(boo::html::CanvasDirection::Rtl);
                        context->SetFillStyle(DOMString(u8"#ee7733"));
                        context->FillText(u8"abc", 330, 170);
                        view.UpdateRendering();
                      });
  }

  TEST_CASE("Golden: canvas filters", "[UI][GPU][Golden]")
  {
    // The calls the scene's script makes for Chrome.
    CheckPageOnDevice("canvas-filters", 360u, 200u, false,
                      [](View &view, gfx::Device &, List<gfx::TextureHandle> &)
                      {
                        using boo::dom::DOMString;
                        auto *canvas = DynamicDowncast<boo::html::HTMLCanvasElement>(
                          view.Document().GetElementById(u8"canvas").get());
                        REQUIRE(canvas);
                        auto context = canvas->GetContext2d();
                        REQUIRE(context);

                        context->SetFilterString(u8"blur(4px)");
                        context->SetFillStyle(DOMString(u8"#336699"));
                        context->FillRect(20, 20, 80, 60);

                        context->SetFilterString(u8"drop-shadow(6px 6px 3px rgba(0, 0, 0, 0.6))");
                        context->SetFillStyle(DOMString(u8"#ee7733"));
                        context->BeginPath();
                        std::ignore = context->Arc(160, 50, 30, 0, 2 * std::numbers::pi_v<float>);
                        context->Fill();

                        auto gradient = context->CreateLinearGradient(220, 0, 320, 0).ReleaseValue();
                        std::ignore = gradient->AddColorStop(0, u8"#ff0000");
                        std::ignore = gradient->AddColorStop(1, u8"#0000ff");
                        context->SetFilterString(u8"grayscale(1)");
                        context->SetFillStyle(krys::move(gradient));
                        context->FillRect(220, 20, 100, 60);

                        context->SetFilterString(u8"opacity(0.5)");
                        context->SetFillStyle(DOMString(u8"#118833"));
                        context->FillRect(20, 110, 80, 50);
                        context->SetFilterString(u8"hue-rotate(180deg)");
                        context->SetFillStyle(DOMString(u8"#cc3333"));
                        context->FillRect(120, 110, 80, 50);

                        context->SetFilterString(u8"blur(1px) drop-shadow(3px 3px 0 #224422)");
                        context->SetLineWidth(6);
                        context->SetStrokeStyle(DOMString(u8"#884488"));
                        context->StrokeRect(230, 110, 80, 50);
                        context->SetFilterString(u8"none");
                        context->SetFillStyle(DOMString(u8"#222222"));
                        context->FillRect(260, 128, 20, 14);
                        view.UpdateRendering();
                      });
  }
}
