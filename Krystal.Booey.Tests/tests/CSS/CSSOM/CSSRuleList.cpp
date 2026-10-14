#include "Krystal.Booey/CSS/CSSOM/CSSRuleList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSRuleList: a static list", "[CSS][CSSOM][CSSRuleList]")
  {
    auto sheet = SheetFromText(u8"a { } b { }");
    auto list = StaticCSSRuleList::Create();
    CSSRuleList &base = *list;

    CHECK(base.Length() == 0);
    CHECK(base.Item(0) == nullptr);
    CHECK(base.GetStyleSheet() == nullptr);

    list->Rules().push_back(ShareRefPtr(sheet->Item(1)));
    list->Rules().push_back(ShareRefPtr(sheet->Item(0)));
    CHECK(base.Length() == 2);
    CHECK(base.Item(0) == sheet->Item(1));
    CHECK(base.Item(1) == sheet->Item(0));
    CHECK(base.IsSupportedPropertyIndex(1));
    CHECK_FALSE(base.IsSupportedPropertyIndex(2));

    // The list keeps its rules even when the sheet deletes them.
    REQUIRE_FALSE(sheet->DeleteRule(0).HasException());
    CHECK(base.Length() == 2);
    CHECK(RuleText(*base.Item(1)) == "a { }");

    RefPtr<CSSRuleList> shared = ShareRefPtr(&base);
    CHECK(shared->GetRefCount() == 2);
  }
}
