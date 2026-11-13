#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include <algorithm>

namespace krys::boo::ax::tests
{
  namespace
  {
    KRYS_NODISCARD bool Contains(const AccessibilityObject::AccessibilityChildrenVector &objects,
                                 const AccessibilityObject &object) noexcept
    {
      return std::ranges::any_of(objects, [&](const auto &candidate) { return candidate.get() == &object; });
    }
  }

  TEST_CASE("AXRelations: labels", "[Accessibility][AXRelations]")
  {
    AccessibilityFixture page(u8"<span id=name>Name</span><button id=button aria-labelledby=name>Go</button>"
                              u8"<label id=label for=input>Field</label><input id=input>");
    auto &name = page.Object(u8"name");
    auto &button = page.Object(u8"button");
    CHECK(Contains(button.LabeledByObjects(), name));
    CHECK(Contains(name.LabelForObjects(), button));

    auto &label = page.Object(u8"label");
    auto &input = page.Object(u8"input");
    CHECK(Contains(label.LabelForObjects(), input));
    CHECK(Contains(input.LabeledByObjects(), label));
  }

  TEST_CASE("AXRelations: changing the attribute changes the relation", "[Accessibility][AXRelations]")
  {
    AccessibilityFixture page(u8"<button id=button aria-controls=first>Go</button>"
                              u8"<div id=first>One</div><div id=second>Two</div>");
    auto &button = page.Object(u8"button");
    auto &first = page.Object(u8"first");
    auto &second = page.Object(u8"second");
    CHECK(Contains(button.ControlledObjects(), first));
    CHECK(Contains(first.Controllers(), button));

    REQUIRE_FALSE(page.ById(u8"button").SetAttribute(u8"aria-controls", u8"second").HasException());
    (void)page.TakeNotifications();
    CHECK_FALSE(Contains(button.ControlledObjects(), first));
    CHECK(Contains(button.ControlledObjects(), second));
    CHECK(first.Controllers().empty());
    CHECK(Contains(second.Controllers(), button));
  }

  TEST_CASE("AXRelations: an ID appearing later", "[Accessibility][AXRelations]")
  {
    AccessibilityFixture page(
      u8"<button id=button aria-describedby=help>Go</button><p id=paragraph>Help</p>");
    auto &button = page.Object(u8"button");
    CHECK(button.DescribedByObjects().empty());

    REQUIRE_FALSE(page.ById(u8"paragraph").SetAttribute(u8"id", u8"help").HasException());
    (void)page.TakeNotifications();
    auto &paragraph = page.Object(u8"help");
    CHECK(Contains(button.DescribedByObjects(), paragraph));
  }

  TEST_CASE("AXRelations: aria-owns", "[Accessibility][AXRelations]")
  {
    AccessibilityFixture page(u8"<div id=owner role=list aria-owns=item><div role=listitem>One</div></div>"
                              u8"<div id=other role=group><div id=item role=listitem>Two</div></div>");
    auto &owner = page.Object(u8"owner");
    auto &other = page.Object(u8"other");
    auto &item = page.Object(u8"item");

    CHECK(item.ParentObject() == &owner);
    const auto &children = owner.Children();
    REQUIRE(children.size() == 2);
    CHECK(children[1].get() == &item);
    CHECK_FALSE(Contains(other.Children(), item));
    CHECK(Contains(item.Owners(), owner));

    SECTION("Removing aria-owns gives the item back")
    {
      page.ById(u8"owner").RemoveAttribute(dom::DOMStringAtom(u8"aria-owns"));
      (void)page.TakeNotifications();
      CHECK(item.ParentObject() == &other);
      CHECK(Contains(other.Children(), item));
      CHECK(owner.Children().size() == 1);
    }
  }

  TEST_CASE("AXRelations: aria-owns can't make a cycle", "[Accessibility][AXRelations]")
  {
    AccessibilityFixture page(
      u8"<div id=outer role=group><div id=inner role=group aria-owns=outer>X</div></div>");
    auto &outer = page.Object(u8"outer");
    auto &inner = page.Object(u8"inner");
    CHECK(inner.ParentObject() == &outer);
    CHECK(inner.OwnedObjects().empty());
  }

