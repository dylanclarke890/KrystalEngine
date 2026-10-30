#include "Krystal.Gfx/Render2D/GlyphAtlas.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Gfx.Tests/TestDevice.hpp"
#include "Krystal.Gfx/Render2D/CoverageBlur.hpp"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace krys::gfx::tests
{
  namespace
  {
    KRYS_NODISCARD UniquePtr<FontFace2D> LiberationSansFace()
    {
      std::ifstream file(std::filesystem::path {KRYS_GFX_TESTS_FONTS_DIR} / "LiberationSans-Regular.ttf",
                         std::ios::binary);
      REQUIRE(file);
      const List<char> bytes {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
      List<byte> data(bytes.size());
      std::memcpy(data.data(), bytes.data(), bytes.size());
      auto face = text::FreeTypeFace::Create(text::FontFileData::Create(krys::move(data)), 0);
      REQUIRE(face);
      auto created = FontFace2D::Create(Ref<text::FreeTypeFace> {krys::move(face)});
      REQUIRE(created.has_value());
      return krys::move(*created);
    }
  }

  TEST_CASE("GlyphAtlas: snaps pens to quarters of a pixel", "[Gfx][Render2D]")
  {
    using Pen = std::pair<int32, uint32>;
    CHECK(GlyphAtlas::SnapPen(0.f) == Pen {0, 0u});
    CHECK(GlyphAtlas::SnapPen(0.1f) == Pen {0, 0u});
    CHECK(GlyphAtlas::SnapPen(0.2f) == Pen {0, 1u});
    CHECK(GlyphAtlas::SnapPen(10.5f) == Pen {10, 2u});
    // Rounding up to the next pixel puts the pen in it.
    CHECK(GlyphAtlas::SnapPen(0.9f) == Pen {1, 0u});
    CHECK(GlyphAtlas::SnapPen(-0.1f) == Pen {0, 0u});
    CHECK(GlyphAtlas::SnapPen(-0.3f) == Pen {-1, 3u});
  }

  TEST_CASE("GlyphAtlas: caches glyphs", "[Gfx][Render2D][GPU]")
  {
    auto device = CreateTestDevice();
    auto face = LiberationSansFace();
    GlyphAtlas atlas(*device, {});
    const uint32 a = face->GlyphIndex(U'a');

    device->BeginFrame();
    const auto first = atlas.GetGlyph(*face, a, 16.f, 0u);
    const auto again = atlas.GetGlyph(*face, a, 16.f, 0u);
    REQUIRE((first && again));
    CHECK(first->Texture.IsValid());
    CHECK(first->Texture == again->Texture);
    CHECK(first->Texels == again->Texels);
    CHECK(first->Bounds == again->Bounds);
    CHECK_FALSE(first->Colour);
    CHECK(atlas.Stats().Rasterised == 1u);
    CHECK(atlas.Stats().Hits == 1u);

    // Each quarter of a pixel, size, transform and face is a glyph of its own.
    CHECK(atlas.GetGlyph(*face, a, 16.f, 1u)->Texels != first->Texels);
    CHECK(atlas.GetGlyph(*face, a, 17.f, 0u)->Texels != first->Texels);
    CHECK(atlas.GetGlyph(*face, a, 16.f, 0u, Transform2D::Scale(2.f, 2.f))->Texels != first->Texels);
    auto other = LiberationSansFace();
    CHECK(atlas.GetGlyph(*other, a, 16.f, 0u)->Texels != first->Texels);
    CHECK(atlas.Stats().Rasterised == 5u);

    // A blurred glyph, for shadows, is another: coverage grown by the blur's reach.
    const auto blurred = atlas.GetGlyph(*face, a, 16.f, 0u, {}, 2.f);
    REQUIRE(blurred.has_value());
    CHECK_FALSE(blurred->Colour);
    CHECK(blurred->Bounds.Width == first->Bounds.Width + 2u * detail::BlurExtent(2.f));
    CHECK(blurred->Bounds.X == first->Bounds.X - static_cast<int32>(detail::BlurExtent(2.f)));
    CHECK(atlas.Stats().Rasterised == 6u);

    // A space has no pixels, so no texture.
    const auto space = atlas.GetGlyph(*face, face->GlyphIndex(U' '), 16.f, 0u);
    REQUIRE(space.has_value());
    CHECK_FALSE(space->Texture.IsValid());
    device->EndFrame();
  }

  TEST_CASE("GlyphAtlas: writes the rasterised coverage", "[Gfx][Render2D][GPU]")
  {
    auto device = CreateTestDevice();
    auto face = LiberationSansFace();
    GlyphAtlas atlas(*device, {.PageSize = 128u, .MaxPages = 1u, .MaxColourPages = 1u});
    const uint32 g = face->GlyphIndex(U'g');

    device->BeginFrame();
    const auto location = atlas.GetGlyph(*face, g, 24.f, 2u);
    REQUIRE(location.has_value());
    const auto bitmap = face->Rasterise(g, 24.f, 0.5f);
    REQUIRE(bitmap.has_value());
    CHECK(location->Bounds == bitmap->Bounds);

    List<byte> texels(static_cast<size_t>(location->Texels.Width) * location->Texels.Height);
    device->ReadTexture(location->Texture,
                        {.X = static_cast<uint32>(location->Texels.X),
                         .Y = static_cast<uint32>(location->Texels.Y),
                         .Width = location->Texels.Width,
                         .Height = location->Texels.Height},
                        texels);
    CHECK(std::memcmp(texels.data(), bitmap->Pixels.data(), texels.size()) == 0);
    device->EndFrame();
  }

  TEST_CASE("GlyphAtlas: makes room by clearing pages", "[Gfx][Render2D][GPU]")
  {
    auto device = CreateTestDevice();
    auto face = LiberationSansFace();
    // One small page: a few large glyphs fill it.
    GlyphAtlas atlas(*device, {.PageSize = 64u, .MaxPages = 1u, .MaxColourPages = 1u});
    const char32 letters[] {U'A', U'B', U'C', U'D', U'E', U'F', U'G', U'H'};

    // Within a frame, a full page drawn from can't be cleared, so later glyphs find no room.
    device->BeginFrame();
    uint32 placed = 0u;
    for (const auto letter : letters)
    {
      placed += atlas.GetGlyph(*face, face->GlyphIndex(letter), 30.f, 0u) ? 1u : 0u;
    }
    CHECK(placed > 0u);
    CHECK(placed < 8u);
    CHECK(atlas.Stats().PageEvictions == 0u);
    device->EndFrame();

    // A later frame clears the page for them, and what was on it is rasterised again when needed.
    device->BeginFrame();
    for (const auto letter : letters)
    {
      CHECK(atlas.GetGlyph(*face, face->GlyphIndex(letter), 30.f, 0u).has_value());
      device->EndFrame();
      device->BeginFrame();
    }
    CHECK(atlas.Stats().PageEvictions > 0u);
    CHECK(atlas.PageCount() == 1u);
    device->EndFrame();
  }
}
