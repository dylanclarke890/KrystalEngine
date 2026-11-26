#include "Krystal.Web/Css/Font/CSSFontSelector.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Text/Script.hpp"
#include "Krystal.Web.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Web.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Web/Css/Font/GenericFontFamilyNames.hpp"
#include "Krystal.Web/Graphics/Fonts/FontCache.hpp"
#include "Krystal.Web/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Web/Graphics/Fonts/FontDatabase.hpp"
#include "Krystal.Web/Page/Settings.hpp"

namespace krys::web::css::tests
{
  using layout::tests::RegisterAhem;
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD const RenderStyle &Rendered(ResolverFixture &fixture, DOMStringView id)
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
      settings->Values.FontGenericFamilies.SetSansSerifFontFamily(u8"Ahem", text::Script::Common);
      settings->Values.FontGenericFamilies.SetFixedFontFamily(u8"No Such Family", text::Script::Common);
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
      CHECK(families.SetSerifFontFamily(u8"Ahem", text::Script::Common));
      CHECK_FALSE(families.SetSerifFontFamily(u8"Ahem", text::Script::Common));
      CHECK(families.SetSerifFontFamily(u8"Liberation Sans", text::Script::Arabic));
      CHECK(families.SerifFontFamily(text::Script::Arabic) == u8"Liberation Sans");
      CHECK(families.SerifFontFamily(text::Script::Latin) == u8"Ahem");
      CHECK(
        *families.FontFamily(generic_font_family_names::FamilyNamesIndex::SerifFamily, text::Script::Greek)
        == u8"Ahem");
      CHECK_FALSE(families.FontFamily(generic_font_family_names::FamilyNamesIndex::SystemUiFamily));
      CHECK(families.SetSerifFontFamily(u8"", text::Script::Common));
      CHECK(families.SerifFontFamily(text::Script::Latin).empty());
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CSSFontSelector: system font keywords are the host's",
                   "[CSS][CSSFontSelector][css-fonts:font-prop]")
  {
    RegisterAhem();
    ResolverFixture fixture {u8R"(<!DOCTYPE html><p id=caption style="font: caption">x</p>
      <p id=menu style="font: menu">x</p><p id=status style="font: status-bar">x</p>)"};
    auto &document = *fixture.Document;

    SECTION("By default, the standard family at Chrome's sizes")
    {
      document.UpdateStyleIfNeeded();
      const auto &caption = Rendered(fixture, u8"caption");
      CHECK(PrimaryFamily(caption) == u8"Liberation Sans");
      CHECK(caption.ComputedFontSize() == 16.f);
      CHECK(Rendered(fixture, u8"menu").ComputedFontSize() == 12.f);
      CHECK(Rendered(fixture, u8"status").ComputedFontSize() == 12.f);
      CHECK(caption.GetFontDescription().Weight() == NormalWeightValue());
    }

    SECTION("The document's settings give them")
    {
      auto settings = Settings::Create();
      auto &menu = settings->Values.SystemFonts[SystemFontKeyword::Menu];
      menu.Family = u8"Ahem";
      menu.Size = 20;
      menu.Weight = BoldWeightValue();
      document.SetSettings(Ref<Settings>(settings));
      document.UpdateStyleIfNeeded();

      const auto &style = Rendered(fixture, u8"menu");
      CHECK(PrimaryFamily(style) == u8"Ahem");
      CHECK(style.ComputedFontSize() == 20.f);
      CHECK(style.GetFontDescription().Weight() == BoldWeightValue());
      CHECK(Rendered(fixture, u8"caption").ComputedFontSize() == 16.f);
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
      auto ranges = selector.FontRangesForFamily(description, u8"Ahem");
      REQUIRE_FALSE(ranges.IsNull());
      CHECK_FALSE(ranges.IsGenericFontFamily());
      CHECK(ranges.FontForFirstRange().PlatformData().FamilyName() == u8"Ahem");
      CHECK(selector.FontRangesForFamily(description, u8"No Such Family").IsNull());
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
      settings->Values.FontGenericFamilies.SetPictographFontFamily(u8"Ahem", text::Script::Common);
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
