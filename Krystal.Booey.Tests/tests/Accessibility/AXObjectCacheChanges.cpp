#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"

namespace krys::boo::ax::tests
{
  TEST_CASE("AXObjectCache changes: a node's children changing", "[Accessibility][AXObjectCache]")
  {
    AccessibilityFixture page(u8"<ul id=list><li>One</li></ul>");
    auto &list = page.Object(u8"list");
    CHECK(list.Children().size() == 1);
    (void)page.TakeNotifications();

    auto item = page.Document->CreateElement(u8"li").ReleaseValue();
    REQUIRE_FALSE(item->AppendChild(*page.Document->CreateTextNode(u8"Two")).HasException());
    REQUIRE_FALSE(page.ById(u8"list").AppendChild(*item).HasException());

    auto notifications = page.TakeNotifications();
    CHECK(AccessibilityFixture::WasPosted(notifications, list, AXNotification::ChildrenChanged));
    CHECK(list.Children().size() == 2);
  }

  TEST_CASE("AXObjectCache changes: the role attribute", "[Accessibility][AXObjectCache]")
  {
    AccessibilityFixture page(u8"<div id=div>Text</div><div id=widget role=button>Go</div>");

    SECTION("Gaining a role makes the object again")
    {
      auto oldID = page.Object(u8"div").ObjectID();
      REQUIRE_FALSE(page.ById(u8"div").SetAttribute(u8"role", u8"checkbox").HasException());
      (void)page.TakeNotifications();
      auto &div = page.Object(u8"div");
      CHECK(div.ObjectID() != oldID);
      CHECK(div.Role() == AccessibilityRole::Checkbox);
    }

    SECTION("Changing it updates the object")
    {
      auto &widget = page.Object(u8"widget");
      auto oldID = widget.ObjectID();
      REQUIRE_FALSE(page.ById(u8"widget").SetAttribute(u8"role", u8"link").HasException());
      (void)page.TakeNotifications();
      CHECK(page.Object(u8"widget").ObjectID() == oldID);
      CHECK(page.Object(u8"widget").Role() == AccessibilityRole::Link);
    }
  }

  TEST_CASE("AXObjectCache changes: ARIA state attributes post notifications",
            "[Accessibility][AXObjectCache]")
  {
    AccessibilityFixture page(u8"<div id=box role=checkbox aria-checked=false>Agree</div>"
                              u8"<button id=button aria-expanded=false>More</button>"
                              u8"<input id=input>");
    auto &box = page.Object(u8"box");
    auto &button = page.Object(u8"button");
    auto &input = page.Object(u8"input");
    (void)page.TakeNotifications();

    REQUIRE_FALSE(page.ById(u8"box").SetAttribute(u8"aria-checked", u8"true").HasException());
    REQUIRE_FALSE(page.ById(u8"button").SetAttribute(u8"aria-expanded", u8"true").HasException());
    REQUIRE_FALSE(page.ById(u8"input").SetAttribute(u8"disabled", u8"").HasException());
    auto notifications = page.TakeNotifications();
    CHECK(AccessibilityFixture::WasPosted(notifications, box, AXNotification::CheckedStateChanged));
    CHECK(AccessibilityFixture::WasPosted(notifications, button, AXNotification::ExpandedChanged));
    CHECK(AccessibilityFixture::WasPosted(notifications, input, AXNotification::DisabledStateChanged));
    CHECK(box.IsChecked());
    CHECK(button.IsExpanded());
    CHECK_FALSE(input.IsEnabled());

    // Setting an attribute to the value it has changes nothing.
    REQUIRE_FALSE(page.ById(u8"box").SetAttribute(u8"aria-checked", u8"true").HasException());
    CHECK_FALSE(
      AccessibilityFixture::WasPosted(page.TakeNotifications(), box, AXNotification::CheckedStateChanged));
  }

  TEST_CASE("AXObjectCache changes: a label changing", "[Accessibility][AXObjectCache]")
  {
    AccessibilityFixture page(u8"<button id=button aria-label=Old>Go</button>");
    auto &button = page.Object(u8"button");
    CHECK(button.AccessibleName() == u8"Old");
    (void)page.TakeNotifications();

    REQUIRE_FALSE(page.ById(u8"button").SetAttribute(u8"aria-label", u8"New").HasException());
    CHECK(AccessibilityFixture::WasPosted(page.TakeNotifications(), button, AXNotification::TextChanged));
    CHECK(button.AccessibleName() == u8"New");
  }

  TEST_CASE("AXObjectCache changes: text changing", "[Accessibility][AXObjectCache]")
  {
    AccessibilityFixture page(u8"<button id=button>Go</button>");
    auto &button = page.Object(u8"button");
    CHECK(button.AccessibleName() == u8"Go");
    auto *text = DynamicDowncast<dom::Text>(page.ById(u8"button").FirstChild());
    REQUIRE(text);
    auto *textObject = page.Cache().GetOrCreate(*text);
    REQUIRE(textObject);
    (void)page.TakeNotifications();

    REQUIRE_FALSE(text->AppendData(u8" home").HasException());
    // The host's next rendering update lays the page out.
    (void)page.Document->UpdateLayout();
    auto notifications = page.TakeNotifications();
    CHECK(AccessibilityFixture::WasPosted(notifications, *textObject, AXNotification::TextChanged));
    CHECK(AccessibilityFixture::WasPosted(notifications, button, AXNotification::TextUnderElementChanged));
    CHECK(button.AccessibleName() == u8"Go home");
  }

