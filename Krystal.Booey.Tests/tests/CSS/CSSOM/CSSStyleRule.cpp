#include "Krystal.Booey/CSS/CSSOM/CSSStyleRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey.Tests/CSS/RuleTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSMediaRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSNestedDeclarations.hpp"
#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSStyleRule: cssText", "[CSS][CSSOM][CSSStyleRule]")
  {
    auto sheet = SheetFromText(u8"a { } "
                               u8"b, c { color: red; margin: 1px 2px; } "
                               u8"d { color: red; & e { color: blue; } } "
                               u8"f { & g { } } "
                               u8"h { @media screen { color: red; } }");

    CHECK(RuleText(*sheet->Item(0)) == "a { }");
    CHECK(RuleText(*sheet->Item(1)) == "b, c { color: red; margin: 1px 2px; }");
    CHECK(RuleText(*sheet->Item(2)) == "d {\n  color: red;\n  & e { color: blue; }\n}");
    CHECK(RuleText(*sheet->Item(3)) == "f {\n  & g { }\n}");
    CHECK(RuleText(*sheet->Item(4)) == "h {\n  @media screen {\n  color: red;\n}\n}");
    CHECK(ToNarrow(sheet->Item(2)->CssText(DefaultSerialisationContext()))
          == "d {\n  color: red;\n  & e { color: blue; }\n}");
  }

  TEST_CASE("CSSStyleRule: selectorText", "[CSS][CSSOM][CSSStyleRule]")
  {
    auto sheet = SheetFromText(u8"a > b, .c { color: red; } d { & e { } }");
    auto &rule = RuleAt<CSSStyleRule>(*sheet, 0);

    CHECK(TypeOf(rule) == RuleType::Style);
    CHECK(ToNarrow(rule.SelectorText()) == "a > b, .c");
    // The text is cached.
    CHECK(ToNarrow(rule.SelectorText()) == "a > b, .c");

    SECTION("set")
    {
      rule.SetSelectorText(u8"#x  ~  y");
      CHECK(ToNarrow(rule.SelectorText()) == "#x ~ y");
      CHECK(RuleText(rule) == "#x ~ y { color: red; }");
      CHECK(SelectorsText(rule.GetStyleRule().GetSelectorList()) == "#x ~ y");
    }

    SECTION("an invalid selector is ignored")
    {
      rule.SetSelectorText(u8"a >");
      CHECK(ToNarrow(rule.SelectorText()) == "a > b, .c");
      rule.SetSelectorText(u8"");
      CHECK(ToNarrow(rule.SelectorText()) == "a > b, .c");
    }

    SECTION("a nested rule's selector")
    {
      auto &outer = RuleAt<CSSStyleRule>(*sheet, 1);
      auto &nested = Downcast<CSSStyleRule>(*outer.Item(0));
      CHECK(ToNarrow(nested.SelectorText()) == "& e");
      nested.SetSelectorText(u8"& f");
      CHECK(ToNarrow(nested.SelectorText()) == "& f");
      CHECK(RuleText(outer) == "d {\n  & f { }\n}");
    }

    SECTION("a rule outside a sheet can't be changed")
    {
      auto standalone = rule.GetStyleRule().CreateCSSOMWrapper();
      auto &wrapper = Downcast<CSSStyleRule>(*standalone);
      wrapper.SetSelectorText(u8"z");
      CHECK(ToNarrow(wrapper.SelectorText()) == "a > b, .c");
    }
  }

  TEST_CASE("CSSStyleRule: style", "[CSS][CSSOM][CSSStyleRule]")
  {
    auto sheet = SheetFromText(u8"a { color: red; }");
    auto &rule = RuleAt<CSSStyleRule>(*sheet, 0);
    auto &style = rule.Style();

    CHECK(&style == &rule.Style());
    CHECK(style.ParentRule() == &rule);
    CHECK(style.ParentStyleSheet() == sheet.get());
    CHECK(ToNarrow(style.GetPropertyValue(u8"color")) == "red");

    REQUIRE_FALSE(style.SetProperty(u8"margin", u8"1px", u8"important").HasException());
    CHECK(RuleText(rule) == "a { color: red; margin: 1px !important; }");
  }

  TEST_CASE("CSSStyleRule: nested rules", "[CSS][CSSOM][CSSStyleRule]")
  {
    auto sheet = SheetFromText(u8"a { color: red; }");
    auto &rule = RuleAt<CSSStyleRule>(*sheet, 0);

    CHECK(rule.Length() == 0);
    CHECK(rule.Item(0) == nullptr);
    CHECK(rule.CssRules().Length() == 0);
    CHECK_FALSE(rule.GetStyleRule().IsStyleRuleWithNesting());

    SECTION("inserting the first nested rule turns the rule into one with nesting")
    {
      auto *styleRule = &rule.GetStyleRule();
      REQUIRE(rule.InsertRule(u8"& b { color: blue; }", 0).HasValue());
      CHECK(rule.GetStyleRule().IsStyleRuleWithNesting());
      CHECK(&rule.GetStyleRule() != styleRule);
      CHECK(sheet->Contents().RuleAt(0) == &rule.GetStyleRule());
      CHECK(rule.Length() == 1);
      CHECK(rule.CssRules().Length() == 1);
      CHECK(rule.Item(0)->ParentRule() == &rule);
      CHECK(RuleText(rule) == "a {\n  color: red;\n  & b { color: blue; }\n}");

      REQUIRE(rule.InsertRule(u8"@media screen { }", 1).HasValue());
      REQUIRE(rule.InsertRule(u8"margin: 0", 0).HasValue());
      CHECK(Is<CSSNestedDeclarations>(*rule.Item(0)));
      CHECK(RuleText(rule)
            == "a {\n  color: red;\n  margin: 0px;\n  & b { color: blue; }\n  @media screen {\n}\n}");

      RefPtr<CSSRule> deleted = ShareRefPtr(rule.Item(1));
      REQUIRE_FALSE(rule.DeleteRule(1).HasException());
      CHECK(deleted->ParentRule() == nullptr);
      CHECK(rule.Length() == 2);
      CHECK(ExceptionCodeOf(rule.DeleteRule(2)) == dom::ExceptionCode::IndexSizeError);
    }

    SECTION("insertRule errors")
    {
      CHECK(ExceptionCodeOf(rule.InsertRule(u8"& b { }", 1)) == dom::ExceptionCode::IndexSizeError);
      CHECK(ExceptionCodeOf(rule.InsertRule(u8"x", 0)) == dom::ExceptionCode::SyntaxError);
      CHECK(rule.InsertRule(u8"@font-face { }", 0).HasException());
      CHECK(rule.Length() == 0);
      CHECK_FALSE(rule.GetStyleRule().IsStyleRuleWithNesting());
    }

    SECTION("a rule outside a sheet")
    {
      auto parsed = Parser::ParseRule(u8"a { color: red; }", CSSOMTestContext(), nullptr,
                                      Parser::AllowedRules::RegularRules);
      REQUIRE(parsed);
      auto standalone = parsed->CreateCSSOMWrapper();
      auto &wrapper = Downcast<CSSStyleRule>(*standalone);
      REQUIRE(wrapper.InsertRule(u8"& b { }", 0).HasValue());
      CHECK(wrapper.GetStyleRule().IsStyleRuleWithNesting());
      CHECK(RuleText(wrapper) == "a {\n  color: red;\n  & b { }\n}");
    }
  }

  TEST_CASE("CSSStyleRule: a nested rule inside a nested rule", "[CSS][CSSOM][CSSStyleRule]")
  {
    auto sheet = SheetFromText(u8"a { & b { } }");
    auto &outer = RuleAt<CSSStyleRule>(*sheet, 0);
    auto &inner = Downcast<CSSStyleRule>(*outer.Item(0));

    REQUIRE(inner.InsertRule(u8"& c { }", 0).HasValue());
    CHECK(outer.Item(0) == &inner);
    CHECK(RuleText(outer) == "a {\n  & b {\n  & c { }\n}\n}");
    CHECK(inner.Item(0)->ParentStyleSheet() == sheet.get());
  }

  TEST_CASE("CSSNestedDeclarations", "[CSS][CSSOM][CSSNestedDeclarations]")
  {
    auto sheet = SheetFromText(u8"a { & b { } color: red; }");
    auto &outer = RuleAt<CSSStyleRule>(*sheet, 0);
    REQUIRE(outer.Length() == 2);
    REQUIRE(Is<CSSNestedDeclarations>(*outer.Item(1)));
    auto &declarations = Downcast<CSSNestedDeclarations>(*outer.Item(1));

    CHECK(TypeOf(declarations) == RuleType::NestedDeclarations);
    CHECK(RuleText(declarations) == "color: red;");

    auto &style = declarations.Style();
    CHECK(&style == &declarations.Style());
    CHECK(style.ParentRule() == &declarations);
    REQUIRE_FALSE(style.SetProperty(u8"color", u8"blue", u8"").HasException());
    CHECK(RuleText(declarations) == "color: blue;");
    CHECK(RuleText(outer) == "a {\n  & b { }\n  color: blue;\n}");
  }
}
