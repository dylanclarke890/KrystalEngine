#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo::ax::tests
{
  namespace
  {
    /// @brief The objects' element ids, in order and separated by commas (a narrow string Catch2 can print).
    KRYS_NODISCARD std::string Ids(const AccessibilityObject::AccessibilityChildrenVector &objects) noexcept
    {
      std::string ids;
      for (const auto &object : objects)
      {
        if (!ids.empty())
        {
          ids += ',';
        }
        if (auto *element = object->GetElement())
        {
          auto id = element->Id();
          ids.append(id.begin(), id.end());
        }
      }
      return ids;
    }
  }

  TEST_CASE("AccessibilityObject: a list box's selected children", "[Accessibility][AccessibilityObject]")
  {
    AccessibilityFixture page(u8"<div id=single role=listbox>"
                              u8"<div id=a role=option aria-selected=true>A</div>"
                              u8"<div id=b role=option aria-selected=true>B</div></div>"
                              u8"<div id=multi role=listbox aria-multiselectable=true>"
                              u8"<div id=c role=option aria-selected=true>C</div>"
                              u8"<div id=d role=option>D</div>"
                              u8"<div id=e role=option aria-selected=true>E</div></div>");

    // A single-selection list box reports only its first selected option.
    CHECK(Ids(page.Object(u8"single").SelectedChildren()) == "a");
    CHECK(Ids(page.Object(u8"multi").SelectedChildren()) == "c,e");
    // Objects that can't have selected children have none.
    CHECK(page.Object(u8"c").SelectedChildren().empty());
  }

  TEST_CASE("AccessibilityObject: a tab list's selected tab", "[Accessibility][AccessibilityObject]")
  {
    AccessibilityFixture page(u8"<div id=tabs role=tablist>"
                              u8"<div id=t1 role=tab aria-controls=p1>One</div>"
                              u8"<div id=t2 role=tab aria-controls=p2>Two</div></div>"
                              u8"<div id=p1 role=tabpanel>First</div>"
                              u8"<div id=p2 role=tabpanel><button id=inside>Go</button></div>");

    auto &tabs = page.Object(u8"tabs");
    CHECK(tabs.SelectedTabItem() == nullptr);
    CHECK(tabs.SelectedChildren().empty());

    // A tab is selected while the focus is in a panel it controls.
    Downcast<html::HTMLElement>(page.ById(u8"inside")).Focus();
    CHECK(page.Object(u8"t2").IsSelected());
    CHECK_FALSE(page.Object(u8"t1").IsSelected());
    CHECK(tabs.SelectedTabItem() == &page.Object(u8"t2"));
    CHECK(Ids(tabs.SelectedChildren()) == "t2");
  }

  TEST_CASE("AccessibilityObject: a menu's selected item is its active or focused one",
            "[Accessibility][AccessibilityObject]")
  {
    AccessibilityFixture page(u8"<div id=menu role=menu tabindex=0 aria-activedescendant=m2>"
                              u8"<div id=m1 role=menuitem tabindex=-1>Open</div>"
                              u8"<div id=m2 role=menuitem tabindex=-1>Save</div></div>");

    auto &menu = page.Object(u8"menu");
    CHECK(page.Object(u8"m2").IsSelected());
    CHECK_FALSE(page.Object(u8"m1").IsSelected());
    CHECK(Ids(menu.SelectedChildren()) == "m2");

    // Without an active descendant, the focused item is the selected one.
    page.ById(u8"menu").RemoveAttribute(dom::DOMStringAtom(u8"aria-activedescendant"));
    Downcast<html::HTMLElement>(page.ById(u8"m1")).Focus();
    (void)page.TakeNotifications();
    CHECK(page.Object(u8"m1").IsSelected());
    CHECK(Ids(menu.SelectedChildren()) == "m1");
  }

  TEST_CASE("AccessibilityObject: a radio group's checked button", "[Accessibility][AccessibilityObject]")
  {
    AccessibilityFixture page(u8"<div id=group role=radiogroup>"
                              u8"<div id=r1 role=radio aria-checked=false>One</div>"
                              u8"<div id=r2 role=radio aria-checked=true>Two</div></div>");

    CHECK(page.Object(u8"group").SelectedRadioButton() == &page.Object(u8"r2"));
    CHECK(page.Object(u8"r1").SelectedRadioButton() == nullptr);
  }

  TEST_CASE("AccessibilityObject: a tree's rows and selected items", "[Accessibility][AccessibilityObject]")
  {
    AccessibilityFixture page(u8"<div id=tree role=tree aria-owns=i1>"
                              u8"<div id=i1 role=treeitem aria-selected=true>One"
                              u8"<div role=group><div id=i2 role=treeitem>Two</div></div></div>"
                              u8"<div id=i3 role=treeitem aria-selected=true>Three</div></div>");

    auto &tree = page.Object(u8"tree");
    REQUIRE(tree.Role() == AccessibilityRole::Tree);
    // Tree items in groups count, and those aria-owns lists come after the rest (aria-owns makes i1 the
    // tree's last child; its group is visited on the way, as on WebKit).
    CHECK(Ids(tree.AriaTreeRows()) == "i3,i2,i1");
    page.ById(u8"tree").RemoveAttribute(dom::DOMStringAtom(u8"aria-owns"));
    (void)page.TakeNotifications();
    // A tree is multi-selectable only when aria-multiselectable says so.
    CHECK(Ids(tree.SelectedChildren()) == "i1");
    REQUIRE_FALSE(page.ById(u8"tree").SetAttribute(u8"aria-multiselectable", u8"true").HasException());
    (void)page.TakeNotifications();
    CHECK(Ids(tree.SelectedChildren()) == "i1,i3");

    // A tree item's content leaves out its group of items.
    auto content = page.Object(u8"i1").AriaTreeItemContent();
    CHECK(std::ranges::none_of(content, [](const auto &child) { return child->IsGroup(); }));
  }

  TEST_CASE("AccessibilityObject: a grid's selected rows", "[Accessibility][AccessibilityObject]")
  {
    AccessibilityFixture page(
      u8"<div id=grid role=grid>"
      u8"<div id=row1 role=row aria-selected=true><div role=gridcell>A</div></div>"
      u8"<div id=row2 role=row><div role=gridcell>B</div></div>"
      u8"<div id=row3 role=row aria-selected=true><div role=gridcell>C</div></div></div>"
      u8"<div id=table role=table aria-multiselectable=true>"
      u8"<div role=row><div role=cell>X</div></div></div>");

    // Grids are multi-selectable unless aria-multiselectable is false; tables never are.
    auto &grid = page.Object(u8"grid");
    CHECK(grid.IsMultiSelectable());
    CHECK(Ids(grid.SelectedChildren()) == "row1,row3");
    CHECK_FALSE(page.Object(u8"table").IsMultiSelectable());

    REQUIRE_FALSE(page.ById(u8"grid").SetAttribute(u8"aria-multiselectable", u8"false").HasException());
    (void)page.TakeNotifications();
    CHECK_FALSE(grid.IsMultiSelectable());
    CHECK(Ids(grid.SelectedChildren()) == "row1");
  }
}
