#include "Krystal.Gfx/Render2D/FontFace2D.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Gfx/Render2D/PathRasteriser.hpp"
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ft2build.h>
#include <iterator>
#include <numbers>
#include FT_FREETYPE_H

namespace krys::gfx::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<text::FontFileData> ReadFont(const std::filesystem::path &path)
    {
      std::ifstream file(path, std::ios::binary);
      REQUIRE(file);
      const List<char> bytes {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
      List<byte> data(bytes.size());
      std::memcpy(data.data(), bytes.data(), bytes.size());
      return text::FontFileData::Create(krys::move(data));
    }

    KRYS_NODISCARD Ref<text::FreeTypeFace> LiberationSans()
    {
      static const auto face = []
      {
        auto created = text::FreeTypeFace::Create(
          ReadFont(std::filesystem::path {KRYS_GFX_TESTS_FONTS_DIR} / "LiberationSans-Regular.ttf"), 0);
        REQUIRE(created);
        Ref<text::FreeTypeFace> result = krys::move(created);
        return result;
      }();
      return face;
    }

    KRYS_NODISCARD UniquePtr<FontFace2D> Face(const FontFace2DOptions &options = {})
    {
      auto face = FontFace2D::Create(LiberationSans(), options);
      REQUIRE(face.has_value());
      return krys::move(*face);
    }

    KRYS_NODISCARD double Total(const GlyphBitmap &bitmap)
    {
      double total = 0.0;
      for (const auto value : bitmap.Pixels)
      {
        total += value / 255.0;
      }
      return total;
    }

    /// @brief The coverage-weighted mean position of the pixels' centres, relative to the pen's pixel.
    KRYS_NODISCARD Vec2f Centroid(const GlyphBitmap &bitmap, uint32 firstRow = 0u, uint32 lastRow = ~0u)
    {
      double x = 0.0;
      double y = 0.0;
      double total = 0.0;
      for (uint32 row = firstRow; row < bitmap.Bounds.Height && row < lastRow; ++row)
      {
        for (uint32 column = 0u; column < bitmap.Bounds.Width; ++column)
        {
          const double value = bitmap.Pixels[row * bitmap.Bounds.Width + column] / 255.0;
          x += value * (bitmap.Bounds.X + static_cast<int32>(column) + 0.5);
          y += value * (bitmap.Bounds.Y + static_cast<int32>(row) + 0.5);
          total += value;
        }
      }
      return {static_cast<float>(x / total), static_cast<float>(y / total)};
    }
  }

  TEST_CASE("FontFace2D: rasterises as FreeType does", "[Gfx][Render2D]")
  {
    // The same face, opened and rendered with FreeType directly.
    FT_Library library = nullptr;
    REQUIRE(FT_Init_FreeType(&library) == 0);
    const auto data =
      ReadFont(std::filesystem::path {KRYS_GFX_TESTS_FONTS_DIR} / "LiberationSans-Regular.ttf");
    FT_Face reference = nullptr;
    REQUIRE(FT_New_Memory_Face(library, reinterpret_cast<const FT_Byte *>(data->Bytes().data()),
                               static_cast<FT_Long>(data->Bytes().size()), 0, &reference)
            == 0);
    REQUIRE(FT_Set_Char_Size(reference, 0, 16 * 64, 72, 72) == 0);

    auto face = Face({.Hinting = FontHinting::None});
    for (const char32 character : {U'H', U'g', U'@'})
    {
      INFO("character " << static_cast<uint32>(character));
      const uint32 glyph = face->GlyphIndex(character);
      REQUIRE(FT_Load_Glyph(reference, glyph, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP) == 0);
      REQUIRE(FT_Render_Glyph(reference->glyph, FT_RENDER_MODE_NORMAL) == 0);
      const auto &expected = reference->glyph->bitmap;

      const auto bitmap = face->Rasterise(glyph, 16.f, 0.f);
      REQUIRE(bitmap.has_value());
      CHECK_FALSE(bitmap->Colour);
      CHECK(bitmap->Bounds.X == reference->glyph->bitmap_left);
      CHECK(bitmap->Bounds.Y == -reference->glyph->bitmap_top);
      REQUIRE(bitmap->Bounds.Width == expected.width);
      REQUIRE(bitmap->Bounds.Height == expected.rows);
      bool same = true;
      for (uint32 row = 0u; row < expected.rows; ++row)
      {
        for (uint32 column = 0u; column < expected.width; ++column)
        {
          same = same
                 && bitmap->Pixels[row * expected.width + column]
                      == expected.buffer[static_cast<ptrdiff_t>(row) * expected.pitch + column];
        }
      }
      CHECK(same);
    }

    FT_Done_Face(reference);
    FT_Done_FreeType(library);
  }

  TEST_CASE("FontFace2D: places glyphs on the baseline and within the pixel", "[Gfx][Render2D]")
  {
    auto face = Face();
    const uint32 h = face->GlyphIndex(U'H');

    // H sits on the baseline, above it.
    const auto upright = face->Rasterise(h, 32.f, 0.f);
    REQUIRE(upright.has_value());
    CHECK(upright->Bounds.Y < 0);
    CHECK(upright->Bounds.Y + static_cast<int32>(upright->Bounds.Height) == 0);

    // Half a pixel along moves it half a pixel, coverage and all.
    const auto moved = face->Rasterise(h, 32.f, 0.5f);
    REQUIRE(moved.has_value());
    CHECK(std::abs(Centroid(*moved).x - Centroid(*upright).x - 0.5f) < 0.05f);
    CHECK(std::abs(Centroid(*moved).y - Centroid(*upright).y) < 0.01f);
    CHECK(std::abs(Total(*moved) - Total(*upright)) < Total(*upright) * 0.01);

    // A space has nothing to draw.
    const auto space = face->Rasterise(face->GlyphIndex(U' '), 32.f, 0.f);
    REQUIRE(space.has_value());
    CHECK(space->Bounds.IsEmpty());
    CHECK(space->Pixels.empty());
    CHECK(face->GlyphPath(face->GlyphIndex(U' '), 32.f).IsEmpty());

    CHECK_FALSE(face->Rasterise(h, 0.f, 0.f).has_value());
    CHECK_FALSE(face->Rasterise(h, std::numeric_limits<float>::quiet_NaN(), 0.f).has_value());
  }

  TEST_CASE("FontFace2D: hints vertically only by default", "[Gfx][Render2D]")
  {
    const auto unhinted = Face({.Hinting = FontHinting::None});
    const auto slight = Face({.Hinting = FontHinting::Slight});
    const auto full = Face({.Hinting = FontHinting::Full});
    const uint32 h = slight->GlyphIndex(U'H');

    // At 11 pixels the H's top falls 0.57 of the way down a row. Slight hinting moves it to the row's edge,
    // so the top row is as dense as a row through the stems; unhinted, it's only partly covered. Hinting is
    // only vertical: the glyph keeps its horizontal positions.
    const auto snapped = slight->Rasterise(h, 11.f, 0.f);
    const auto exact = unhinted->Rasterise(h, 11.f, 0.f);
    const auto both = full->Rasterise(h, 11.f, 0.f);
    REQUIRE((snapped && exact && both));
    auto row = [](const GlyphBitmap &bitmap, uint32 index)
    {
      double total = 0.0;
      for (uint32 column = 0u; column < bitmap.Bounds.Width; ++column)
      {
        total += bitmap.Pixels[index * bitmap.Bounds.Width + column] / 255.0;
      }
      return total;
    };
    CHECK(row(*snapped, 0u) > row(*snapped, 1u) * 0.9);
    CHECK(row(*exact, 0u) < row(*exact, 1u) * 0.8);
    CHECK(std::abs(Centroid(*snapped).x - Centroid(*exact).x) < 0.1f);
    CHECK(both->Pixels != snapped->Pixels);
  }

  TEST_CASE("FontFace2D: synthesises bold and oblique", "[Gfx][Render2D]")
  {
    const auto regular = Face({.Hinting = FontHinting::None});
    const auto bold = Face({.Hinting = FontHinting::None, .SyntheticBold = true});
    const auto oblique = Face({.Hinting = FontHinting::None, .SyntheticOblique = true});
    const uint32 l = regular->GlyphIndex(U'l');

    const auto thin = regular->Rasterise(l, 48.f, 0.f);
    const auto thick = bold->Rasterise(l, 48.f, 0.f);
    REQUIRE((thin && thick));
    // A 24th of the em wider: 2 pixels on a stem about 5 wide.
    CHECK(Total(*thick) > Total(*thin) * 1.25);

    // Slanted right: the top of the l is further right than its bottom.
    const auto slanted = oblique->Rasterise(l, 48.f, 0.f);
    REQUIRE(slanted.has_value());
    const uint32 rows = slanted->Bounds.Height;
    const float top = Centroid(*slanted, 0u, rows / 4u).x;
    const float bottom = Centroid(*slanted, rows * 3u / 4u, rows).x;
    CHECK(top - bottom > static_cast<float>(rows) * 0.5f * 0.25f * 0.9f);
  }

  TEST_CASE("FontFace2D: rasterises under a transform", "[Gfx][Render2D]")
  {
    const auto face = Face({.Hinting = FontHinting::None});
    const uint32 l = face->GlyphIndex(U'l');
    const auto upright = face->Rasterise(l, 32.f, 0.f);
    // A quarter turn clockwise lays the l along x, its top to the right... of the pen: y down, x right.
    const auto turned = face->Rasterise(l, 32.f, 0.f, Transform2D::Rotation(std::numbers::pi_v<float> / 2.f));
    REQUIRE((upright && turned));
    CHECK(turned->Bounds.Width == upright->Bounds.Height);
    CHECK(turned->Bounds.Height == upright->Bounds.Width);
    CHECK(std::abs(Total(*turned) - Total(*upright)) < Total(*upright) * 0.02);
    // Doubling the scale doubles the height.
    const auto scaled = face->Rasterise(l, 32.f, 0.f, Transform2D::Scale(1.f, 2.f));
    REQUIRE(scaled.has_value());
    CHECK(std::abs(static_cast<int>(scaled->Bounds.Height) - 2 * static_cast<int>(upright->Bounds.Height))
          <= 1);
  }

  TEST_CASE("FontFace2D: measures and outlines glyphs", "[Gfx][Render2D]")
  {
    const auto face = Face({.Hinting = FontHinting::None});
    const uint32 h = face->GlyphIndex(U'H');
    CHECK(face->GlyphIndex(U'\u0628') == 0u);

    // Advances scale linearly with the size.
    const float advance = face->Advance(h, 10.f);
    CHECK(advance > 5.f);
    CHECK(std::abs(face->Advance(h, 20.f) - 2.f * advance) < 1e-4f);

    // The outline covers what the rasterised glyph does.
    const auto path = face->GlyphPath(face->GlyphIndex(U'g'), 40.f);
    REQUIRE_FALSE(path.IsEmpty());
    const auto bitmap = face->Rasterise(face->GlyphIndex(U'g'), 40.f, 0.f);
    REQUIRE(bitmap.has_value());
    PathRasteriser rasteriser;
    const auto mask = rasteriser.Rasterise(path, Transform2D::Identity(), FillRule::NonZero,
                                           {.X = -100, .Y = -100, .Width = 200u, .Height = 200u});
    double area = 0.0;
    for (const auto value : mask.Coverage)
    {
      area += value / 255.0;
    }
    CHECK(std::abs(area - Total(*bitmap)) < Total(*bitmap) * 0.02);
    CHECK(mask.Bounds.Y == bitmap->Bounds.Y);
    CHECK(mask.Bounds.X == bitmap->Bounds.X);
  }

  TEST_CASE("FontFace2D: rasterises colour glyphs as RGBA", "[Gfx][Render2D]")
  {
    const std::filesystem::path emoji = "C:/Windows/Fonts/seguiemj.ttf";
    if (!std::filesystem::exists(emoji))
    {
      SKIP("No colour font on this machine.");
    }
    auto created = text::FreeTypeFace::Create(ReadFont(emoji), 0);
    REQUIRE(created);
    auto face = FontFace2D::Create(Ref<text::FreeTypeFace> {krys::move(created)});
    REQUIRE(face.has_value());
    const auto bitmap = (*face)->Rasterise((*face)->GlyphIndex(U'\U0001F600'), 32.f, 0.f);
    REQUIRE(bitmap.has_value());
    CHECK(bitmap->Colour);
    CHECK(bitmap->Pixels.size() == static_cast<size_t>(bitmap->Bounds.Width) * bitmap->Bounds.Height * 4u);
  }
}
