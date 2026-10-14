#include "Krystal.Booey/CSS/CSSOM/CSSGroupingRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSConditionRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSMediaRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSNestedDeclarations.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleRule.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSGroupingRule: child rules", "[CSS][CSSOM][CSSGroupingRule]")
  {
    auto sheet = SheetFromText(u8"@media screen { a { color: red; } }");
    auto &media = RuleAt<CSSGroupingRule>(*sheet, 0);

    REQUIRE(Is<CSSConditionRule>(media));
    REQUIRE(media.Length() == 1);
    REQUIRE(media.Item(1) == nullptr);
    auto *first = media.Item(0);
    REQUIRE(first == media.Item(0));
    REQUIRE(first->ParentRule() == &media);

    auto &rules = media.CssRules();
    REQUIRE(&rules == &media.CssRules());
    REQUIRE(rules.Length() == 1);
    REQUIRE(rules.Item(0) == first);
    REQUIRE(rules.GetStyleSheet() == sheet.get());

    SECTION("insertRule")
    {
      auto result = media.InsertRule(u8"b { color: blue; }", 1);
      REQUIRE(result.HasValue());
      REQUIRE(result.Value() == 1);
      REQUIRE(rules.Length() == 2);
      REQUIRE(media.Item(0) == first);
      REQUIRE(RuleText(media) == "@media screen {\n  a { color: red; }\n  b { color: blue; }\n}");

      REQUIRE(media.InsertRule(u8"@media print { }", 0).HasValue());
      REQUIRE(RuleText(media)
              == "@media screen {\n  @media print {\n}\n  a { color: red; }\n  b { color: blue; }\n}");
    }

    SECTION("insertRule errors")
    {
      CHECK(ExceptionCodeOf(media.InsertRule(u8"b { }", 2)) == dom::ExceptionCode::IndexSizeError);
      CHECK(ExceptionCodeOf(media.InsertRule(u8"x", 0)) == dom::ExceptionCode::SyntaxError);
      // Declarations are only allowed inside a style or scope rule.
      CHECK(ExceptionCodeOf(media.InsertRule(u8"color: red;", 0)) == dom::ExceptionCode::SyntaxError);
      CHECK(ExceptionCodeOf(media.InsertRule(u8"@import \"a.css\";", 0))
            == dom::ExceptionCode::HierarchyRequestError);
      CHECK(ExceptionCodeOf(media.InsertRule(u8"@namespace a url(a);", 0))
            == dom::ExceptionCode::HierarchyRequestError);
      CHECK(media.Length() == 1);
    }

    SECTION("deleteRule")
    {
      RefPtr<CSSRule> held = ShareRefPtr(first);
      CHECK(ExceptionCodeOf(media.DeleteRule(1)) == dom::ExceptionCode::IndexSizeError);
      REQUIRE_FALSE(media.DeleteRule(0).HasException());
      CHECK(rules.Length() == 0);
      CHECK(held->ParentRule() == nullptr);
      CHECK(RuleText(media) == "@media screen {\n}");
    }

    SECTION("a style rule inside can hold nested rules")
    {
      auto &style = Downcast<CSSStyleRule>(*first);
      REQUIRE(style.InsertRule(u8"& b { color: blue; }", 0).HasValue());
      // The child rule is replaced in the group, keeping the wrapper.
      REQUIRE(media.Item(0) == first);
      REQUIRE(RuleText(media) == "@media screen {\n  a {\n  color: red;\n  & b { color: blue; }\n}\n}");
    }
  }

  TEST_CASE("CSSGroupingRule: inside a style rule", "[CSS][CSSOM][CSSGroupingRule]")
  {
    auto sheet = SheetFromText(u8"a { @media screen { color: red; } }");
    auto &style = RuleAt<CSSStyleRule>(*sheet, 0);
    auto &media = Downcast<CSSGroupingRule>(*style.Item(0));

    REQUIRE(media.HasStyleRuleAncestor());
    REQUIRE(media.Length() == 1);
    REQUIRE(Is<CSSNestedDeclarations>(*media.Item(0)));

    SECTION("declarations")
    {
      REQUIRE(media.InsertRule(u8"background: blue", 1).HasValue());
      REQUIRE(Is<CSSNestedDeclarations>(*media.Item(1)));
      REQUIRE(RuleText(*media.Item(1)) == "background: blue;");
    }

    SECTION("style rules")
    {
      REQUIRE(media.InsertRule(u8"& b { }", 0).HasValue());
      REQUIRE(RuleText(*media.Item(0)) == "& b { }");
    }

    SECTION("rules that can't be nested")
    {
      CHECK(media.InsertRule(u8"@font-face { }", 0).HasException());
      CHECK(media.Length() == 1);
    }

    CHECK(RuleText(style).starts_with("a {\n  @media screen {"));
  }

  TEST_CASE("CSSGroupingRule: cssText with a serialisation context", "[CSS][CSSOM][CSSGroupingRule]")
  {
    auto sheet = SheetFromText(u8"@media screen { a { } b { } }");
    auto &media = RuleAt<CSSMediaRule>(*sheet, 0);
    CHECK(ToNarrow(static_cast<CSSRule &>(media).CssText(DefaultSerialisationContext()))
          == "@media screen {\n  a { }\n  b { }\n}");
  }
}
