#include "Krystal.Booey/Graphics/OpenType/OpenTypeMathData.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCache.hpp"
#include "Krystal.Booey/Graphics/FontCustomPlatformData.hpp"
#include "Krystal.Booey/Graphics/FontDatabase.hpp"
#include "Krystal.Booey/Graphics/WOFFFileFormat.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace krys::boo::tests
{
  using Catch::Approx;
  using MathConstant = OpenTypeMathData::MathConstant;
  using AssemblyPart = OpenTypeMathData::AssemblyPart;

  namespace
  {
    /// @brief A 10px font (0.01px a unit, as the fonts' em is 1000 units) of the face in @p path, under the
    /// test data.
    KRYS_NODISCARD Ref<Font> LoadFont(const std::filesystem::path &path) noexcept
    {
      std::ifstream stream(std::filesystem::path {KRYS_BOOEY_TESTS_DATA_DIR} / path, std::ios::binary);
      REQUIRE(stream);
      List<uint8> bytes {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
      RefPtr<SharedBuffer> buffer = SharedBuffer::Create(krys::move(bytes));
      (void)ConvertWOFFToSfntIfNecessary(buffer);
      REQUIRE(buffer);
      auto data = FontCustomPlatformData::Create(*buffer, {});
      REQUIRE(data);
      return FontCache::Singleton().FontForPlatformData(
        FontPlatformData {Ref {data->Face}, 10, false, false});
    }

    KRYS_NODISCARD Ref<Font> LoadMathFont(const char *name) noexcept
    {
      return LoadFont(std::filesystem::path {"fonts"} / "math" / (std::string {name} + ".woff"));
    }
  }

  TEST_CASE("OpenTypeMathData: fonts without a MATH table have none", "[Graphics][OpenTypeMathData]")
  {
    auto ahem = LoadFont(std::filesystem::path {"fonts"} / "Ahem.ttf");
    CHECK(ahem->MathData() == nullptr);
    // Asking again doesn't find one either.
    CHECK(ahem->MathData() == nullptr);

    auto &liberation = FontDatabase::Singleton().BundledFace();
    auto text =
      FontCache::Singleton().FontForPlatformData(FontPlatformData {ShareRef(liberation), 16, false, false});
    CHECK(text->MathData() == nullptr);
  }

  TEST_CASE("OpenTypeMathData: constants", "[Graphics][OpenTypeMathData]")
  {
    SECTION("Lengths are scaled to the font's size")
    {
      auto font = LoadMathFont("fraction-rulethickness10000");
      const auto *math = font->MathData();
      REQUIRE(math);
      CHECK(math->HasMathData());
      CHECK(math->GetMathConstant(*font, MathConstant::FractionRuleThickness) == Approx(100));
      CHECK(math->GetMathConstant(*font, MathConstant::FractionNumeratorShiftUp) == 0);
      // The same data is returned each time.
      CHECK(font->MathData() == math);

      auto axis = LoadMathFont("axisheight5000-verticalarrow14000");
      REQUIRE(axis->MathData());
      CHECK(axis->MathData()->GetMathConstant(*axis, MathConstant::AxisHeight) == Approx(50));
    }

    SECTION("Percentages are fractions, whatever the size")
    {
      auto font = LoadMathFont("scriptpercentscaledown80-scriptscriptpercentscaledown40");
      REQUIRE(font->MathData());
      CHECK(font->MathData()->GetMathConstant(*font, MathConstant::ScriptPercentScaleDown) == Approx(0.8));
      CHECK(font->MathData()->GetMathConstant(*font, MathConstant::ScriptScriptPercentScaleDown)
            == Approx(0.4));

      auto radical = LoadMathFont("radical-degreebottomraisepercent25-rulethickness1000");
      REQUIRE(radical->MathData());
      CHECK(radical->MathData()->GetMathConstant(*radical, MathConstant::RadicalDegreeBottomRaisePercent)
            == Approx(0.25));
      CHECK(radical->MathData()->GetMathConstant(*radical, MathConstant::RadicalRuleThickness) == Approx(10));
    }
  }

  TEST_CASE("OpenTypeMathData: size variants and italic corrections", "[Graphics][OpenTypeMathData]")
  {
    auto font = LoadMathFont("largeop-displayoperatorminheight2000-2AFF-italiccorrection3000");
    const auto *math = font->MathData();
    REQUIRE(math);
    CHECK(math->GetMathConstant(*font, MathConstant::DisplayOperatorMinHeight) == Approx(20));

    auto base = font->GlyphForCharacter(U'⫿');
    REQUIRE(base != 0);
    List<Glyph> sizeVariants;
    List<AssemblyPart> assemblyParts;
    math->GetMathVariants(base, true, sizeVariants, assemblyParts);
    REQUIRE(sizeVariants.size() == 2);
    CHECK(sizeVariants[0] == base);
    CHECK(assemblyParts.empty());

    // The display variant has the italic correction; the base glyph has none.
    CHECK(math->GetItalicCorrection(*font, sizeVariants[1]) == Approx(30));
    CHECK(math->GetItalicCorrection(*font, base) == 0);

    // It has no horizontal variants.
    math->GetMathVariants(base, false, sizeVariants, assemblyParts);
    CHECK(sizeVariants.empty());
  }

  TEST_CASE("OpenTypeMathData: glyph assemblies", "[Graphics][OpenTypeMathData]")
  {
    auto font = LoadMathFont("stretchy");
    const auto *math = font->MathData();
    REQUIRE(math);

    // The size variants and parts are in private use code points: v0 to v3 at U+E000, U+E002, U+E004 and
    // U+E006, and h0 to h3 after each.
    auto glyph = [&](char32 character)
    {
      auto result = font->GlyphForCharacter(character);
      REQUIRE(result != 0);
      return result;
    };

    List<Glyph> sizeVariants;
    List<AssemblyPart> assemblyParts;

    SECTION("Vertically")
    {
      math->GetMathVariants(glyph(U'⥜'), true, sizeVariants, assemblyParts);
      CHECK(sizeVariants == List<Glyph> {glyph(0xE000), glyph(0xE002), glyph(0xE004), glyph(0xE006)});
      CHECK(assemblyParts
            == List<AssemblyPart> {{.Glyph = glyph(0xE004), .IsExtender = false},
                                   {.Glyph = glyph(0xE002), .IsExtender = true}});
    }

    SECTION("Horizontally")
    {
      math->GetMathVariants(glyph(U'⥚'), false, sizeVariants, assemblyParts);
      CHECK(sizeVariants == List<Glyph> {glyph(0xE001), glyph(0xE003), glyph(0xE005), glyph(0xE007)});
      CHECK(assemblyParts
            == List<AssemblyPart> {{.Glyph = glyph(0xE005), .IsExtender = false},
                                   {.Glyph = glyph(0xE003), .IsExtender = true}});
    }

    SECTION("Results replace what the lists held")
    {
      math->GetMathVariants(glyph(U'⥜'), true, sizeVariants, assemblyParts);
      math->GetMathVariants(glyph(U' '), true, sizeVariants, assemblyParts);
      CHECK(sizeVariants == List<Glyph> {glyph(U' ')});
      CHECK(assemblyParts.empty());
    }
  }

  TEST_CASE("OpenTypeMathData: the bundled math font", "[Graphics][OpenTypeMathData]")
  {
    auto &face = FontDatabase::Singleton().BundledMathFace();
    CHECK(face.FamilyName() == u8"STIX Two Math");

    auto font =
      FontCache::Singleton().FontForPlatformData(FontPlatformData {ShareRef(face), 20, false, false});
    const auto *math = font->MathData();
    REQUIRE(math);
    CHECK(math->GetMathConstant(*font, MathConstant::AxisHeight) > 0);
    CHECK(math->GetMathConstant(*font, MathConstant::FractionRuleThickness) > 0);
    auto scaleDown = math->GetMathConstant(*font, MathConstant::ScriptPercentScaleDown);
    CHECK(scaleDown > 0.5f);
    CHECK(scaleDown < 1);

    // Parentheses stretch through size variants and an assembly.
    List<Glyph> sizeVariants;
    List<AssemblyPart> assemblyParts;
    math->GetMathVariants(font->GlyphForCharacter(U'('), true, sizeVariants, assemblyParts);
    CHECK(sizeVariants.size() > 1);
    CHECK(assemblyParts.size() >= 3);
    CHECK(std::ranges::any_of(assemblyParts, &AssemblyPart::IsExtender));
  }

  TEST_CASE("FontDatabase: the bundled math font is the last fallback", "[Graphics][FontDatabase]")
  {
    auto &database = FontDatabase::Singleton();
    CHECK(database.FacesInFamily(u8"STIX Two Math").size() == 1);
    CHECK(&database.BundledMathFace() != &database.BundledFace());
  }
}
