#include "Krystal.Gfx.Tests/Golden.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Gfx.Tests/TestDevice.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <numbers>

namespace krys::gfx::tests
{
  namespace
  {
    /// @brief Render2D's own golden is checked tightly: any change beyond rounding is a regression.
    constexpr uint32 OwnTolerance = 2u;
    /// @brief Chrome antialiases, blurs and dithers a little differently, so its golden is checked loosely.
    constexpr uint32 ChromeTolerance = 24u;

    constexpr SRGBA<uint8> Black {0, 0, 0};

    /// @brief How far away shapes go to cast only their shadows, as `box-shadow` paints them.
    constexpr float Away = 10'000.f;

    /// @brief An outer `box-shadow`: the box's shape, moved out of view, casts it, clipped out of the box. An
    /// opaque box is clipped out a pixel inside its edge, so the shadow carries on under its antialiasing.
    void BoxShadow(Renderer2D &renderer, RoundedRect2D box, Vec2f offset, float blur, SRGBA<uint8> colour)
    {
      renderer.Save();
      auto clipOut = box;
      clipOut.Rect = {box.Rect.X + 1.f, box.Rect.Y + 1.f, box.Rect.Width - 2.f, box.Rect.Height - 2.f};
      renderer.ClipOutRoundedRect(clipOut);
      renderer.SetDropShadow({.Offset = {offset.x - Away, offset.y}, .BlurRadius = blur, .Colour = colour});
      box.Rect.X += Away;
      renderer.FillRoundedRect(box, Black);
      renderer.Restore();
    }

    /// @brief An inset `box-shadow`: a rect with the box as its hole, moved out of view, casts it, clipped to
    /// the box.
    void InsetShadow(Renderer2D &renderer, const RoundedRect2D &box, Vec2f offset, float blur,
                     SRGBA<uint8> colour)
    {
      renderer.Save();
      renderer.ClipRoundedRect(box);
      renderer.SetDropShadow({.Offset = {offset.x - Away, offset.y}, .BlurRadius = blur, .Colour = colour});
      renderer.Translate(Away, 0.f);
      const float reach = blur + std::abs(offset.x) + std::abs(offset.y) + 2.f;
      const auto &rect = box.Rect;
      renderer.FillRectWithRoundedHole(
        {rect.X - reach, rect.Y - reach, rect.Width + 2.f * reach, rect.Height + 2.f * reach}, box, Black);
      renderer.Restore();
    }

    KRYS_NODISCARD RoundedRect2D Box(float x, float y, float width, float height, float radius)
    {
      return {{x, y, width, height}, CornerRadii2D::Uniform(radius)};
    }

    KRYS_NODISCARD ExtendedSRGBA<float> Hex(uint32 rgb)
    {
      return {static_cast<float>((rgb >> 16u) & 255u) / 255.f, static_cast<float>((rgb >> 8u) & 255u) / 255.f,
              static_cast<float>(rgb & 255u) / 255.f, 1.f};
    }

    KRYS_NODISCARD SRGBA<uint8> Rgb(uint32 rgb, uint8 alpha = 255u)
    {
      return {static_cast<uint8>((rgb >> 16u) & 255u), static_cast<uint8>((rgb >> 8u) & 255u),
              static_cast<uint8>(rgb & 255u), alpha};
    }

    /// @brief `golden/scenes/shadows.html`: outer shadows soft, sharp and wide, and an inset shadow.
    void ShadowsScene(Renderer2D &renderer)
    {
      renderer.FillRect({0.f, 0.f, 320.f, 240.f}, Rgb(0xe9ecf2));

      const auto soft = Box(24.f, 24.f, 112.f, 72.f, 12.f);
      BoxShadow(renderer, soft, {0.f, 4.f}, 12.f, Rgb(0x000000, 89));
      renderer.FillRoundedRect(soft, Rgb(0xffffff));

      const auto sharp = Box(184.f, 24.f, 112.f, 72.f, 0.f);
      BoxShadow(renderer, sharp, {6.f, 6.f}, 0.f, Rgb(0x2846aa));
      renderer.FillRoundedRect(sharp, Rgb(0xffffff));

      const auto inset = Box(24.f, 136.f, 112.f, 72.f, 16.f);
      renderer.FillRoundedRect(inset, Rgb(0xfafafa));
      InsetShadow(renderer, inset, {0.f, 4.f}, 10.f, Rgb(0x000000, 128));

      const auto wide = Box(184.f, 136.f, 112.f, 72.f, 36.f);
      BoxShadow(renderer, wide, {0.f, 0.f}, 30.f, Rgb(0xdc3c3c, 217));
      renderer.FillRoundedRect(wide, Rgb(0xffffff));
    }

