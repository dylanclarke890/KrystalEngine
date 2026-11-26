#include "Krystal.Web/Accessibility/AXMenuList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Accessibility/AXTestUtils.hpp"
#include "Krystal.Web/Accessibility/AXMenuListPopup.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"

namespace krys::web::ax::tests
{
  TEST_CASE("AXMenuList: a drop-down select", "[Accessibility][AXMenuList]")
  {
    AXFixture page(u8"<select id=select><option id=a>Apple</option>"
                   u8"<option id=b selected>Banana</option><option id=c disabled>Cherry</option></select>");

    auto &select = page.Object(u8"select");
    CHECK(select.IsMenuList());
    CHECK(select.Role() == AXRole::PopUpButton);
    CHECK(select.IsCollapsed());
    CHECK(select.CanSetFocusAttribute());
    CHECK(select.StringValue() == u8"Banana");

    // The menu list's one child is its pop-up, whose children are the options.
    REQUIRE(select.Children().size() == 1);
    auto &popup = *select.Children()[0];
    CHECK(popup.IsMenuListPopup());
    CHECK(popup.Role() == AXRole::MenuListPopup);
    CHECK(popup.ParentObject() == &select);
    CHECK(popup.IsEnabled());

    const auto &options = popup.Children();
    REQUIRE(options.size() == 3);
    CHECK(options[0].get() == &page.Object(u8"a"));
    CHECK(options[1]->IsMenuListOption());
    CHECK(options[1]->Role() == AXRole::MenuListOption);
    CHECK(options[1]->ParentObject() == &popup);
    CHECK(options[1]->IsSelected());
    CHECK(options[1]->StringValue() == u8"Banana");
    CHECK_FALSE(options[2]->IsEnabled());
    CHECK(options[0]->ElementRect() == select.ElementRect());
  }

  TEST_CASE("AXMenuList: options added later join the pop-up", "[Accessibility][AXMenuList]")
  {
    AXFixture page(u8"<select id=select><option>Apple</option></select>");
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

  TEST_CASE("AXMenuList: removing the select removes the pop-up", "[Accessibility][AXMenuList]")
  {
    AXFixture page(u8"<select id=select><option>Apple</option></select>");
    auto popup = ShareRef(*page.Object(u8"select").Children()[0]);
    REQUIRE_FALSE(page.ById(u8"select").Remove().HasException());
    CHECK(popup->IsDetached());
    CHECK_FALSE(page.Cache().ObjectForID(popup->ObjectID()));
  }
}