  TEST_CASE("AXRelations: an aria-owned child names its owner", "[Accessibility][AXRelations]")
  {
    AccessibilityFixture page(u8"<div id=button role=button aria-owns=more>Go</div><div id=more>home</div>");
    CHECK(page.Object(u8"button").AccessibleName() == u8"Go home");
  }

  TEST_CASE("AXRelations: the active descendant", "[Accessibility][AXRelations]")
  {
    AccessibilityFixture page(
      u8"<div id=list role=listbox tabindex=0 aria-activedescendant=first>"
      u8"<div id=first role=option>One</div><div id=second role=option>Two</div></div>");
    auto &list = page.Object(u8"list");
    auto &first = page.Object(u8"first");
    auto &second = page.Object(u8"second");
    CHECK(list.ActiveDescendant() == &first);

    Downcast<html::HTMLElement>(page.ById(u8"list")).Focus();
    (void)page.TakeNotifications();
    CHECK(page.Cache().FocusedObject() == &first);

    REQUIRE_FALSE(page.ById(u8"list").SetAttribute(u8"aria-activedescendant", u8"second").HasException());
    auto notifications = page.TakeNotifications();
    CHECK(list.ActiveDescendant() == &second);
    CHECK(page.Cache().FocusedObject() == &second);
    CHECK(AccessibilityFixture::WasPosted(notifications, second, AXNotification::FocusedUIElementChanged));
    CHECK(AccessibilityFixture::WasPosted(notifications, list, AXNotification::ActiveDescendantChanged));
  }

  TEST_CASE("AXRelations: a cell's headers", "[Accessibility][AXRelations]")
  {
    AccessibilityFixture page(u8"<table><tr><th id=a>A</th><th id=b>B</th></tr>"
                              u8"<tr><td id=cell headers=b>1</td><td>2</td></tr></table>");
    auto &cell = page.Object(u8"cell");
    auto headers = cell.ColumnHeaders();
    REQUIRE(headers.size() == 1);
    CHECK(headers[0].get() == &page.Object(u8"b"));
  }

  TEST_CASE("AXRelations: a popover's invoker", "[Accessibility][AXRelations]")
  {
    AccessibilityFixture page(u8"<button id=button popovertarget=popover>Open</button>"
                              u8"<div id=popover popover>Content</div>");
    auto &button = page.Object(u8"button");
    auto &popover = page.Object(u8"popover");
    CHECK(Contains(popover.Controllers(), button));
    (void)page.TakeNotifications();

    REQUIRE_FALSE(Downcast<html::HTMLElement>(page.ById(u8"popover")).ShowPopover().HasException());
    CHECK(AccessibilityFixture::WasPosted(page.TakeNotifications(), button, AXNotification::ExpandedChanged));
  }

  TEST_CASE("AXRelations: references to a shadow host go to its reference target",
            "[Accessibility][AXRelations][html:resolve-the-reference-target]")
  {
    AccessibilityFixture page(u8"<button id=button aria-controls=host aria-labelledby=host>Go</button>"
                              u8"<x-host id=host>Outside</x-host>");
    auto shadow =
      page.ById(u8"host").AttachShadow({.ReferenceTarget = dom::DOMString(u8"inner")}).ReleaseValue();
    REQUIRE_FALSE(shadow->SetHTMLUnsafe(u8"<div id=inner>Inner</div><div id=other>Other</div><slot></slot>")
                    .HasException());
    (void)page.TakeNotifications();
    page.Cache().RelationsNeedUpdate();
    auto &button = page.Object(u8"button");
    auto &inner = *page.Cache().GetOrCreate(*shadow->GetElementById(u8"inner"));
    auto &other = *page.Cache().GetOrCreate(*shadow->GetElementById(u8"other"));

    CHECK(Contains(button.ControlledObjects(), inner));
    CHECK(Contains(inner.Controllers(), button));
    // The name comes from what the reference names, not the host.
    CHECK(button.AccessibleName() == u8"Inner");

    // Changing the reference target moves the relations.
    shadow->SetReferenceTarget(dom::DOMString(u8"other"));
    CHECK_FALSE(Contains(button.ControlledObjects(), inner));
    CHECK(Contains(button.ControlledObjects(), other));

    // A reference target naming nothing leaves no relation.
    shadow->SetReferenceTarget(dom::DOMString(u8"missing"));
    CHECK(button.ControlledObjects().empty());
  }
}