    /// @brief `golden/scenes/ui.html`: gradients, a bordered card, a clip, a group's opacity, a blend mode, a
    /// rotation and a stroked path.
    void UiScene(Renderer2D &renderer)
    {
      renderer.FillRect({0.f, 0.f, 360.f, 260.f}, Rgb(0xffffff));

      // A header: linear-gradient(to right, ...).
      const Gradient2D header {
        .Shape = LinearGradient2D {{0.f, 20.f}, {360.f, 20.f}},
        .Stops = {{.Offset = 0.f, .Colour = Hex(0x4f6df5)}, {.Offset = 1.f, .Colour = Hex(0xb04ff5)}}};
      renderer.FillRect({0.f, 0.f, 360.f, 40.f}, header);

      // A card with a radial-gradient background (positioned in its padding box) and a 3px border.
      const auto card = Box(16.f, 56.f, 140.f, 100.f, 10.f);
      const Gradient2D glow {
        .Shape = RadialGradient2D {{59.f, 99.f}, {59.f, 99.f}, 0.f, 50.f},
        .Stops = {{.Offset = 0.f, .Colour = Hex(0xffffff)}, {.Offset = 1.f, .Colour = Hex(0x99cccc)}}};
      renderer.Save();
      renderer.ClipRoundedRect(card);
      renderer.FillRect(card.Rect, glow);
      renderer.FillRectWithRoundedHole(card.Rect, Box(19.f, 59.f, 134.f, 94.f, 7.f), Rgb(0x33aa77));
      renderer.Restore();

      // conic-gradient(from 45deg, red, yellow, lime, aqua, blue, magenta, red) in a circle.
      const uint32 hues[] {0xff0000, 0xffff00, 0x00ff00, 0x00ffff, 0x0000ff, 0xff00ff, 0xff0000};
      Gradient2D wheel {.Shape = ConicGradient2D {{226.f, 106.f}, std::numbers::pi_v<float> / 4.f}};
      for (uint32 i = 0u; i < 7u; ++i)
      {
        wheel.Stops.push_back({.Offset = static_cast<float>(i) / 6.f, .Colour = Hex(hues[i])});
      }
      renderer.FillRoundedRect(Box(176.f, 56.f, 100.f, 100.f, 50.f), wheel);

      // A square turned by 20 degrees about its centre.
      renderer.Save();
      renderer.Translate(316.f, 106.f);
      renderer.Rotate(20.f * std::numbers::pi_v<float> / 180.f);
      renderer.FillRect({-20.f, -20.f, 40.f, 40.f}, Rgb(0xee0044));
      renderer.Restore();

      // overflow: hidden with rounded corners, its child overflowing.
      const auto clip = Box(16.f, 172.f, 140.f, 72.f, 14.f);
      renderer.Save();
      renderer.ClipRoundedRect(clip);
      renderer.FillRect(clip.Rect, Rgb(0x222233));
      renderer.FillRect({76.f, 200.f, 120.f, 60.f}, Rgb(0xf5a623));
      renderer.Restore();

      // opacity: 0.5 on a group of two circles.
      renderer.BeginLayer(0.5f);
      renderer.FillRoundedRect(Box(176.f, 166.f, 40.f, 40.f, 20.f), Rgb(0xff0000));
      renderer.FillRoundedRect(Box(196.f, 176.f, 40.f, 40.f, 20.f), Rgb(0x0000ff));
      renderer.EndLayer();

      // mix-blend-mode: multiply over a rect.
      renderer.FillRect({250.f, 176.f, 90.f, 40.f}, Rgb(0x3366ff));
      renderer.Save();
      renderer.SetBlendMode(BlendMode::Multiply);
      renderer.FillRoundedRect(Box(270.f, 166.f, 50.f, 50.f, 25.f), Rgb(0x33cc66));
      renderer.Restore();

      // An SVG polyline, 3px with round joins.
      Path2D zigzag;
      zigzag.MoveTo({181.f, 252.f});
      zigzag.LineTo({221.f, 228.f});
      zigzag.LineTo({261.f, 252.f});
      zigzag.LineTo({301.f, 228.f});
      zigzag.LineTo({341.f, 252.f});
      renderer.StrokePath(zigzag, {.Width = 3.f, .Join = LineJoin::Round}, Rgb(0x333333));
    }

