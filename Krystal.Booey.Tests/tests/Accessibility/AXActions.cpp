#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AXTestUtils.hpp"
#include "Krystal.Booey.Tests/Dom/DOMTestUtils.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/Dom/Events/EventListener.hpp"
#include "Krystal.Booey/Dom/Nodes/Element.hpp"
#include "Krystal.Booey/Events/UI/KeyboardEvent.hpp"
#include "Krystal.Booey/Html/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLSelectElement.hpp"

namespace krys::boo::ax::tests
{
  using boo::tests::FunctionListener;

  namespace
  {
    void ListenWith(Element &element, DOMStringAtom type, Func<void(Event &)> &&function) noexcept
    {
      element.AddEventListener(krys::move(type),
                               Ref<EventListenerCallback>(CreateRef<FunctionListener>(krys::move(function))),
                               false);
    }
  }

  TEST_CASE("AXActions: pressing", "[Accessibility][AXActions]")
  {
    AXFixture page(u8"<button id=button>Go</button><input id=box type=checkbox>"
                   u8"<div id=group role=button><span>Inside</span><button id=inner>Real</button></div>"
                   u8"<p id=text>Text</p>");
    int clicks = 0;
    ListenWith(page.ById(u8"button"), u8"click", [&](Event &) { ++clicks; });
    CHECK(page.Object(u8"button").Press());
    CHECK(clicks == 1);

    CHECK(page.Object(u8"box").Press());
    CHECK(Downcast<HTMLInputElement>(page.ById(u8"box")).Checked());

    // The real control the author hid in an ARIA one is pressed.
    CHECK(page.Object(u8"group").ActionElement() == &page.ById(u8"inner"));

    CHECK_FALSE(page.Object(u8"text").Press());
  }

  TEST_CASE("AXActions: focusing", "[Accessibility][AXActions]")
  {
    AXFixture page(u8"<input id=input><p id=text>Text</p>");
    auto &input = page.Object(u8"input");
    CHECK(input.CanSetFocusAttribute());
    input.SetFocused(true);
    CHECK(page.Document->FocusedElement() == &page.ById(u8"input"));
    input.SetFocused(false);
    CHECK(page.Document->FocusedElement() != &page.ById(u8"input"));
  }

  TEST_CASE("AXActions: values", "[Accessibility][AXActions]")
  {
    AXFixture page(u8"<input id=input value=old><input id=readonly readonly>"
                   u8"<input id=range type=range min=0 max=100 value=50 step=10>"
                   u8"<div id=aria role=slider aria-valuenow=5 aria-valuemin=0 aria-valuemax=10 "
                   u8"tabindex=0>5</div>");
    auto &input = page.Object(u8"input");
    CHECK(input.CanSetValueAttribute());
    CHECK_FALSE(page.Object(u8"readonly").CanSetValueAttribute());
    CHECK(input.SetValue(DOMStringView(u8"new")));
    CHECK(Downcast<HTMLInputElement>(page.ById(u8"input")).Value() == u8"new");

    // A focused field is typed into.
    input.SetFocused(true);
    CHECK(input.SetValue(DOMStringView(u8"typed")));
    CHECK(Downcast<HTMLInputElement>(page.ById(u8"input")).Value() == u8"typed");

    auto &range = page.Object(u8"range");
    CHECK(range.SetValue(30.0f));
    CHECK(range.ValueForRange() == 30);
    range.Increment();
    CHECK(range.ValueForRange() == 40);
    range.Decrement();
    CHECK(range.ValueForRange() == 30);

    SECTION("An ARIA slider is stepped with the arrow keys")
    {
      utf8_string key;
      ListenWith(page.ById(u8"aria"), u8"keydown",
                 [&](Event &event) { key = Downcast<KeyboardEvent>(event).Key(); });
      page.Object(u8"aria").Increment();
      CHECK(key == u8"ArrowRight");
    }
  }

  TEST_CASE("AXActions: selecting and expanding", "[Accessibility][AXActions]")
  {
    AXFixture page(u8"<select id=list size=3><option>One</option><option id=two>Two</option></select>"
                   u8"<details id=details><summary>More</summary>Hidden</details>");
    auto &list = page.Object(u8"list");
    REQUIRE(list.Children().size() == 2);
    list.Children()[1]->SetSelected(true);
    CHECK(Downcast<HTMLSelectElement>(page.ById(u8"list")).SelectedIndex() == 1);

    auto &details = page.Object(u8"details");
    details.SetIsExpanded(true);
    CHECK(page.ById(u8"details").HasAttribute(u8"open"));
  }

  TEST_CASE("AXActions: hit testing", "[Accessibility][AXActions]")
  {
    AXFixture page(u8"<style>body { margin: 0 }</style>"
                   u8"<button id=button style='width: 100px; height: 40px'>Go</button>"
                   u8"<p id=far style='margin-top: 5000px'>Far</p>");
    auto *root = page.Cache().RootObject();
    REQUIRE(root);
    CHECK(root->AccessibilityHitTest({10, 10}) == &page.Object(u8"button"));
    CHECK_FALSE(page.Object(u8"button").IsOffScreen());
    CHECK(page.Object(u8"far").IsOffScreen());
  }
}
