#include "Krystal.Booey/Graphics/Font.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/FontDatabase.hpp"
#include "Krystal.Booey/Graphics/FontDescription.hpp"

namespace krys::boo::tests
{
  namespace
  {
    KRYS_NODISCARD text::FreeTypeFace &Ahem()
    {
      auto &database = FontDatabase::Singleton();
      (void)database.AddFontFile(std::filesystem::path {KRYS_BOOEY_TESTS_DATA_DIR} / "fonts" / "Ahem.ttf");
      auto faces = database.FacesInFamily(u8"Ahem");
      REQUIRE(faces.size() == 1u);
      return *faces.front();
    }

    KRYS_NODISCARD Ref<Font> CreateFont(text::FreeTypeFace &face, float size, bool syntheticBold = false,
                                        bool syntheticOblique = false)
    {
      return Font::Create(FontPlatformData {ShareRef(face), size, syntheticBold, syntheticOblique});
    }
  }

  TEST_CASE("Font: metrics", "[Graphics][Font]")
  {
    SECTION("Ahem")
    {
      auto font = CreateFont(Ahem(), 16);
      auto &metrics = font->GetFontMetrics();
      CHECK(metrics.UnitsPerEm() == 1'000u);
      CHECK(metrics.Ascent() == Catch::Approx(12.8f));
      CHECK(metrics.Descent() == Catch::Approx(3.2f));
      CHECK(metrics.LineGap() == 0);
      CHECK(metrics.LineSpacing() == 16);
      CHECK(*metrics.XHeight() == Catch::Approx(12.8f));
      CHECK(*metrics.CapHeight() == Catch::Approx(12.8f));
      CHECK(*metrics.ZeroWidth() == 16);
      // Ahem's water ideogram is 1em wide.
      CHECK(*metrics.IdeogramWidth() == 16);
      CHECK(*metrics.UnderlinePosition() == Catch::Approx(2.128f));
      CHECK(*metrics.UnderlineThickness() == Catch::Approx(0.32f));
      CHECK(font->SizePerUnit() == Catch::Approx(0.016f));
      CHECK(font->AvgCharWidth() == 16);
      CHECK(font->MaxCharWidth() == 16);
      CHECK_FALSE(font->AllowsAntialiasing());
    }

    SECTION("Liberation Sans")
    {
      auto font = CreateFont(FontDatabase::Singleton().BundledFace(), 16);
      auto &metrics = font->GetFontMetrics();
      CHECK(metrics.UnitsPerEm() == 2'048u);
      CHECK(metrics.Ascent() == Catch::Approx(14.484375f));
      CHECK(metrics.Descent() == Catch::Approx(3.390625f));
      CHECK(metrics.LineGap() == Catch::Approx(0.5234375f));
      CHECK(metrics.LineSpacing() == 18);
      CHECK(*metrics.XHeight() == Catch::Approx(8.453125f));
      CHECK(*metrics.CapHeight() == Catch::Approx(11.0078125f));
      CHECK(*metrics.ZeroWidth() == Catch::Approx(8.8984375f));
      CHECK(font->SpaceWidth() == Catch::Approx(4.4453125f));
      CHECK(font->AllowsAntialiasing());
      CHECK(font->GetPitch() == VariablePitch);
    }
  }

  TEST_CASE("Font: glyphs", "[Graphics][Font]")
  {
    auto font = CreateFont(Ahem(), 16);

    auto data = font->GlyphDataForCharacter(U'a');
    CHECK(data.Glyph == 67);
    CHECK(data.Font.get() == font.get());
    CHECK(data.IsValid());
    CHECK(font->GlyphForCharacter(U'X') == 58);
    CHECK(font->WidthForGlyph(58) == 16);

    // Spaces share the space glyph; ZERO WIDTH SPACE has its own, with no width.
    CHECK(font->SpaceGlyph() == 3);
    CHECK(font->GlyphForCharacter(U'\t') == 3);
    CHECK(font->GlyphForCharacter(U'\u00A0') == 3);
    CHECK(font->ZeroWidthSpaceGlyph() == 253);
    CHECK(font->IsZeroWidthSpaceGlyph(253));
    CHECK(font->WidthForGlyph(253) == 0);

    // Default ignorable and control characters the font lacks use ZERO WIDTH SPACE; those it has keep theirs.
    CHECK(font->GlyphForCharacter(U'\u0001') == 253);
    CHECK(font->GlyphForCharacter(U'\u200D') == 256);

    // Characters the font lacks have no glyph.
    CHECK(font->GlyphForCharacter(U'\u0628') == 0);
    CHECK_FALSE(font->GlyphDataForCharacter(U'\u0628').IsValid());
    CHECK_FALSE(font->GetGlyphPage(GlyphPage::PageNumberForCodePoint(U'\u0628')));
    REQUIRE(font->GetGlyphPage(GlyphPage::PageNumberForCodePoint(U'a')));
    CHECK(&font->GetGlyphPage(GlyphPage::PageNumberForCodePoint(U'a'))->GetFont() == font.get());

    // The ink of an Ahem glyph is its em square, from the ascent above the baseline.
    auto bounds = font->BoundsForGlyph(58);
    CHECK(bounds.X() == 0);
    CHECK(bounds.Y() == Catch::Approx(-12.8f));
    CHECK(bounds.Width() == 16);
    CHECK(bounds.Height() == 16);
    CHECK(font->BoundsForGlyph(253) == FloatRect());
    auto glyphs = Array<Glyph, 2> {58, 67};
    CHECK(font->BoundsForGlyphs(glyphs).size() == 2u);
  }

  TEST_CASE("Font: code point support", "[Graphics][Font]")
  {
    auto font = CreateFont(Ahem(), 16);
    CHECK(font->SupportsCodePoint(U'a'));
    CHECK_FALSE(font->SupportsCodePoint(U'\u0628'));
    // Support for format characters is the font's own, not the ZERO WIDTH SPACE replacement.
    CHECK(font->SupportsCodePoint(U'\u200D'));
    CHECK_FALSE(font->SupportsCodePoint(U'\u0001'));
    CHECK(font->SupportsCodePoint(U'\u00A0') == font->PlatformSupportsCodePoint(U'\u00A0'));
    CHECK(font->CanRenderCombiningCharacterSequence(u8"ab"));
    CHECK_FALSE(font->CanRenderCombiningCharacterSequence(u8"a\u0628"));
    // A variation sequence falls back to its characters.
    CHECK(font->CanRenderCombiningCharacterSequence(u8"a\uFE0F") == font->SupportsCodePoint(U'\uFE0F'));
  }

  TEST_CASE("Font: synthesis and derived fonts", "[Graphics][Font]")
  {
    SECTION("Synthetic bold widens glyphs by a pixel")
    {
      auto font = CreateFont(Ahem(), 16, true);
      CHECK(font->SyntheticBoldOffset() == 1);
      CHECK(font->WidthForGlyph(58) == 17);
      CHECK(font->WidthForGlyph(58, Font::SyntheticBoldInclusion::Exclude) == 16);
      CHECK(font->SpaceWidth() == 17);
      CHECK(font->SpaceWidth(Font::SyntheticBoldInclusion::Exclude) == 16);
    }

    SECTION("Synthetic oblique slants the ink")
    {
      auto font = CreateFont(Ahem(), 16, false, true);
      auto bounds = font->BoundsForGlyph(58);
      CHECK(bounds.Width() > 16);
      CHECK(font->WidthForGlyph(58) == 16);
    }

    SECTION("Derived fonts")
    {
      auto font = CreateFont(Ahem(), 20);
      FontDescription description;
      auto *smallCaps = font->SmallCapsFont(description);
      REQUIRE(smallCaps);
      CHECK(smallCaps->PlatformData().Size() == Catch::Approx(14.f));
      CHECK(smallCaps->WidthForGlyph(58) == Catch::Approx(14.f));
      CHECK(font->SmallCapsFont(description) == smallCaps);
      CHECK(font->EmphasisMarkFont(description)->PlatformData().Size() == 10);
      CHECK(font->VariantFont(description, SmallCapsVariant) == smallCaps);
      CHECK(&font->NoSynthesizableFeaturesFont() == font.get());
      CHECK(font->InvisibleFont().GetVisibility() == FontVisibility::Invisible);
      CHECK(font->InvisibleFont().IsInterstitial());
      CHECK(font->UprightOrientationFont().IsTextOrientationFallback());
      CHECK(font->VerticalRightOrientationFont().PlatformData().Orientation() == FontOrientation::Horizontal);
      CHECK(&font->BrokenIdeographFont() != font.get());
      CHECK_FALSE(font->HalfWidthFont());
      CHECK_FALSE(font->VariantCapsSupportedForSynthesis(FontVariantCaps::Small));
      CHECK(font->VariantCapsSupportedForSynthesis(FontVariantCaps::Normal));
    }

    SECTION("Platform data")
    {
      FontPlatformData data {ShareRef(Ahem()), 16, false, false};
      CHECK(data == FontPlatformData {ShareRef(Ahem()), 16, false, false});
      CHECK(data.Hash() == FontPlatformData {ShareRef(Ahem()), 16, false, false}.Hash());
      CHECK_FALSE(data == FontPlatformData::CloneWithSize(data, 12));
      CHECK(FontPlatformData::CloneWithSize(data, 12).Size() == 12);
      CHECK(FontPlatformData::CloneWithSyntheticOblique(data, true).SyntheticOblique());
      CHECK(FontPlatformData::CloneWithOrientation(data, FontOrientation::Vertical).Orientation()
            == FontOrientation::Vertical);
      CHECK(data.FamilyName() == u8"Ahem");
      CHECK(data.Description() == u8"Ahem 16px");
      CHECK_FALSE(FontPlatformData {}.Face());
      CHECK(FontPlatformData {HashTableDeletedValueTag {}}.IsHashTableDeletedValue());
      CHECK(Font::CreateSystemFallbackFontPlaceholder()->IsSystemFontFallbackPlaceholder());
      CHECK(Font::Create(data)->Description() == u8"Ahem 16px");
    }
  }
}
