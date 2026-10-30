#include "Krystal.Booey/CSS/CSSFontSelector.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/WebKitFontFamilyNames.hpp"
#include "Krystal.Booey/Graphics/FontCache.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/FontDatabase.hpp"
#include "Krystal.Booey/Page/Settings.hpp"

namespace krys::boo::css::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    void RegisterAhem()
    {
      (void)FontDatabase::Singleton().AddFontFile(std::filesystem::path {KRYS_BOOEY_TESTS_DATA_DIR} / "fonts"
                                                  / "Ahem.ttf");
    }

    KRYS_NODISCARD const RenderStyle &Rendered(ResolverFixture &fixture, dom::DOMStringView id)
    {
      auto *style = fixture.ById(id).GetRenderStyle();
      REQUIRE(style);
      return *style;
    }

    KRYS_NODISCARD utf8_string PrimaryFamily(const RenderStyle &style)
    {
      return style.GetFontCascade().PrimaryFont()->PlatformData().FamilyName();
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CSSFontSelector: generic families", "[CSS][CSSFontSelector]")
  {
    RegisterAhem();
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      #sans { font-family: sans-serif; font-size: 10px; width: 10ex }
      #named { font-family: Ahem, serif; font-size: 10px }
      #unknown { font-family: "No Such Family"; font-size: 10px; width: 10ex }
      #mono { font-family: monospace }
    </style><p id=sans>x</p><p id=named>x</p><p id=unknown>x</p><p id=mono>x</p>)"};
    auto &document = *fixture.Document;

    SECTION("Unset generic families use the last resort")
    {
      document.UpdateStyleIfNeeded();
      CHECK(PrimaryFamily(Rendered(fixture, u8"sans")) == u8"Liberation Sans");
      CHECK(PrimaryFamily(Rendered(fixture, u8"named")) == u8"Ahem");
      CHECK(PrimaryFamily(Rendered(fixture, u8"unknown")) == u8"Liberation Sans");
      // `ex` is the primary font's x-height: Liberation Sans' is 1082/2048em.
      auto &unknown = Rendered(fixture, u8"unknown");
      CHECK(ResolverFixture::Serialised(unknown, unknown.Width()) == "52.832031px");
    }

    SECTION("Generic families resolve with the document's settings")
    {
      auto settings = Settings::Create();
      settings->Values.FontGenericFamilies.SetSansSerifFontFamily(u8"Ahem", USCRIPT_COMMON);
      settings->Values.FontGenericFamilies.SetFixedFontFamily(u8"No Such Family", USCRIPT_COMMON);
      document.SetSettings(Ref<Settings>(settings));
      document.UpdateStyleIfNeeded();

      auto &sans = Rendered(fixture, u8"sans");
      CHECK(PrimaryFamily(sans) == u8"Ahem");
      // Ahem's x-height is 0.8em.
      CHECK(ResolverFixture::Serialised(sans, sans.Width()) == "80px");
      // A generic family whose family isn't registered falls through to the last resort.
      CHECK(PrimaryFamily(Rendered(fixture, u8"mono")) == u8"Liberation Sans");
    }

    SECTION("A script's family, else the common script's")
    {
      FontGenericFamilies families;
      CHECK(families.SerifFontFamily().empty());
      CHECK(families.SetSerifFontFamily(u8"Ahem", USCRIPT_COMMON));
      CHECK_FALSE(families.SetSerifFontFamily(u8"Ahem", USCRIPT_COMMON));
      CHECK(families.SetSerifFontFamily(u8"Liberation Sans", USCRIPT_ARABIC));
      CHECK(families.SerifFontFamily(USCRIPT_ARABIC) == u8"Liberation Sans");
      CHECK(families.SerifFontFamily(USCRIPT_LATIN) == u8"Ahem");
      CHECK(*families.FontFamily(WebKitFontFamilyNames::FamilyNamesIndex::SerifFamily, USCRIPT_GREEK)
            == u8"Ahem");
      CHECK_FALSE(families.FontFamily(WebKitFontFamilyNames::FamilyNamesIndex::SystemUiFamily));
      CHECK(families.SetSerifFontFamily(u8"", USCRIPT_COMMON));
      CHECK(families.SerifFontFamily(USCRIPT_LATIN).empty());
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CSSFontSelector: the document's selector", "[CSS][CSSFontSelector]")
  {
    RegisterAhem();
    ResolverFixture fixture {u8R"(<!DOCTYPE html><p id=p style="font-family: Ahem">x</p>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();

    auto &selector = document.GetFontSelector();
    CHECK(&selector == &document.GetFontSelector());
    CHECK(selector.IsCSSFontSelector());
    CHECK(selector.IsEmpty());
    CHECK(selector.IsSimpleFontSelectorForDescription());
    CHECK(selector.FallbackFontCount() == 0u);
    CHECK(Rendered(fixture, u8"p").GetFontCascade().GetFontSelector() == &selector);
    CHECK(Rendered(fixture, u8"p").GetFontCascade().IsCurrent(selector));

    SECTION("Ranges for a family")
    {
      FontDescription description;
      description.SetComputedSize(16);
      auto ranges = selector.FontRangesForFamily(description, StringAtom {u8"Ahem"});
      REQUIRE_FALSE(ranges.IsNull());
      CHECK_FALSE(ranges.IsGenericFontFamily());
      CHECK(ranges.FontForFirstRange().PlatformData().FamilyName() == u8"Ahem");
      CHECK(selector.FontRangesForFamily(description, StringAtom {u8"No Such Family"}).IsNull());
    }

    SECTION("Invalidating the font cache restyles the document")
    {
      auto version = selector.Version();
      FontCache::InvalidateAllFontCaches();
      CHECK(selector.Version() > version);
      CHECK_FALSE(Rendered(fixture, u8"p").GetFontCascade().IsCurrent(selector));
      CHECK(document.NeedsStyleRecalc());
      document.UpdateStyleIfNeeded();
      CHECK(Rendered(fixture, u8"p").GetFontCascade().IsCurrent(selector));
      CHECK(PrimaryFamily(Rendered(fixture, u8"p")) == u8"Ahem");
    }

    SECTION("Pictograph fallback")
    {
      auto settings = Settings::Create();
      settings->Values.FontFallbackPrefersPictographs = true;
      settings->Values.FontGenericFamilies.SetPictographFontFamily(u8"Ahem", USCRIPT_COMMON);
      document.SetSettings(Ref<Settings>(settings));
      CHECK(selector.FallbackFontCount() == 1u);
      FontDescription description;
      description.SetComputedSize(16);
      auto font = selector.FallbackFontAt(description, 0);
      REQUIRE(font);
      CHECK(font->PlatformData().FamilyName() == u8"Ahem");
    }
  }
}
