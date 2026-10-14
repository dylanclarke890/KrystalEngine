#include "Krystal.Booey/CSS/Descriptors/PropertySetCSSDescriptors.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSFontFaceRule.hpp"
#include "Krystal.Booey/CSS/Descriptors/CSSFontFaceDescriptors.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("PropertySetCSSDescriptors", "[CSS][CSSOM][PropertySetCSSDescriptors]")
  {
    auto sheet = SheetFromText(u8"@font-face { font-family: a; src: url(a.woff); font-weight: 100 200; }");
    auto &rule = RuleAt<CSSFontFaceRule>(*sheet, 0);
    auto &descriptors = rule.Style();
    CSSStyleDeclaration &style = descriptors;

    CHECK(style.GetStyleDeclarationType() == StyleDeclarationType::FontFace);
    CHECK(Is<CSSFontFaceDescriptors>(style));
    CHECK(static_cast<PropertySetCSSDescriptors &>(descriptors).GetRuleType() == RuleType::FontFace);
    CHECK(style.ParentRule() == &rule);
    CHECK(style.ParentStyleSheet() == sheet.get());
    CHECK(style.CssRules() == nullptr);
    CHECK(ToNarrow(style.CssText()) == "font-family: a; src: url(\"a.woff\"); font-weight: 100 200;");

    SECTION("length and item")
    {
      CHECK(style.Length() == 3);
      CHECK(ToNarrow(style.Item(0)) == "font-family");
      CHECK(ToNarrow(style.Item(2)) == "font-weight");
      CHECK(style.Item(3).empty());
    }

    SECTION("getters")
    {
      CHECK(ToNarrow(style.GetPropertyValue(u8"font-family")) == "a");
      CHECK(ToNarrow(style.GetPropertyValue(u8"font-weight")) == "100 200");
      CHECK(style.GetPropertyValue(u8"nonsense").empty());
      CHECK(style.GetPropertyValue(u8"--x").empty());
      CHECK(style.GetPropertyPriority(u8"font-family").empty());
      CHECK(style.GetPropertyShorthand(u8"font-family").empty());
      CHECK_FALSE(style.IsPropertyImplicit(u8"font-family"));
    }

    SECTION("setProperty")
    {
      REQUIRE_FALSE(style.SetProperty(u8"font-style", u8"italic", u8"").HasException());
      CHECK(ToNarrow(style.GetPropertyValue(u8"font-style")) == "italic");

      // A value that isn't valid for the descriptor, and an invalid priority, change nothing.
      REQUIRE_FALSE(style.SetProperty(u8"font-display", u8"nonsense", u8"").HasException());
      REQUIRE_FALSE(style.SetProperty(u8"font-display", u8"swap", u8"x").HasException());
      CHECK(style.GetPropertyValue(u8"font-display").empty());

      CHECK(
        RuleText(rule)
        == "@font-face { font-family: a; src: url(\"a.woff\"); font-weight: 100 200; font-style: italic; }");
    }

    SECTION("removeProperty")
    {
      auto removed = style.RemoveProperty(u8"src");
      REQUIRE(removed.HasValue());
      CHECK(ToNarrow(removed.Value()) == "url(\"a.woff\")");
      CHECK(RuleText(rule) == "@font-face { font-family: a; font-weight: 100 200; }");

      auto missing = style.RemoveProperty(u8"nonsense");
      REQUIRE(missing.HasValue());
      CHECK(missing.Value().empty());
    }

    SECTION("setCssText")
    {
      REQUIRE_FALSE(style.SetCssText(u8"font-family: b; font-display: block").HasException());
      CHECK(RuleText(rule) == "@font-face { font-family: b; font-display: block; }");
    }
  }

  TEST_CASE("CSSFontFaceDescriptors", "[CSS][CSSOM][CSSFontFaceDescriptors]")
  {
    auto sheet = SheetFromText(u8"@font-face { font-family: a; }");
    auto &style = RuleAt<CSSFontFaceRule>(*sheet, 0).Style();

    REQUIRE_FALSE(style.SetSrc(u8"local(b)").HasException());
    CHECK(ToNarrow(style.Src()) == "local(\"b\")");
    REQUIRE_FALSE(style.SetFontFamily(u8"c").HasException());
    CHECK(ToNarrow(style.FontFamily()) == "c");
    REQUIRE_FALSE(style.SetFontStyle(u8"oblique 10deg").HasException());
    CHECK(ToNarrow(style.FontStyle()) == "oblique 10deg");
    REQUIRE_FALSE(style.SetFontWeight(u8"bold").HasException());
    CHECK(ToNarrow(style.FontWeight()) == "bold");
    REQUIRE_FALSE(style.SetFontWidth(u8"condensed").HasException());
    CHECK(ToNarrow(style.FontWidth()) == "condensed");
    // `font-stretch` is an alias for `font-width`.
    CHECK(ToNarrow(style.FontStretch()) == "condensed");
    REQUIRE_FALSE(style.SetFontStretch(u8"expanded").HasException());
    CHECK(ToNarrow(style.FontWidth()) == "expanded");
    REQUIRE_FALSE(style.SetSizeAdjust(u8"50%").HasException());
    CHECK(ToNarrow(style.SizeAdjust()) == "50%");
    REQUIRE_FALSE(style.SetUnicodeRange(u8"U+0-7F").HasException());
    CHECK(ToNarrow(style.UnicodeRange()) == "U+0-7f");
    REQUIRE_FALSE(style.SetFontFeatureSettings(u8"\"liga\" 0").HasException());
    CHECK(ToNarrow(style.FontFeatureSettings()) == "\"liga\" 0");
    REQUIRE_FALSE(style.SetFontDisplay(u8"optional").HasException());
    CHECK(ToNarrow(style.FontDisplay()) == "optional");
  }
}
