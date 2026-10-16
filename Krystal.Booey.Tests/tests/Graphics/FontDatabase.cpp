#include "Krystal.Booey/Graphics/FontDatabase.hpp"
#include "catch2/catch_test_macros.hpp"
#include <algorithm>

namespace krys::boo::tests
{
  namespace
  {
    const std::filesystem::path FontsDir = std::filesystem::path {KRYS_BOOEY_TESTS_DATA_DIR} / "fonts";

    KRYS_NODISCARD FontDatabase &DatabaseWithAhem()
    {
      auto &database = FontDatabase::Singleton();
      (void)database.AddFontFile(FontsDir / "Ahem.ttf");
      return database;
    }

    constexpr uint32 Tag(char a, char b, char c, char d) noexcept
    {
      return (static_cast<uint32>(a) << 24) | (static_cast<uint32>(b) << 16) | (static_cast<uint32>(c) << 8)
             | static_cast<uint32>(d);
    }
  }

  TEST_CASE("FontDatabase: the bundled font", "[Graphics][FontDatabase]")
  {
    auto &database = FontDatabase::Singleton();
    auto &bundled = database.BundledFace();
    CHECK(bundled.FamilyName() == u8"Liberation Sans");
    CHECK(bundled.StyleName() == u8"Regular");
    CHECK(bundled.PostScriptName() == u8"LiberationSans");
    CHECK(bundled.UnitsPerEm() == 2'048u);
    CHECK(bundled.Capabilities().Weight == FontSelectionRange {NormalWeightValue()});
    CHECK(bundled.Capabilities().Width == FontSelectionRange {NormalWidthValue()});
    CHECK(bundled.Capabilities().Slope == FontSelectionRange {NormalItalicValue()});
    CHECK_FALSE(bundled.IsFixedPitch());
    CHECK(database.Faces().front().get() == &bundled);

    // Families match ASCII case-insensitively.
    auto faces = database.FacesInFamily(u8"liberation SANS");
    REQUIRE(faces.size() == 1u);
    CHECK(faces.front().get() == &bundled);
    CHECK(database.MatchFace(u8"Liberation Sans", {BoldWeightValue(), NormalWidthValue(), ItalicValue()})
          == &bundled);
    CHECK_FALSE(database.MatchFace(u8"No Such Family", {NormalWeightValue(), NormalWidthValue(), null}));
  }

  TEST_CASE("FontDatabase: registering fonts", "[Graphics][FontDatabase]")
  {
    auto &database = FontDatabase::Singleton();
    (void)database.AddFontFile(FontsDir / "Ahem.ttf");
    auto generation = database.Generation();

    SECTION("A file is registered once")
    {
      CHECK(database.AddFontFile(FontsDir / "Ahem.ttf") == 0u);
      CHECK(database.AddFontFile(FontsDir / "." / "Ahem.ttf") == 0u);
      CHECK(database.AddFontDirectory(FontsDir) == 0u);
      CHECK(database.Generation() == generation);

      auto faces = database.FacesInFamily(u8"AHEM");
      REQUIRE(faces.size() == 1u);
      CHECK(faces.front()->FamilyName() == u8"Ahem");
      auto families = database.Families();
      CHECK(std::ranges::find(families, utf8_string {u8"Ahem"}) != families.end());
      CHECK(std::ranges::find(families, utf8_string {u8"Liberation Sans"}) != families.end());
    }

    SECTION("What isn't a font isn't registered")
    {
      CHECK(database.AddFontFile(FontsDir / "missing.ttf") == 0u);
      CHECK(database.AddFontFile(FontsDir / "README.md") == 0u);
      CHECK(database.AddFontData(List<byte>(64, byte {0x42})) == 0u);
      CHECK(database.AddFontDirectory(FontsDir / "missing") == 0u);
      CHECK(database.Generation() == generation);
    }
  }

  TEST_CASE("FreeTypeFace", "[Graphics][FontDatabase]")
  {
    auto &database = DatabaseWithAhem();
    auto faces = database.FacesInFamily(u8"Ahem");
    REQUIRE(faces.size() == 1u);
    auto &ahem = *faces.front();

    CHECK(ahem.UnitsPerEm() == 1'000u);
    CHECK(ahem.GlyphIndex(U'a') == 67u);
    CHECK(ahem.GlyphIndex(U'X') == 58u);
    CHECK(ahem.GlyphIndex(U'\u200B') == 253u);
    CHECK(ahem.GlyphIndex(U'\u6C34') == 270u);
    CHECK(ahem.GlyphIndex(U'\u0628') == 0u);
    CHECK(ahem.GlyphIndex(U'a', U'\uFE0F') == 0u);
    CHECK(ahem.OpenTypeTable(Tag('O', 'S', '/', '2')));
    auto head = database.BundledFace().OpenTypeTable(Tag('h', 'e', 'a', 'd'));
    REQUIRE(head);
    CHECK(head->size() == 54u);
    CHECK_FALSE(ahem.OpenTypeTable(Tag('x', 'x', 'x', 'x')));

    auto data = FontFileData::Create(List<byte>(16, byte {0}));
    CHECK(FreeTypeFace::FaceCount(*data) == 0u);
    CHECK_FALSE(FreeTypeFace::Create(data, 0));
  }
}
