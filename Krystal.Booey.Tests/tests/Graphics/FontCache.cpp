#include "Krystal.Booey/Graphics/FontCache.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/FontDatabase.hpp"

namespace krys::boo::tests
{
  namespace
  {
    void RegisterAhem()
    {
      (void)FontDatabase::Singleton().AddFontFile(std::filesystem::path {KRYS_BOOEY_TESTS_DATA_DIR} / "fonts"
                                                  / "Ahem.ttf");
    }

    KRYS_NODISCARD FontCascadeDescription Description(List<StringAtom> &&families, float size = 16)
    {
      FontCascadeDescription description;
      description.SetFamilies(krys::move(families));
      description.SetComputedSize(size);
      return description;
    }

    const StringAtom Ahem {u8"Ahem"};
  }

  TEST_CASE("FontCache: fonts for families", "[Graphics][FontCache]")
  {
    RegisterAhem();
    auto &cache = FontCache::Singleton();
    auto description = Description({Ahem});

    SECTION("A registered family")
    {
      auto font = cache.FontForFamily(description, Ahem);
      REQUIRE(font);
      CHECK(font->PlatformData().FamilyName() == u8"Ahem");
      CHECK(font->PlatformData().Size() == 16);
      CHECK_FALSE(font->PlatformData().SyntheticBold());
      CHECK_FALSE(font->PlatformData().SyntheticOblique());
      // Families match case-insensitively, and the same font is returned for the same description.
      CHECK(cache.FontForFamily(description, StringAtom {u8"ahem"}) == font);
      CHECK(cache.FontForPlatformData(font->PlatformData()).get() == font.get());
    }

    SECTION("An unknown family")
    {
      CHECK_FALSE(cache.FontForFamily(description, StringAtom {u8"No Such Family"}));
      // Aliases only find what's registered.
      CHECK_FALSE(cache.FontForFamily(description, StringAtom {u8"Arial"}));
      // Generic families aren't families: the font selector resolves them.
      CHECK_FALSE(cache.FontForFamily(description, StringAtom {u8"sans-serif"}));
    }

    SECTION("Synthesis")
    {
      auto bold = description;
      bold.SetWeight(BoldWeightValue());
      bold.SetIsItalic(true);
      auto font = cache.FontForFamily(bold, Ahem);
      REQUIRE(font);
      CHECK(font->PlatformData().SyntheticBold());
      CHECK(font->PlatformData().SyntheticOblique());

      // The options aren't part of the cache key, so this asks for a size that wasn't looked up before.
      EnumSet<FontLookupOptions> disallow {FontLookupOptions::DisallowBoldSynthesis,
                                           FontLookupOptions::DisallowObliqueSynthesis};
      auto exactDescription = bold;
      exactDescription.SetComputedSize(17);
      auto exact = cache.FontForFamily(exactDescription, Ahem, disallow);
      REQUIRE(exact);
      CHECK_FALSE(exact->PlatformData().SyntheticBold());
      CHECK_FALSE(exact->PlatformData().SyntheticOblique());

      bold.SetFontSynthesisWeight(FontSynthesisLonghandValue::None);
      bold.SetFontSynthesisStyle(FontSynthesisLonghandValue::None);
      auto unsynthesized = cache.FontForFamily(bold, Ahem);
      REQUIRE(unsynthesized);
      CHECK_FALSE(unsynthesized->PlatformData().SyntheticBold());
      CHECK_FALSE(unsynthesized->PlatformData().SyntheticOblique());
    }

    SECTION("The last resort is the bundled font")
    {
      auto font = cache.LastResortFallbackFont(description);
      CHECK(font->PlatformData().Face() == &FontDatabase::Singleton().BundledFace());
      CHECK(font->PlatformData().Size() == 16);
    }

    SECTION("System fallback searches the registered faces")
    {
      auto &original = *cache.FontForFamily(description, Ahem);
      // Liberation Sans has no CJK; Ahem has U+4E00.
      auto cjk = cache.SystemFallbackForCharacterCluster(Description({}), original, IsForPlatformFont::No,
                                                         FontCache::PreferColoredFont::No, u8"\u4E00");
      REQUIRE(cjk);
      CHECK(cjk->PlatformData().FamilyName() == u8"Ahem");
      // Only the bundled font has Cyrillic.
      auto cyrillic = cache.SystemFallbackForCharacterCluster(description, original, IsForPlatformFont::No,
                                                              FontCache::PreferColoredFont::No, u8"\u0416");
      REQUIRE(cyrillic);
      CHECK(cyrillic->PlatformData().Face() == &FontDatabase::Singleton().BundledFace());
      // No font has Thai.
      CHECK_FALSE(cache.SystemFallbackForCharacterCluster(description, original, IsForPlatformFont::No,
                                                          FontCache::PreferColoredFont::No, u8"\u0E01"));
      // Variation selectors don't need to be in the font.
      CHECK(cache.SystemFallbackForCharacterCluster(description, original, IsForPlatformFont::No,
                                                    FontCache::PreferColoredFont::No, u8"\u4E00\uFE0E"));
    }

    SECTION("Families")
    {
      auto families = cache.SystemFontFamilies();
      CHECK(std::ranges::find(families, utf8_string {u8"Ahem"}) != families.end());
      CHECK(cache.UseBackslashAsYenSignForFamily(StringAtom {u8"MS Gothic"}));
      CHECK_FALSE(cache.UseBackslashAsYenSignForFamily(Ahem));
    }
  }

