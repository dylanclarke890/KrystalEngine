#include "Krystal.Booey/Accessibility/AccessibilityListBoxOption.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"

namespace krys::boo::ax::tests
{
  TEST_CASE("AccessibilityListBoxOption: a list box's options", "[Accessibility][AccessibilityListBoxOption]")
  {
    AccessibilityFixture page(u8"<select id=list multiple size=4>"
                              u8"<option id=a>Apple</option>"
                              u8"<optgroup id=group label=Citrus><option id=b selected>Banana</option>"
                              u8"<option id=c disabled label=Cherry>C</option></optgroup>"
                              u8"</select>");

    auto &list = page.Object(u8"list");
    CHECK(list.Role() == AccessibilityRole::ListBox);
    CHECK(list.IsMultiSelectable());

    // The options and the group are all the list box's children, in order.
    const auto &children = list.Children();
    REQUIRE(children.size() == 4);
    CHECK(children[0].get() == &page.Object(u8"a"));
    CHECK(children[1].get() == &page.Object(u8"group"));
    CHECK(children[2].get() == &page.Object(u8"b"));
    CHECK(children[3].get() == &page.Object(u8"c"));

    auto &b = page.Object(u8"b");
    CHECK(b.IsListBoxOption());
    CHECK(b.Role() == AccessibilityRole::ListBoxOption);
    CHECK(b.ParentObject() == &list);
    CHECK(b.IsSelected());
    CHECK_FALSE(page.Object(u8"a").IsSelected());
    CHECK(b.CanSetSelectedAttribute());
    CHECK(b.StringValue() == u8"Banana");
    CHECK(b.Children().empty());

    CHECK_FALSE(page.Object(u8"group").IsEnabled());
    CHECK(page.Object(u8"group").StringValue() == u8"Citrus");
    CHECK_FALSE(page.Object(u8"c").IsEnabled());
    CHECK_FALSE(page.Object(u8"c").CanSetSelectedAttribute());
    CHECK(page.Object(u8"c").StringValue() == u8"Cherry");

    // Each option is a row of the list box.
    auto listRect = list.ElementRect();
    auto aRect = page.Object(u8"a").ElementRect();
    CHECK_FALSE(aRect.IsEmpty());
    CHECK(listRect.Contains(aRect));
    CHECK(page.Object(u8"b").ElementRect().Y() > aRect.Y());
  }
}
