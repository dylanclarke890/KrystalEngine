#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Gfx.Tests/TestDevice.hpp"
#include "Krystal.Gfx/Render2D/CoverageBlur.hpp"
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
    using Pixel = Array<uint8, 4>;

    KRYS_NODISCARD Ref<text::FreeTypeFace> ReadFace(const std::filesystem::path &path)
    {
      std::ifstream file(path, std::ios::binary);
      REQUIRE(file);
      const List<char> bytes {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
      List<byte> data(bytes.size());
      std::memcpy(data.data(), bytes.data(), bytes.size());
      auto face = text::FreeTypeFace::Create(text::FontFileData::Create(krys::move(data)), 0);
      REQUIRE(face);
      return Ref<text::FreeTypeFace> {krys::move(face)};
    }

    class TextFixture
    {
    protected:
      UniquePtr<Device> _device;
      UniquePtr<Renderer2D> _renderer;
      UniquePtr<FontFace2D> _face;
      TextureHandle _target {};
      uint32 _width {64u};
      uint32 _height {32u};

    public:
      TextFixture() : _device(CreateTestDevice())
      {
        auto renderer = Renderer2D::Create(*_device);
        REQUIRE(renderer.has_value());
        _renderer = krys::move(*renderer);
        auto face = FontFace2D::Create(
          ReadFace(std::filesystem::path {KRYS_GFX_TESTS_FONTS_DIR} / "LiberationSans-Regular.ttf"));
        REQUIRE(face.has_value());
        _face = krys::move(*face);
      }

      ~TextFixture()
      {
        _renderer = nullptr;
        if (_target.IsValid())
        {
          _device->Destroy(_target);
        }
      }

      void UseTarget(uint32 width, uint32 height)
      {
        if (_target.IsValid())
        {
          _device->Destroy(_target);
        }
        auto target = _device->CreateTexture({.Format = TextureFormat::RGBA8Unorm,
                                              .Width = width,
                                              .Height = height,
                                              .Usage = TextureUsage::RenderTarget | TextureUsage::Sampled,
                                              .DebugName = u8"Target"});
        REQUIRE(target.has_value());
        _target = *target;
        _width = width;
        _height = height;
      }

      template <typename TFunc>
      void Draw(TFunc &&draw)
      {
        _device->BeginFrame();
        auto &commands = _device->GetCommandBuffer();
        _renderer->Begin(commands, {.Texture = _target,
                                    .Width = _width,
                                    .Height = _height,
                                    .Format = TextureFormat::RGBA8Unorm,
                                    .ClearColour = Array<float, 4> {0.f, 0.f, 0.f, 0.f}});
        draw(*_renderer);
        _renderer->End();
        _device->Submit(commands);
        _device->EndFrame();
      }

      KRYS_NODISCARD List<Pixel> Read()
      {
        List<Pixel> pixels(_width * _height);
        _device->ReadTexture(
          _target, TextureRegion::Whole2D(_width, _height),
          Span<byte>(reinterpret_cast<byte *>(pixels.data()), pixels.size() * sizeof(Pixel)));
        return pixels;
      }

      /// @brief The total of one channel over the target, from 0 to 1 per pixel.
      KRYS_NODISCARD static double Total(const List<Pixel> &pixels, size_t channel)
      {
        double total = 0.0;
        for (const auto &pixel : pixels)
        {
          total += pixel[channel] / 255.0;
        }
        return total;
      }

      /// @brief Checks the target holds `bitmap`'s coverage in red, with its pen's pixel at (x, y), and
      /// nothing else.
      KRYS_NODISCARD bool Shows(const List<Pixel> &pixels, const GlyphBitmap &bitmap, int32 x, int32 y) const
      {
        for (int32 row = 0; row < static_cast<int32>(_height); ++row)
        {
          for (int32 column = 0; column < static_cast<int32>(_width); ++column)
          {
            const int32 u = column - x - bitmap.Bounds.X;
            const int32 v = row - y - bitmap.Bounds.Y;
            const bool inside = u >= 0 && v >= 0 && u < static_cast<int32>(bitmap.Bounds.Width)
                                && v < static_cast<int32>(bitmap.Bounds.Height);
            const uint8 coverage =
              inside ? bitmap.Pixels[static_cast<size_t>(v) * bitmap.Bounds.Width + static_cast<size_t>(u)]
                     : 0u;
            const auto &pixel = pixels[static_cast<size_t>(row) * _width + static_cast<size_t>(column)];
            if (pixel[0] != coverage || pixel[3] != coverage || pixel[1] != 0u || pixel[2] != 0u)
            {
              UNSCOPED_INFO("pixel " << column << ", " << row << ": " << int(pixel[0]) << " for "
                                     << int(coverage));
              return false;
            }
          }
        }
        return true;
      }
    };

    constexpr SRGBA<uint8> Red {255, 0, 0};
  }

  TEST_CASE_METHOD(TextFixture, "Renderer2D: draws glyphs from the atlas, pens snapped",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(48u, 32u);
    const uint32 g = _face->GlyphIndex(U'g');

    SECTION("On a whole pixel, the glyph's coverage exactly")
    {
      const uint32 glyphs[] {g};
      const Vec2f positions[] {{0.f, 0.f}};
      Draw([&](Renderer2D &renderer)
           { renderer.DrawGlyphs(*_face, 20.f, glyphs, positions, {10.f, 20.f}, Red); });
      const auto bitmap = _face->Rasterise(g, 20.f, 0.f);
      REQUIRE(bitmap.has_value());
      CHECK(Shows(Read(), *bitmap, 10, 20));
      CHECK(_renderer->GetStats().GlyphsRasterised == 1u);
    }

    SECTION("Across, to the nearest quarter of a pixel; down, to the nearest pixel")
    {
      const uint32 glyphs[] {g};
      const Vec2f positions[] {{0.3f, 0.4f}};
      Draw([&](Renderer2D &renderer)
           { renderer.DrawGlyphs(*_face, 20.f, glyphs, positions, {10.f, 20.f}, Red); });
      const auto bitmap = _face->Rasterise(g, 20.f, 0.25f);
      REQUIRE(bitmap.has_value());
      CHECK(Shows(Read(), *bitmap, 10, 20));
    }

    SECTION("Under a uniform scale, at the scaled size")
    {
      const uint32 glyphs[] {g};
      const Vec2f positions[] {{0.f, 0.f}};
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.Scale(2.f, 2.f);
          renderer.DrawGlyphs(*_face, 10.f, glyphs, positions, {5.f, 10.f}, Red);
        });
      const auto bitmap = _face->Rasterise(g, 20.f, 0.f);
      REQUIRE(bitmap.has_value());
      CHECK(Shows(Read(), *bitmap, 10, 20));
    }
  }

  TEST_CASE_METHOD(TextFixture, "Renderer2D: draws a run of glyphs in one batch, cached",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(96u, 32u);
    const char32 text[] {U'K', U'r', U'y', U's', U't', U'a', U'l'};
    List<uint32> glyphs;
    List<Vec2f> positions;
    float x = 0.f;
    for (const auto character : text)
    {
      glyphs.push_back(_face->GlyphIndex(character));
      positions.push_back({x, 0.f});
      x += _face->Advance(glyphs.back(), 16.f);
    }
    for (int frame = 0; frame < 2; ++frame)
    {
      Draw([&](Renderer2D &renderer)
           { renderer.DrawGlyphs(*_face, 16.f, glyphs, positions, {4.f, 22.f}, Red); });
    }
    CHECK(_renderer->GetStats().Batches == 1u);
    CHECK(_renderer->GetStats().Instances == 7u);
    CHECK(_renderer->GetStats().GlyphsRasterised == 0u);
    CHECK(_renderer->GetStats().GlyphCacheHits == 7u);
    CHECK(Total(Read(), 0) > 20.0);
  }

  TEST_CASE_METHOD(TextFixture, "Renderer2D: paints text with gradients, clips and transforms",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(64u, 64u);
    const uint32 glyphs[] {_face->GlyphIndex(U'H')};
    const Vec2f positions[] {{0.f, 0.f}};
    Draw([&](Renderer2D &renderer)
         { renderer.DrawGlyphs(*_face, 40.f, glyphs, positions, {12.f, 48.f}, Red); });
    const auto plain = Read();
    const double coverage = Total(plain, 3);
    REQUIRE(coverage > 50.0);

    SECTION("A gradient across the glyph")
    {
      const Gradient2D gradient {.Shape = LinearGradient2D {{12.f, 0.f}, {44.f, 0.f}},
                                 .Stops = {{.Offset = 0.f, .Colour = {1.f, 0.f, 0.f, 1.f}},
                                           {.Offset = 1.f, .Colour = {0.f, 0.f, 1.f, 1.f}}}};
      Draw([&](Renderer2D &renderer)
           { renderer.DrawGlyphs(*_face, 40.f, glyphs, positions, {12.f, 48.f}, gradient); });
      const auto pixels = Read();
      CHECK(std::abs(Total(pixels, 3) - coverage) < coverage * 0.01);
      // Red on the left stem, blue on the right.
      double leftRed = 0.0;
      double rightBlue = 0.0;
      for (uint32 y = 0u; y < 64u; ++y)
      {
        for (uint32 x = 0u; x < 64u; ++x)
        {
          const auto &pixel = pixels[y * 64u + x];
          (x < 24u ? leftRed : rightBlue) += x < 24u ? pixel[0] - pixel[2] : pixel[2] - pixel[0];
        }
      }
      CHECK(leftRed > 0.0);
      CHECK(rightBlue > 0.0);
    }

    SECTION("Clipped to the left half")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.ClipRect({0.f, 0.f, 32.f, 64.f});
          renderer.DrawGlyphs(*_face, 40.f, glyphs, positions, {12.f, 48.f}, Red);
        });
      const auto pixels = Read();
      for (uint32 y = 0u; y < 64u; ++y)
      {
        CHECK(pixels[y * 64u + 40u][3] == 0u);
      }
      CHECK(Total(pixels, 3) > coverage * 0.3);
    }

    SECTION("Turned, rasterised under the turn")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.Translate(32.f, 32.f);
          renderer.Rotate(std::numbers::pi_v<float> / 6.f);
          renderer.DrawGlyphs(*_face, 40.f, glyphs, positions, {-14.f, 14.f}, Red);
        });
      CHECK(std::abs(Total(Read(), 3) - coverage) < coverage * 0.03);
    }
  }

  TEST_CASE_METHOD(TextFixture, "Renderer2D: fills large text as outlines", "[Gfx][Render2D][GPU]")
  {
    UseTarget(320u, 320u);
    const uint32 glyphs[] {_face->GlyphIndex(U'l')};
    const Vec2f positions[] {{0.f, 0.f}};
    Draw([&](Renderer2D &renderer)
         { renderer.DrawGlyphs(*_face, 300.f, glyphs, positions, {100.f, 290.f}, Red); });
    CHECK(_renderer->GetStats().GlyphsRasterised == 0u);
    CHECK(_renderer->GetStats().MasksRasterised == 1u);
    // An l is a stem: about 0.07 of the em wide, 0.73 tall.
    const auto bitmap = _face->Rasterise(glyphs[0], 300.f, 0.f);
    REQUIRE(bitmap.has_value());
    double expected = 0.0;
    for (const auto value : bitmap->Pixels)
    {
      expected += value / 255.0;
    }
    CHECK(std::abs(Total(Read(), 3) - expected) < expected * 0.02);
  }

  TEST_CASE_METHOD(TextFixture, "Renderer2D: casts text shadows", "[Gfx][Render2D][GPU]")
  {
    UseTarget(64u, 48u);
    const uint32 glyphs[] {_face->GlyphIndex(U'H')};
    const Vec2f positions[] {{0.f, 0.f}};
    const auto bitmap = _face->Rasterise(glyphs[0], 24.f, 0.f);
    REQUIRE(bitmap.has_value());

    SECTION("A sharp shadow is the glyph again, beneath it")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.SetDropShadow({.Offset = {20.f, 0.f}, .Colour = SRGBA<uint8> {0, 0, 255}});
          renderer.DrawGlyphs(*_face, 24.f, glyphs, positions, {4.f, 32.f}, Red);
        });
      const auto pixels = Read();
      CHECK(Total(pixels, 2) > 0.0);
      // The shadow is the same glyph, in blue, 20 pixels along.
      bool same = true;
      for (uint32 v = 0u; v < bitmap->Bounds.Height; ++v)
      {
        for (uint32 u = 0u; u < bitmap->Bounds.Width; ++u)
        {
          const int32 x = 24 + bitmap->Bounds.X + static_cast<int32>(u);
          const int32 y = 32 + bitmap->Bounds.Y + static_cast<int32>(v);
          same = same
                 && pixels[static_cast<size_t>(y) * 64u + static_cast<size_t>(x)][2]
                      == bitmap->Pixels[v * bitmap->Bounds.Width + u];
        }
      }
      CHECK(same);
    }

    SECTION("A blurred shadow spreads the glyph's coverage")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.SetDropShadow({.Offset = {0.f, 0.f}, .BlurRadius = 6.f, .Colour = SRGBA<uint8> {0, 0, 0}});
          renderer.DrawGlyphs(*_face, 24.f, glyphs, positions, {16.f, 32.f}, SRGBA<uint8> {255, 0, 0, 0});
        });
      // A transparent paint casts no shadow.
      CHECK(Total(Read(), 3) == 0.0);
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.SetDropShadow({.Offset = {0.f, 0.f}, .BlurRadius = 6.f, .Colour = SRGBA<uint8> {0, 0, 0}});
          renderer.Translate(1'000.f, 0.f);
          renderer.SetDropShadow(
            {.Offset = {-1'000.f, 0.f}, .BlurRadius = 6.f, .Colour = SRGBA<uint8> {0, 0, 0}});
          renderer.DrawGlyphs(*_face, 24.f, glyphs, positions, {16.f, 32.f}, Red);
        });
      // Exactly the glyph's own coverage blurred, at its pen.
      const auto blurred = detail::BlurCoverage({.Bounds = bitmap->Bounds, .Coverage = bitmap->Pixels}, 3.f);
      const auto pixels = Read();
      bool same = true;
      for (uint32 row = 0u; row < _height; ++row)
      {
        for (uint32 column = 0u; column < _width; ++column)
        {
          const int32 u = static_cast<int32>(column) - 16 - blurred.Bounds.X;
          const int32 v = static_cast<int32>(row) - 32 - blurred.Bounds.Y;
          const bool inside = u >= 0 && v >= 0 && u < static_cast<int32>(blurred.Bounds.Width)
                              && v < static_cast<int32>(blurred.Bounds.Height);
          const uint8 expected =
            inside ? blurred.Coverage[static_cast<size_t>(v) * blurred.Bounds.Width + static_cast<size_t>(u)]
                   : 0u;
          same = same && pixels[row * _width + column][3] == expected;
        }
      }
      CHECK(same);
    }
  }

  TEST_CASE_METHOD(TextFixture, "Renderer2D: reuses large text's outlines when it moves by whole pixels",
                   "[Gfx][Render2D][GPU]")
  {
    UseTarget(320u, 320u);
    const uint32 glyphs[] {_face->GlyphIndex(U'l')};
    const Vec2f positions[] {{0.f, 0.f}};
    const Gradient2D gradient {.Shape = LinearGradient2D {{0.f, 0.f}, {320.f, 0.f}},
                               .Stops = {{.Offset = 0.f, .Colour = {1.f, 0.f, 0.f, 1.f}},
                                         {.Offset = 1.f, .Colour = {0.f, 0.f, 1.f, 1.f}}}};
    Draw([&](Renderer2D &renderer)
         { renderer.DrawGlyphs(*_face, 300.f, glyphs, positions, {100.f, 290.f}, gradient); });
    CHECK(_renderer->GetStats().MasksRasterised == 1u);
    const auto before = Read();
    Draw([&](Renderer2D &renderer)
         { renderer.DrawGlyphs(*_face, 300.f, glyphs, positions, {140.f, 290.f}, gradient); });
    CHECK(_renderer->GetStats().MasksRasterised == 0u);
    CHECK(_renderer->GetStats().MaskCacheHits == 1u);
    // The glyph moved; the gradient, in the current coordinates, didn't: further right, it's bluer.
    const auto after = Read();
    CHECK(std::abs(Total(after, 3) - Total(before, 3)) < 0.5);
    CHECK(Total(after, 2) > Total(before, 2));
  }

  TEST_CASE_METHOD(TextFixture, "Renderer2D: draws colour glyphs in their colours", "[Gfx][Render2D][GPU]")
  {
    const std::filesystem::path emoji = "C:/Windows/Fonts/seguiemj.ttf";
    if (!std::filesystem::exists(emoji))
    {
      SKIP("No colour font on this machine.");
    }
    auto face = FontFace2D::Create(ReadFace(emoji));
    REQUIRE(face.has_value());
    UseTarget(48u, 48u);
    const uint32 glyphs[] {(*face)->GlyphIndex(U'\U0001F600')};
    const Vec2f positions[] {{0.f, 0.f}};
    // The paint's colour doesn't tint it; its alpha fades it.
    Draw([&](Renderer2D &renderer)
         { renderer.DrawGlyphs(**face, 32.f, glyphs, positions, {8.f, 38.f}, SRGBA<uint8> {0, 0, 255}); });
    const auto opaque = Read();
    CHECK(Total(opaque, 0) > Total(opaque, 2));
    Draw(
      [&](Renderer2D &renderer)
      { renderer.DrawGlyphs(**face, 32.f, glyphs, positions, {8.f, 38.f}, SRGBA<uint8> {0, 0, 255, 128}); });
    CHECK(std::abs(Total(Read(), 3) - Total(opaque, 3) * 0.5) < Total(opaque, 3) * 0.02);
  }

  TEST_CASE_METHOD(TextFixture, "Renderer2D: casts colour glyphs' shadows from their alpha",
                   "[Gfx][Render2D][GPU]")
  {
    const std::filesystem::path emoji = "C:/Windows/Fonts/seguiemj.ttf";
    if (!std::filesystem::exists(emoji))
    {
      SKIP("No colour font on this machine.");
    }
    auto face = FontFace2D::Create(ReadFace(emoji));
    REQUIRE(face.has_value());
    UseTarget(96u, 48u);
    const uint32 glyphs[] {(*face)->GlyphIndex(U'\U0001F600')};
    const Vec2f positions[] {{0.f, 0.f}};
    const auto bitmap = (*face)->Rasterise(glyphs[0], 32.f, 0.f);
    REQUIRE(bitmap.has_value());
    REQUIRE(bitmap->Colour);

    SECTION("A sharp shadow is its silhouette in the shadow's colour")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.SetDropShadow({.Offset = {44.f, 0.f}, .Colour = SRGBA<uint8> {0, 0, 255}});
          renderer.DrawGlyphs(**face, 32.f, glyphs, positions, {4.f, 38.f}, SRGBA<uint8> {0, 0, 0});
        });
      const auto pixels = Read();
      bool same = true;
      for (uint32 v = 0u; v < bitmap->Bounds.Height; ++v)
      {
        for (uint32 u = 0u; u < bitmap->Bounds.Width; ++u)
        {
          const int32 x = 48 + bitmap->Bounds.X + static_cast<int32>(u);
          const int32 y = 38 + bitmap->Bounds.Y + static_cast<int32>(v);
          const auto &pixel = pixels[static_cast<size_t>(y) * 96u + static_cast<size_t>(x)];
          const uint8 alpha = bitmap->Pixels[(v * bitmap->Bounds.Width + u) * 4u + 3u];
          same = same && pixel[0] == 0u && pixel[1] == 0u && pixel[2] == alpha && pixel[3] == alpha;
        }
      }
      CHECK(same);
    }

    SECTION("A blurred shadow spreads its alpha")
    {
      Draw(
        [&](Renderer2D &renderer)
        {
          renderer.Translate(1'000.f, 0.f);
          renderer.SetDropShadow(
            {.Offset = {-1'000.f, 0.f}, .BlurRadius = 6.f, .Colour = SRGBA<uint8> {0, 0, 0}});
          renderer.DrawGlyphs(**face, 32.f, glyphs, positions, {24.f, 38.f}, SRGBA<uint8> {0, 0, 0});
        });
      double expected = 0.0;
      for (size_t i = 3u; i < bitmap->Pixels.size(); i += 4u)
      {
        expected += bitmap->Pixels[i] / 255.0;
      }
      CHECK(std::abs(Total(Read(), 3) - expected) < expected * 0.01);
    }
  }
}