  TEST_CASE("FontCascade: fonts", "[Graphics][FontCache]")
  {
    RegisterAhem();

    SECTION("The primary font is the first family that has a font")
    {
      FontCascade cascade {Description({StringAtom {u8"No Such Family"}, Ahem})};
      cascade.Update();
      CHECK(cascade.PrimaryFont()->PlatformData().FamilyName() == u8"Ahem");
      CHECK(cascade.MetricsOfPrimaryFont().Ascent() == Catch::Approx(12.8f));
      CHECK(cascade.ZeroWidth() == 16);
      CHECK_FALSE(cascade.IsPlatformFont());
      CHECK_FALSE(cascade.IsLoadingCustomFonts());

      auto glyph = cascade.GlyphDataForCharacter(U'a', false);
      CHECK(glyph.Glyph == 67);
      CHECK(glyph.Font.get() == cascade.PrimaryFont().get());
    }

    SECTION("Characters the families lack fall back")
    {
      FontCascade cascade {Description({Ahem})};
      cascade.Update();
      auto cyrillic = cascade.GlyphDataForCharacter(U'\u0416', false);
      REQUIRE(cyrillic.IsValid());
      CHECK(cyrillic.Font->PlatformData().Face() == &FontDatabase::Singleton().BundledFace());
      CHECK_FALSE(cascade.GlyphDataForCharacter(U'\u0E01', false).IsValid());
      // Private use characters don't fall back: they get the primary font's missing glyph.
      auto privateUse = cascade.GlyphDataForCharacter(U'\uE000', false);
      CHECK(privateUse.Glyph == 0);
      CHECK(privateUse.Font.get() == cascade.PrimaryFont().get());
      CHECK(cascade.FontForCombiningCharacterSequence(u8"\u0416\u0301")->PlatformData().Face()
            == &FontDatabase::Singleton().BundledFace());
    }

    SECTION("Without a family that has a font, the primary font is the last resort")
    {
      FontCascade cascade {Description({StringAtom {u8"No Such Family"}})};
      CHECK(cascade.PrimaryFont()->PlatformData().Face() == &FontDatabase::Singleton().BundledFace());
    }

    SECTION("Small caps use a smaller font for lowercase letters")
    {
      auto description = Description({Ahem}, 20);
      description.SetVariantCaps(FontVariantCaps::Small);
      FontCascade cascade {krys::move(description)};
      cascade.Update();
      auto lower = cascade.GlyphDataForCharacter(U'a', false);
      CHECK(lower.Glyph == cascade.PrimaryFont()->GlyphForCharacter(U'A'));
      CHECK(lower.Font->PlatformData().Size() == Catch::Approx(14.f));
      CHECK(cascade.GlyphDataForCharacter(U'A', false).Font.get() == cascade.PrimaryFont().get());
    }

    SECTION("Cascades with equal descriptions share their fonts")
    {
      FontCascade a {Description({Ahem})};
      FontCascade b {Description({StringAtom {u8"AHEM"}})};
      a.Update();
      b.Update();
      CHECK(a.Fonts() == b.Fonts());
      FontCascade c {Description({Ahem}, 12)};
      c.Update();
      CHECK(a.Fonts() != c.Fonts());
    }

    SECTION("A cascade for a platform font")
    {
      auto font = FontCache::Singleton().FontForFamily(Description({Ahem}), Ahem);
      FontCascade cascade {font->PlatformData()};
      CHECK(cascade.IsPlatformFont());
      CHECK(cascade.PrimaryFont().get() == font.get());
      CHECK(cascade.GetFontDescription().ComputedSize() == 16);
    }

    SECTION("Invalidating the font cache makes cascades stale")
    {
      FontCascade cascade {Description({Ahem})};
      cascade.Update();
      auto generation = FontCache::Singleton().Generation();
      FontCache::InvalidateAllFontCaches();
      CHECK(FontCache::Singleton().Generation() != generation);
      CHECK(cascade.Fonts()->Generation() != FontCache::Singleton().Generation());
      cascade.Update();
      CHECK(cascade.Fonts()->Generation() == FontCache::Singleton().Generation());
    }
  }
}
