#include "Krystal.Booey/CSS/CSSOM/CSSKeyframesRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSKeyframeRule.hpp"
#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSKeyframesRule", "[CSS][CSSOM][CSSKeyframesRule]")
  {
    auto sheet =
      SheetFromText(u8"@keyframes spin { from { opacity: 0; } 50%, 75% { opacity: 0.5; } to { } }");
    auto &rule = RuleAt<CSSKeyframesRule>(*sheet, 0);

    CHECK(rule.StyleRuleType() == RuleType::Keyframes);
    CHECK(ToNarrow(CSSOMString(rule.Name().View())) == "spin");
    CHECK(rule.Length() == 3);
    CHECK(rule.IsSupportedPropertyIndex(2));
    CHECK_FALSE(rule.IsSupportedPropertyIndex(3));
    CHECK(rule.Item(3) == nullptr);
    CHECK(RuleText(rule)
          == "@keyframes spin { \n  0% { opacity: 0; }\n  50%,75% { opacity: 0.5; }\n  100% { }\n}");

    auto *keyframe = rule.Item(1);
    REQUIRE(keyframe);
    CHECK(keyframe == rule.Item(1));
    CHECK(keyframe->ParentRule() == &rule);
    CHECK(keyframe->ParentStyleSheet() == sheet.get());
    CHECK(TypeOf(*keyframe) == RuleType::Keyframe);
    CHECK(ToNarrow(keyframe->KeyText()) == "50%,75%");

    auto &rules = rule.CssRules();
    CHECK(&rules == &rule.CssRules());
    CHECK(rules.Length() == 3);
    CHECK(rules.Item(1) == keyframe);
    CHECK(rules.GetStyleSheet() == sheet.get());

    SECTION("findRule")
    {
      CHECK(rule.FindRule(u8"50%, 75%") == keyframe);
      CHECK(rule.FindRule(u8"0%") == rule.Item(0));
      CHECK(rule.FindRule(u8"to") == rule.Item(2));
      CHECK(rule.FindRule(u8"50%") == nullptr);
      CHECK(rule.FindRule(u8"x") == nullptr);
    }

    SECTION("appendRule")
    {
      rule.AppendRule(u8"25% { opacity: 0.25; }");
      CHECK(rule.Length() == 4);
      CHECK(ToNarrow(rule.Item(3)->KeyText()) == "25%");

      rule.AppendRule(u8"nonsense");
      CHECK(rule.Length() == 4);
    }

    SECTION("deleteRule")
    {
      RefPtr<CSSKeyframeRule> held = ShareRefPtr(keyframe);
      rule.DeleteRule(u8"50%,75%");
      CHECK(rule.Length() == 2);
      CHECK(held->ParentRule() == nullptr);

      rule.DeleteRule(u8"30%");
      CHECK(rule.Length() == 2);
    }

    SECTION("setName")
    {
      rule.SetName(u8"turn");
      CHECK(RuleText(rule).starts_with("@keyframes turn { \n"));
    }

    SECTION("a keyframe's key text and style")
    {
      keyframe->SetKeyText(u8"10%");
      CHECK(ToNarrow(keyframe->KeyText()) == "10%");
      // An invalid key text is ignored.
      keyframe->SetKeyText(u8"nonsense");
      CHECK(ToNarrow(keyframe->KeyText()) == "10%");

      auto &style = keyframe->Style();
      CHECK(&style == &keyframe->Style());
      CHECK(style.ParentRule() == keyframe);
      CHECK(ToNarrow(style.GetPropertyValue(u8"opacity")) == "0.5");
      REQUIRE_FALSE(style.SetProperty(u8"opacity", u8"1", u8"").HasException());
      CHECK(RuleText(*keyframe) == "10% { opacity: 1; }");
    }
  }
}
