#include "Krystal.Booey/CSS/CSSOM/CSSViewTransitionRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSViewTransitionRule", "[CSS][CSSOM][CSSViewTransitionRule]")
  {
    auto sheet = SheetFromText(u8"@view-transition { navigation: auto; types: slide forwards; } "
                               u8"@view-transition { navigation: none; } "
                               u8"@view-transition { }");

    SECTION("navigation and types")
    {
      auto &rule = RuleAt<CSSViewTransitionRule>(*sheet, 0);
      CHECK(rule.StyleRuleType() == RuleType::ViewTransition);
      CHECK(ToNarrow(CSSOMString(rule.Navigation().View())) == "auto");
      auto types = rule.Types();
      REQUIRE(types.size() == 2);
      CHECK(ToNarrow(CSSOMString(types[0].View())) == "slide");
      CHECK(ToNarrow(CSSOMString(types[1].View())) == "forwards");
      CHECK(RuleText(rule) == "@view-transition { navigation: auto; types: slide forwards; }");
    }

    SECTION("navigation only")
    {
      auto &rule = RuleAt<CSSViewTransitionRule>(*sheet, 1);
      CHECK(ToNarrow(CSSOMString(rule.Navigation().View())) == "none");
      CHECK(rule.Types().empty());
      CHECK(RuleText(rule) == "@view-transition { navigation: none; }");
    }

    SECTION("nothing")
    {
      auto &rule = RuleAt<CSSViewTransitionRule>(*sheet, 2);
      CHECK(rule.Navigation().View().empty());
      CHECK(RuleText(rule) == "@view-transition { }");
    }
  }
}