    /// @brief `golden/scenes/conic.html`: a conic gradient through saturated stops, whose corners show as
    /// rays.
    void ConicScene(Renderer2D &renderer)
    {
      renderer.FillRect({0.f, 0.f, 200.f, 200.f}, Rgb(0xffffff));
      const Gradient2D wheel {.Shape = ConicGradient2D {{100.f, 100.f}, 0.f},
                              .Stops = {{.Offset = 0.f, .Colour = Hex(0xef476f)},
                                        {.Offset = 0.33f, .Colour = Hex(0xffd166)},
                                        {.Offset = 0.66f, .Colour = Hex(0x06d6a0)},
                                        {.Offset = 1.f, .Colour = Hex(0xef476f)}}};
      renderer.FillRoundedRect(Box(10.f, 10.f, 180.f, 180.f, 90.f), wheel);
    }

    /// @brief Liberation Sans, as the text scene draws it; made once.
    KRYS_NODISCARD const FontFace2D &LiberationSans()
    {
      static const auto face = []
      {
        std::ifstream file(std::filesystem::path {KRYS_GFX_TESTS_FONTS_DIR} / "LiberationSans-Regular.ttf",
                           std::ios::binary);
        REQUIRE(file);
        const List<char> bytes {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
        List<byte> data(bytes.size());
        std::memcpy(data.data(), bytes.data(), bytes.size());
        auto opened = text::FreeTypeFace::Create(text::FontFileData::Create(krys::move(data)), 0);
        REQUIRE(opened);
        auto created = FontFace2D::Create(Ref<text::FreeTypeFace> {krys::move(opened)});
        REQUIRE(created.has_value());
        return krys::move(*created);
      }();
      return *face;
    }

    /// @brief Left-to-right text without shaping or kerning: each glyph at the sum of the advances before it.
    void DrawText(Renderer2D &renderer, const char32_t *text, Vec2f baseline, float size,
                  const Paint2D &paint)
    {
      const auto &face = LiberationSans();
      List<uint32> glyphs;
      List<Vec2f> positions;
      float x = 0.f;
      for (const char32_t *character = text; *character != U'\0'; ++character)
      {
        glyphs.push_back(face.GlyphIndex(*character));
        positions.push_back({x, 0.f});
        x += face.Advance(glyphs.back(), size);
      }
      renderer.DrawGlyphs(face, size, glyphs, positions, baseline, paint);
    }

    /// @brief `golden/scenes/text.html`: Liberation Sans at several sizes, dark on light and light on dark.
    void TextScene(Renderer2D &renderer)
    {
      renderer.FillRect({0.f, 0.f, 360.f, 200.f}, Rgb(0xffffff));
      DrawText(renderer, U"The quick brown fox jumps over the lazy dog", {10.f, 20.f}, 12.f, Rgb(0x222222));
      DrawText(renderer, U"The quick brown fox jumps over the lazy", {10.f, 44.f}, 16.f, Rgb(0x222222));
      DrawText(renderer, U"Krystal Engine 0123456789", {10.f, 78.f}, 24.f, Rgb(0x222222));
      DrawText(renderer, U"Hamburgefonstiv", {10.f, 140.f}, 48.f, Rgb(0x4f6df5));
      renderer.FillRect({0.f, 160.f, 360.f, 40.f}, Rgb(0x23283a));
      DrawText(renderer, U"Light on dark, at 14 pixels", {10.f, 185.f}, 14.f, Rgb(0xffffff));
    }

    struct Scene
    {
      const char *Name;
      uint32 Width;
      uint32 Height;
      void (*Draw)(Renderer2D &);
      /// @brief Whether Chrome's golden is only reported, not checked: Chrome on Windows draws text with
      /// DirectWrite, which hints and weighs glyphs differently from FreeType.
      bool ChromeReportOnly {false};
    };

    KRYS_NODISCARD GoldenImage Render(Device &device, Renderer2D &renderer, const Scene &scene)
    {
      auto target = device.CreateTexture({.Format = TextureFormat::RGBA8Unorm,
                                          .Width = scene.Width,
                                          .Height = scene.Height,
                                          .Usage = TextureUsage::RenderTarget | TextureUsage::Sampled,
                                          .DebugName = u8"Golden"});
      REQUIRE(target.has_value());
      device.BeginFrame();
      auto &commands = device.GetCommandBuffer();
      renderer.Begin(commands, {.Texture = *target,
                                .Width = scene.Width,
                                .Height = scene.Height,
                                .Format = TextureFormat::RGBA8Unorm,
                                .ClearColour = Array<float, 4> {0.f, 0.f, 0.f, 0.f}});
      scene.Draw(renderer);
      renderer.End();
      device.Submit(commands);
      device.EndFrame();

      GoldenImage image {.Width = scene.Width, .Height = scene.Height, .Pixels = {}};
      image.Pixels.resize(static_cast<size_t>(scene.Width) * scene.Height);
      device.ReadTexture(*target, TextureRegion::Whole2D(scene.Width, scene.Height),
                         Span<byte>(reinterpret_cast<byte *>(image.Pixels.data()), image.Pixels.size() * 4u));
      device.Destroy(*target);
      return image;
    }

    /// @brief Checks a scene against Render2D's golden (writing it when updating or missing) and against
    /// Chrome's, writing the actual image and the differences to the output directory either way.
    void CheckScene(const Scene &scene)
    {
      auto device = CreateTestDevice();
      auto renderer = Renderer2D::Create(*device);
      REQUIRE(renderer.has_value());
      const auto actual = Render(*device, **renderer, scene);
      const std::string name = scene.Name;
      const auto output = GoldenOutputDirectory();
      CHECK(SavePng(output / (name + ".actual.png"), actual));

      const auto ownPath = GoldenDirectory() / (name + ".png");
      const auto own = LoadPng(ownPath);
      if (UpdatingGoldens() || !own)
      {
        WARN("Writing Render2D's golden for " << name << "; review it before committing.");
        CHECK(SavePng(ownPath, actual));
      }
      else
      {
        const auto comparison = CompareImages(actual, *own, OwnTolerance);
        INFO(name << " against Render2D's golden: largest difference " << comparison.LargestDifference << ", "
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
        WARN("No Chrome golden for " << name << "; run tools/golden/capture_chrome.py.");
        return;
      }
      const auto comparison = CompareImages(actual, *chrome, ChromeTolerance);
      SavePng(output / (name + ".chrome-diff.png"), DifferenceImage(actual, *chrome));
      WARN(name << " against Chrome: largest difference " << comparison.LargestDifference << ", mean "
                << comparison.MeanDifference << ", " << comparison.PixelsOver << " of "
                << comparison.PixelCount << " pixels over " << ChromeTolerance);
      CHECK(comparison.SizesMatch);
      if (scene.ChromeReportOnly)
      {
        return;
      }
      // Antialiased curves differ most (Chrome antialiases some edges twice, through a clip and a fill), so a
      // few pixels may differ by more; on average, images should be within a step or so.
      CHECK(comparison.PixelsOver <= comparison.PixelCount / 200u);
      CHECK(comparison.MeanDifference < 1.5);
    }
  }

  TEST_CASE("Golden: shadows", "[Gfx][Render2D][GPU][Golden]")
  {
    CheckScene({.Name = "shadows", .Width = 320u, .Height = 240u, .Draw = ShadowsScene});
  }

  TEST_CASE("Golden: a conic gradient", "[Gfx][Render2D][GPU][Golden]")
  {
    CheckScene({.Name = "conic", .Width = 200u, .Height = 200u, .Draw = ConicScene});
  }

  TEST_CASE("Golden: text", "[Gfx][Render2D][GPU][Golden]")
  {
    CheckScene({.Name = "text", .Width = 360u, .Height = 200u, .Draw = TextScene, .ChromeReportOnly = true});
  }

  TEST_CASE("Golden: a UI", "[Gfx][Render2D][GPU][Golden]")
  {
    CheckScene({.Name = "ui", .Width = 360u, .Height = 260u, .Draw = UiScene});
  }
}
