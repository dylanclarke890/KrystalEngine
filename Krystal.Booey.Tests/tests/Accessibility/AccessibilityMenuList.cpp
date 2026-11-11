#include "Krystal.Booey/Accessibility/AccessibilityMenuList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityMenuListPopup.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"

namespace krys::boo::ax::tests
{
  TEST_CASE("AccessibilityMenuList: a drop-down select", "[Accessibility][AccessibilityMenuList]")
  {
    AccessibilityFixture page(
      u8"<select id=select><option id=a>Apple</option>"
      u8"<option id=b selected>Banana</option><option id=c disabled>Cherry</option></select>");

    auto &select = page.Object(u8"select");
    CHECK(select.IsMenuList());
    CHECK(select.Role() == AccessibilityRole::PopUpButton);
    CHECK(select.IsCollapsed());
    CHECK(select.CanSetFocusAttribute());
    CHECK(select.StringValue() == u8"Banana");

    // The menu list's one child is its pop-up, whose children are the options.
    REQUIRE(select.Children().size() == 1);
    auto &popup = *select.Children()[0];
    CHECK(popup.IsMenuListPopup());
    CHECK(popup.Role() == AccessibilityRole::MenuListPopup);
    CHECK(popup.ParentObject() == &select);
    CHECK(popup.IsEnabled());

    const auto &options = popup.Children();
    REQUIRE(options.size() == 3);
    CHECK(options[0].get() == &page.Object(u8"a"));
    CHECK(options[1]->IsMenuListOption());
    CHECK(options[1]->Role() == AccessibilityRole::MenuListOption);
    CHECK(options[1]->ParentObject() == &popup);
    CHECK(options[1]->IsSelected());
    CHECK(options[1]->StringValue() == u8"Banana");
    CHECK_FALSE(options[2]->IsEnabled());
    CHECK(options[0]->ElementRect() == select.ElementRect());
  }

  TEST_CASE("AccessibilityMenuList: options added later join the pop-up",
            "[Accessibility][AccessibilityMenuList]")
  {
    AccessibilityFixture page(u8"<select id=select><option>Apple</option></select>");
    auto &select = page.Object(u8"select");
    auto &popup = *select.Children()[0];
    CHECK(popup.Children().size() == 1);

    auto option = page.Document->CreateElement(u8"option").ReleaseValue();
    REQUIRE_FALSE(page.ById(u8"select").AppendChild(*option).HasException());
    (void)page.Document->UpdateLayout();
    page.Cache().ChildrenChanged(&select);
    page.Cache().PerformDeferredCacheUpdate();
    CHECK(popup.Children().size() == 2);
  }

  TEST_CASE("AccessibilityMenuList: removing the select removes the pop-up",
            "[Accessibility][AccessibilityMenuList]")
  {
    AccessibilityFixture page(u8"<select id=select><option>Apple</option></select>");
    auto popup = ShareRef(*page.Object(u8"select").Children()[0]);
    REQUIRE_FALSE(page.ById(u8"select").Remove().HasException());
    CHECK(popup->IsDetached());
    CHECK_FALSE(page.Cache().ObjectForID(popup->ObjectID()));
  }
}
