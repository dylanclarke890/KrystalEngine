#include "Krystal.Booey/Graphics/FontDatabase.hpp"
#include "catch2/catch_test_macros.hpp"
#include <algorithm>

namespace krys::boo::tests
{
  namespace
  {
    const std::filesystem::path FontsDir = std::filesystem::path {KRYS_BOOEY_TESTS_DATA_DIR} / "fonts";
  }

  TEST_CASE("FontDatabase: the bundled font", "[Graphics][FontDatabase]")
  {
    auto &database = FontDatabase::Singleton();
    auto &bundled = database.BundledFace();
    CHECK(bundled.FamilyName() == u8"Liberation Sans");
    CHECK(bundled.StyleName() == u8"Regular");
    CHECK(bundled.PostScriptName() == u8"LiberationSans");
    CHECK(bundled.UnitsPerEm() == 2'048u);
    CHECK(CapabilitiesOfFace(bundled).Weight == FontSelectionRange {NormalWeightValue()});
    CHECK(CapabilitiesOfFace(bundled).Width == FontSelectionRange {NormalWidthValue()});
    CHECK(CapabilitiesOfFace(bundled).Slope == FontSelectionRange {NormalItalicValue()});
    CHECK_FALSE(bundled.IsFixedPitch());
    CHECK(database.Faces().front().get() == &bundled);

    // The bundled Noto Sans Arabic, the last resort for Arabic, is registered after it.
    auto &arabic = database.BundledArabicFace();
    CHECK(arabic.FamilyName() == u8"Noto Sans Arabic");
    CHECK(arabic.StyleName() == u8"Regular");
    CHECK(arabic.GlyphIndex(U'ا'));
    CHECK(database.Faces()[1].get() == &arabic);

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
}
