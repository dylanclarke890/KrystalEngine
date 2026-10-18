#include "Krystal.Text/FreeTypeFace.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include <cstring>
#include <filesystem>
#include <freetype/freetype.h>
#include <fstream>
#include <hb.h>
#include <iterator>

namespace krys::text::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<FontFileData> LiberationSans()
    {
      std::ifstream file(std::filesystem::path {KRYS_TEXT_TESTS_FONTS_DIR} / "LiberationSans-Regular.ttf",
                         std::ios::binary);
      REQUIRE(file);
      const List<char> bytes {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
      List<byte> data(bytes.size());
      std::memcpy(data.data(), bytes.data(), bytes.size());
      return FontFileData::Create(krys::move(data));
    }

    constexpr uint32 Tag(char a, char b, char c, char d) noexcept
    {
      return (static_cast<uint32>(a) << 24) | (static_cast<uint32>(b) << 16) | (static_cast<uint32>(c) << 8)
             | static_cast<uint32>(d);
    }
  }

  TEST_CASE("FreeTypeFace: opens a face", "[Text]")
  {
    auto data = LiberationSans();
    CHECK(FreeTypeFace::FaceCount(*data) == 1u);
    auto face = FreeTypeFace::Create(data, 0);
    REQUIRE(face);

    CHECK(face->FamilyName() == u8"Liberation Sans");
    CHECK(face->StyleName() == u8"Regular");
    CHECK(face->PostScriptName() == u8"LiberationSans");
    CHECK(face->Style() == FaceStyle {.Weight = 400.f, .Width = 100.f, .Italic = false});
    CHECK_FALSE(face->IsFixedPitch());
    CHECK(face->UnitsPerEm() == 2'048u);
    REQUIRE(face->Face() != nullptr);
    // No size is set: users measure in font units or make sizes of their own.
    CHECK((face->Face()->size == nullptr || face->Face()->size->metrics.x_ppem == 0u));
  }

  TEST_CASE("FreeTypeFace: maps characters to glyphs", "[Text]")
  {
    auto face = FreeTypeFace::Create(LiberationSans(), 0);
    REQUIRE(face);
    const uint32 a = face->GlyphIndex(U'A');
    CHECK(a != 0u);
    CHECK(face->GlyphIndex(U'B') != 0u);
    CHECK(face->GlyphIndex(U'B') != a);
    // Liberation Sans has no Arabic, and no variant of A.
    CHECK(face->GlyphIndex(U'ب') == 0u);
    CHECK(face->GlyphIndex(U'A', U'️') == 0u);
  }

  TEST_CASE("FreeTypeFace: reads tables and makes a HarfBuzz face", "[Text]")
  {
    auto face = FreeTypeFace::Create(LiberationSans(), 0);
    REQUIRE(face);
    auto head = face->OpenTypeTable(Tag('h', 'e', 'a', 'd'));
    REQUIRE(head);
    CHECK(head->size() == 54u);
    CHECK(face->OpenTypeTable(Tag('O', 'S', '/', '2')));
    CHECK_FALSE(face->OpenTypeTable(Tag('x', 'x', 'x', 'x')));

    auto *harfBuzz = face->HarfBuzzFace();
    REQUIRE(harfBuzz != nullptr);
    CHECK(face->HarfBuzzFace() == harfBuzz);
    CHECK(hb_face_get_glyph_count(harfBuzz) == static_cast<unsigned>(face->Face()->num_glyphs));
    CHECK(hb_face_get_upem(harfBuzz) == face->UnitsPerEm());
  }

  TEST_CASE("FreeTypeFace: refuses what isn't a font", "[Text]")
  {
    auto data = FontFileData::Create(List<byte>(16, byte {0}));
    CHECK(FreeTypeFace::FaceCount(*data) == 0u);
    CHECK_FALSE(FreeTypeFace::Create(data, 0));
    CHECK_FALSE(FreeTypeFace::Create(LiberationSans(), 1));
  }
}
