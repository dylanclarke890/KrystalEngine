#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSGroupingRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSKeyframesRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSScopeRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSRule: the CSSOM type", "[CSS][CSSOM][CSSRule]")
  {
    auto sheet = SheetFromText(u8"@import \"a.css\"; "
                               u8"@namespace a url(a); "
                               u8"b { } "
                               u8"@media screen { } "
                               u8"@font-face { } "
                               u8"@page { } "
                               u8"@keyframes c { } "
                               u8"@counter-style d { system: cyclic; symbols: x; } "
                               u8"@supports (color: red) { } "
                               u8"@font-feature-values e { } "
                               u8"@layer f; "
                               u8"@container (min-width: 1px) { } "
                               u8"@property --g { syntax: \"*\"; inherits: false; } "
                               u8"@view-transition { } ");

    auto typeAt = [&](uint32 index)
    {
      REQUIRE(sheet->Item(index));
      return sheet->Item(index)->TypeForCSSOM();
    };

    REQUIRE(sheet->Length() == 14);
    CHECK(typeAt(0) == 3);  // IMPORT_RULE
    CHECK(typeAt(1) == 10); // NAMESPACE_RULE
    CHECK(typeAt(2) == 1);  // STYLE_RULE
    CHECK(typeAt(3) == 4);  // MEDIA_RULE
    CHECK(typeAt(4) == 5);  // FONT_FACE_RULE
    CHECK(typeAt(5) == 6);  // PAGE_RULE
    CHECK(typeAt(6) == 7);  // KEYFRAMES_RULE
    CHECK(typeAt(7) == 11); // COUNTER_STYLE_RULE
    CHECK(typeAt(8) == 12); // SUPPORTS_RULE
    CHECK(typeAt(9) == 14); // FONT_FEATURE_VALUES_RULE
    // Rules added after the enumeration was frozen are 0.
    CHECK(typeAt(10) == 0);
    CHECK(typeAt(11) == 0);
    CHECK(typeAt(12) == 0);
    CHECK(typeAt(13) == 0);
  }

  TEST_CASE("CSSRule: parents", "[CSS][CSSOM][CSSRule]")
  {
    auto sheet = SheetFromText(u8"@media screen { a { & b { } } } @scope (.c) { d { } }");

    auto &media = RuleAt<CSSGroupingRule>(*sheet, 0);
    auto &a = Downcast<CSSStyleRule>(*media.Item(0));
    auto &b = Downcast<CSSStyleRule>(*a.Item(0));
    auto &scope = RuleAt<CSSScopeRule>(*sheet, 1);
    auto &d = Downcast<CSSStyleRule>(*scope.Item(0));

    CHECK(media.ParentRule() == nullptr);
    CHECK(media.ParentStyleSheet() == sheet.get());
    CHECK(a.ParentRule() == &media);
    CHECK(a.ParentStyleSheet() == sheet.get());
    CHECK(b.ParentRule() == &a);
    CHECK(b.ParentStyleSheet() == sheet.get());

    CHECK_FALSE(media.HasStyleRuleAncestor());
    CHECK_FALSE(a.HasStyleRuleAncestor());
    CHECK(b.HasStyleRuleAncestor());
    CHECK_FALSE(d.HasStyleRuleAncestor());

    CHECK_FALSE(media.GetNestedContext());
    CHECK_FALSE(a.GetNestedContext());
    CHECK(b.GetNestedContext() == NestedParserContextType::Style);
    CHECK(d.GetNestedContext() == NestedParserContextType::Scope);

    CHECK(static_cast<CSSRule &>(media).IsGroupingRule());
    CHECK_FALSE(a.IsGroupingRule());

    // cssText can't be set.
    CHECK_FALSE(a.SetCssText(u8"x { }").HasException());
    CHECK(RuleText(a) == "a {\n  & b { }\n}");
  }

  TEST_CASE("CSSRule: a wrapper created without a parent", "[CSS][CSSOM][CSSRule]")
  {
    auto rule = Parser::ParseRule(u8"a { color: red; }", CSSOMTestContext(), nullptr,
                                  Parser::AllowedRules::RegularRules);
    REQUIRE(rule);

    auto wrapper = rule->CreateCSSOMWrapper();
    CHECK(wrapper->ParentRule() == nullptr);
    CHECK(wrapper->ParentStyleSheet() == nullptr);
    CHECK(RuleText(*wrapper) == "a { color: red; }");
  }
}