  TEST_CASE("AXObjectCache changes: visibility", "[Accessibility][AXObjectCache]")
  {
    AccessibilityFixture page(u8"<div id=outer><p id=inner>Text</p></div>");
    auto &outer = page.Object(u8"outer");
    CHECK(outer.Children().size() == 1);

    REQUIRE_FALSE(page.ById(u8"inner").SetAttribute(u8"style", u8"visibility: hidden").HasException());
    (void)page.TakeNotifications();
    CHECK(page.Object(u8"inner").IsIgnored());
    CHECK(outer.Children().empty());
  }

  TEST_CASE("AXObjectCache changes: aria-hidden", "[Accessibility][AXObjectCache]")
  {
    AccessibilityFixture page(u8"<div id=outer><p id=inner>Text</p></div>");
    auto &inner = page.Object(u8"inner");
    CHECK_FALSE(inner.IsIgnored());

    REQUIRE_FALSE(page.ById(u8"outer").SetAttribute(u8"aria-hidden", u8"true").HasException());
    (void)page.TakeNotifications();
    CHECK(inner.IsIgnored());

    page.ById(u8"outer").RemoveAttribute(dom::DOMStringAtom(u8"aria-hidden"));
    (void)page.TakeNotifications();
    CHECK_FALSE(inner.IsIgnored());
  }

  TEST_CASE("AXObjectCache changes: selecting an item", "[Accessibility][AXObjectCache]")
  {
    AccessibilityFixture page(u8"<div id=list role=listbox>"
                              u8"<div id=first role=option aria-selected=true>One</div>"
                              u8"<div id=second role=option>Two</div></div>");
    auto &list = page.Object(u8"list");
    auto &second = page.Object(u8"second");
    (void)page.TakeNotifications();

    REQUIRE_FALSE(page.ById(u8"second").SetAttribute(u8"aria-selected", u8"true").HasException());
    auto notifications = page.TakeNotifications();
    CHECK(AccessibilityFixture::WasPosted(notifications, list, AXNotification::SelectedChildrenChanged));
    CHECK(AccessibilityFixture::WasPosted(notifications, second, AXNotification::SelectedStateChanged));
  }

  TEST_CASE("AXObjectCache changes: focus moving", "[Accessibility][AXObjectCache]")
  {
    AccessibilityFixture page(u8"<button id=first>One</button><button id=second>Two</button>");
    auto &second = page.Object(u8"second");
    (void)page.TakeNotifications();

    Downcast<html::HTMLElement>(page.ById(u8"second")).Focus();
    CHECK(AccessibilityFixture::WasPosted(page.TakeNotifications(), second,
                                          AXNotification::FocusedUIElementChanged));
    CHECK(page.Cache().FocusedObject() == &second);
  }

  TEST_CASE("AXObjectCache changes: form controls", "[Accessibility][AXObjectCache]")
  {
    AccessibilityFixture page(
      u8"<input id=text><input id=box type=checkbox>"
      u8"<input id=a type=radio name=group><input id=b type=radio name=group>"
      u8"<select id=select><option id=one>One</option><option id=two>Two</option></select>"
      u8"<input id=required required>");
    auto &text = page.Object(u8"text");
    auto &box = page.Object(u8"box");
    auto &a = page.Object(u8"a");
    auto &two = page.Object(u8"two");
    auto &required = page.Object(u8"required");
    (void)page.TakeNotifications();

    SECTION("Values")
    {
      REQUIRE_FALSE(Downcast<html::HTMLInputElement>(page.ById(u8"text")).SetValue(u8"Hello").HasException());
      CHECK(AccessibilityFixture::WasPosted(page.TakeNotifications(), text, AXNotification::ValueChanged));
    }

    SECTION("Checked states")
    {
      Downcast<html::HTMLInputElement>(page.ById(u8"box")).SetChecked(true);
      CHECK(
        AccessibilityFixture::WasPosted(page.TakeNotifications(), box, AXNotification::CheckedStateChanged));
    }

    SECTION("Selected options")
    {
      Downcast<html::HTMLOptionElement>(page.ById(u8"two")).Selected(true);
      CHECK(
        AccessibilityFixture::WasPosted(page.TakeNotifications(), two, AXNotification::SelectedStateChanged));
    }

    SECTION("Validity")
    {
      REQUIRE_FALSE(
        Downcast<html::HTMLInputElement>(page.ById(u8"required")).SetValue(u8"Filled in").HasException());
      CHECK(AccessibilityFixture::WasPosted(page.TakeNotifications(), required,
                                            AXNotification::InvalidStatusChanged));
    }

    SECTION("Radio groups")
    {
      auto radio = page.Document->CreateElement(u8"input").ReleaseValue();
      REQUIRE_FALSE(radio->SetAttribute(u8"type", u8"radio").HasException());
      REQUIRE_FALSE(radio->SetAttribute(u8"name", u8"group").HasException());
      REQUIRE_FALSE(page.ById(u8"text").ParentNode()->AppendChild(*radio).HasException());
      CHECK(AccessibilityFixture::WasPosted(page.TakeNotifications(), a,
                                            AXNotification::RadioGroupMembershipChanged));
    }
  }
}
