#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/FocusEvent.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <string>

namespace krys::boo::html::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD HTMLElement &HTML(dom::Element &element)
    {
      return Downcast<HTMLElement>(element);
    }

    KRYS_NODISCARD std::string Id(const dom::EventTarget *target)
    {
      auto *element = dynamic_cast<const dom::Element *>(target);
      if (!element)
      {
        return "null";
      }
      auto id = element->GetAttribute(u8"id").value_or(dom::DOMString());
      return std::string(id.begin(), id.end());
    }

    /// @brief Records the focus events it handles as `type:target(relatedTarget)`.
    struct FocusRecorder : public dom::EventListenerCallback
    {
      List<std::string> &Records;

      explicit FocusRecorder(List<std::string> &records) noexcept : Records(records)
      {
      }

      void HandleEvent(dom::Event &event) noexcept override
      {
        auto &focus = static_cast<dom::FocusEvent &>(event);
        auto type = focus.Type().View();
        Records.push_back(std::string(type.begin(), type.end()) + ":" + Id(event.Target().get()) + "("
                          + Id(focus.RelatedTarget().get()) + ")");
      }
    };

    void Listen(dom::EventTarget &target, List<std::string> &records)
    {
      Ref<dom::EventListenerCallback> recorder = CreateRef<FocusRecorder>(records);
      for (auto type : {u8"focus", u8"blur", u8"focusin", u8"focusout"})
      {
        target.AddEventListener(dom::DOMStringAtom(type), Ref {recorder}, false);
      }
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "FocusAlgorithms: tabindex and focusable areas", "[HTML][Focus]")
  {
    ResolverFixture fixture {
      u8R"(<!DOCTYPE html><style>.none { display: none } .hidden { visibility: hidden }</style>
      <body>
        <a id=link href=x></a><a id=anchor></a><button id=button></button><button id=disabled disabled></button>
        <input id=input><input id=hiddenInput type=hidden><div id=div></div><div id=tabbable tabindex=-1></div>
        <div id=invalid tabindex=x></div><button id=none class=none></button><button id=invisible class=hidden></button>
        <div inert><button id=inert></button></div><div id=editable contenteditable><p id=editableChild></p></div>
        <details><summary id=summary></summary><summary id=second></summary></details><iframe id=iframe></iframe>
        <fieldset disabled><input id=fieldsetInput></fieldset><div id=host></div>
      </body>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    auto focusable = [&](dom::DOMStringView id)
    {
      return FocusAlgorithms::IsFocusableArea(fixture.ById(id));
    };

    SECTION("tabindex values")
    {
      CHECK(FocusAlgorithms::TabIndexValue(fixture.ById(u8"tabbable")) == -1);
      CHECK_FALSE(FocusAlgorithms::TabIndexValue(fixture.ById(u8"invalid")));
      CHECK_FALSE(FocusAlgorithms::TabIndexValue(fixture.ById(u8"div")));

      CHECK(HTML(fixture.ById(u8"link")).TabIndex() == 0);
      CHECK(HTML(fixture.ById(u8"anchor")).TabIndex() == 0);
      CHECK(HTML(fixture.ById(u8"div")).TabIndex() == -1);
      CHECK(HTML(fixture.ById(u8"tabbable")).TabIndex() == -1);
      CHECK(HTML(fixture.ById(u8"invalid")).TabIndex() == -1);
      CHECK(HTML(fixture.ById(u8"summary")).TabIndex() == 0);
      CHECK(HTML(fixture.ById(u8"second")).TabIndex() == -1);

      HTML(fixture.ById(u8"div")).TabIndex(3);
      CHECK(fixture.ById(u8"div").GetAttribute(u8"tabindex") == u8"3");
      CHECK(HTML(fixture.ById(u8"div")).TabIndex() == 3);
    }

    SECTION("Focusable areas")
    {
      CHECK(focusable(u8"link"));
      CHECK_FALSE(focusable(u8"anchor"));
      CHECK(focusable(u8"button"));
      CHECK_FALSE(focusable(u8"disabled"));
      CHECK(focusable(u8"input"));
      CHECK_FALSE(focusable(u8"hiddenInput"));
      CHECK_FALSE(focusable(u8"div"));
      CHECK(focusable(u8"tabbable"));
      CHECK_FALSE(focusable(u8"invalid"));
      CHECK_FALSE(focusable(u8"none"));
      CHECK_FALSE(focusable(u8"invisible"));
      CHECK_FALSE(focusable(u8"inert"));
      CHECK(focusable(u8"editable"));
      CHECK_FALSE(focusable(u8"editableChild"));
      CHECK(focusable(u8"summary"));
      CHECK_FALSE(focusable(u8"second"));
      CHECK(focusable(u8"iframe"));
      CHECK_FALSE(focusable(u8"fieldsetInput"));

      // A shadow host that delegates focus isn't focusable itself.
      auto &host = fixture.ById(u8"host");
      REQUIRE_FALSE(host.SetAttribute(u8"tabindex", u8"0").HasException());
      CHECK(focusable(u8"host"));
      REQUIRE_FALSE(host.AttachShadow({.DelegatesFocus = dom::DelegatesFocus(true)}).HasException());
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(focusable(u8"host"));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "FocusAlgorithms: focus() and blur()", "[HTML][Focus]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      :focus { color: rgb(0, 0, 255) }
      :focus-within { width: 5px }
    </style><body><div id=d><button id=a></button><button id=b></button><span id=s></span></div></body>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    auto &a = HTML(fixture.ById(u8"a"));
    auto &b = HTML(fixture.ById(u8"b"));
    auto &s = HTML(fixture.ById(u8"s"));
    List<std::string> records;
    Listen(a, records);
    Listen(b, records);

    REQUIRE_FALSE(document.FocusedElement());
    CHECK(document.ActiveElement() == document.Body());
    CHECK(document.HasFocus());

    SECTION("Focusing and moving the focus")
    {
      a.Focus();
      CHECK(document.FocusedElement() == &a);
      CHECK(document.ActiveElement() == &a);
      CHECK(a.Focused());
      CHECK(fixture.ById(u8"d").HasFocusWithin());
      CHECK(records == List<std::string> {"focus:a(null)", "focusin:a(null)"});
      document.UpdateStyleIfNeeded();
      CHECK(a.GetRenderStyle()->GetColor() == boo::Color(boo::Color::blue));

      records.clear();
      b.Focus();
      CHECK(document.FocusedElement() == &b);
      CHECK_FALSE(a.Focused());
      CHECK(records == List<std::string> {"blur:a(b)", "focusout:a(b)", "focus:b(a)", "focusin:b(a)"});

      // Focusing the focused element does nothing.
      records.clear();
      b.Focus();
      CHECK(records.empty());

      // Elements that aren't focusable don't take the focus.
      s.Focus();
      CHECK(document.FocusedElement() == &b);
    }

    SECTION("blur() focuses the viewport")
    {
      a.Focus();
      records.clear();
      a.Blur();
      CHECK_FALSE(document.FocusedElement());
      CHECK(document.ActiveElement() == document.Body());
      CHECK_FALSE(fixture.ById(u8"d").HasFocusWithin());
      CHECK(records == List<std::string> {"blur:a(null)", "focusout:a(null)"});

      // Blurring an element that isn't focused does nothing.
      records.clear();
      b.Blur();
      CHECK(records.empty());
    }

    SECTION("Removing the focused element focuses the viewport without events")
    {
      a.Focus();
      records.clear();
      Ref<dom::Element> protectedA = ShareRef<dom::Element>(a);
      REQUIRE_FALSE(fixture.ById(u8"d").RemoveChild(a).HasException());
      CHECK_FALSE(document.FocusedElement());
      CHECK_FALSE(a.Focused());
      CHECK_FALSE(fixture.ById(u8"d").HasFocusWithin());
      CHECK(records.empty());
    }

    SECTION("System focus")
    {
      a.Focus();
      document.SetFocusedAndActive(false);
      CHECK_FALSE(document.HasFocus());
      // The focused area stays, but `:focus` doesn't match.
      CHECK(document.ActiveElement() == &a);
      document.UpdateStyleIfNeeded();
      CHECK(a.GetRenderStyle()->GetColor() != boo::Color(boo::Color::blue));
      document.SetFocusedAndActive(true);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "FocusAlgorithms: focus visibility and clicks", "[HTML][Focus]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><body><button id=a><span id=inner></span></button>
      <button id=b></button><input id=i><p id=p></p></body>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    auto &a = HTML(fixture.ById(u8"a"));
    auto &b = HTML(fixture.ById(u8"b"));

    // Script focus shows the focus, unless the user last focused by clicking.
    a.Focus();
    CHECK(a.HasFocusVisible());

    // Clicking focuses the nearest focusable ancestor, without showing the focus.
    a.Blur();
    FocusAlgorithms::FocusForClick(fixture.ById(u8"inner"));
    CHECK_FALSE(a.HasFocusVisible());
    REQUIRE(document.FocusedElement() == &a);
    b.Focus();
    CHECK_FALSE(b.HasFocusVisible());
    FocusAlgorithms::FocusForClick(b);
    CHECK_FALSE(b.HasFocusVisible());

    // Text fields always show the focus.
    HTML(fixture.ById(u8"i")).Focus();
    CHECK(fixture.ById(u8"i").HasFocusVisible());

    // The options can force it either way.
    a.Focus({.FocusVisible = true});
    CHECK(a.HasFocusVisible());
    HTML(fixture.ById(u8"i")).Focus({.FocusVisible = false});
    CHECK_FALSE(fixture.ById(u8"i").HasFocusVisible());

    // Clicking something that isn't focusable focuses the viewport.
    FocusAlgorithms::FocusForClick(fixture.ById(u8"p"));
    CHECK_FALSE(document.FocusedElement());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "FocusAlgorithms: shadow trees", "[HTML][Focus]")
  {
    ResolverFixture fixture {
      u8R"(<!DOCTYPE html><style>:focus { color: rgb(0, 0, 255) }</style><body><div id=host></div></body>)"};
    auto &document = *fixture.Document;
    auto &host = HTML(fixture.ById(u8"host"));
    auto shadow = host.AttachShadow({.DelegatesFocus = dom::DelegatesFocus(true)}).ReleaseValue();
    auto first = document.CreateElement(u8"button").ReleaseValue();
    auto second = document.CreateElement(u8"button").ReleaseValue();
    REQUIRE_FALSE(shadow->AppendChild(*first).HasException());
    REQUIRE_FALSE(shadow->AppendChild(*second).HasException());
    document.UpdateStyleIfNeeded();

    SECTION("A host that delegates focus focuses its first focusable descendant")
    {
      host.Focus();
      CHECK(document.FocusedElement() == first.get());
      // The document sees the host; the shadow root sees the button.
      CHECK(document.ActiveElement() == &host);
      CHECK(shadow->ActiveElement() == first.get());
      document.UpdateStyleIfNeeded();
      CHECK(host.GetRenderStyle()->GetColor() == boo::Color(boo::Color::blue));

      // Focusing the host again keeps the focus where it is.
      HTML(*second).Focus();
      host.Focus();
      CHECK(document.FocusedElement() == second.get());
    }

    SECTION("Autofocus takes priority")
    {
      REQUIRE_FALSE(second->SetAttribute(u8"autofocus", u8"").HasException());
      host.Focus();
      CHECK(document.FocusedElement() == second.get());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "FocusAlgorithms: sequential focus navigation", "[HTML][Focus]")
  {
    ResolverFixture fixture {
      u8R"(<!DOCTYPE html><body><button id=a></button><div id=b tabindex=2></div><a id=c href=x></a>)"
      u8R"(<div id=d tabindex=-1></div><div id=e tabindex=1></div><div id=host><button id=slotted></button></div>)"
      u8R"(<button id=f></button><div id=negative tabindex=-1></div><button id=g disabled></button>)"
      u8R"(<p id=wrap><span id=first></span><button id=h></button></p></body>)"};
    auto &document = *fixture.Document;

    // The host's shadow tree: a button, a slot for the host's button and a button with a positive tabindex,
    // which comes first in the host's scope.
    auto shadow = HTML(fixture.ById(u8"host")).AttachShadow({}).ReleaseValue();
    auto append = [&](dom::ContainerNode &parent, dom::DOMStringView tagName, dom::DOMStringView id,
                      dom::DOMStringView tabIndex = {})
    {
      auto element = document.CreateElement(tagName).ReleaseValue();
      REQUIRE_FALSE(element->SetAttribute(u8"id", dom::DOMString(id)).HasException());
      if (!tabIndex.empty())
      {
        REQUIRE_FALSE(element->SetAttribute(u8"tabindex", dom::DOMString(tabIndex)).HasException());
      }
      REQUIRE_FALSE(parent.AppendChild(*element).HasException());
      return element;
    };
    auto s1 = append(*shadow, u8"button", u8"s1");
    (void)append(*shadow, u8"slot", u8"slot");
    auto s2 = append(*shadow, u8"button", u8"s2", u8"1");
    // A shadow host with a negative tabindex is left out with its scope.
    auto negativeShadow = HTML(fixture.ById(u8"negative")).AttachShadow({}).ReleaseValue();
    (void)append(*negativeShadow, u8"button", u8"skipped");
    document.UpdateStyleIfNeeded();

    auto ids = [](const List<dom::Element *> &elements)
    {
      List<std::string> result;
      for (auto *element : elements)
      {
        result.push_back(Id(element));
      }
      return result;
    };
    auto focused = [&]
    {
      return Id(document.FocusedElement());
    };
    auto next = [&]
    {
      CHECK(FocusAlgorithms::SequentialNavigation(document, dom::FocusDirection::Forward));
      return focused();
    };
    auto previous = [&]
    {
      CHECK(FocusAlgorithms::SequentialNavigation(document, dom::FocusDirection::Backward));
      return focused();
    };

    SECTION("the order")
    {
      // Positive tabindex values first, then tree order; the host's scope as a whole, its positive tabindex
      // first, with the slot's scope in place of the slot.
      CHECK(ids(FocusAlgorithms::SequentialFocusNavigationOrder(document))
            == List<std::string> {"e", "b", "a", "c", "s2", "s1", "slotted", "f", "h"});
      CHECK(FocusAlgorithms::IsFocusNavigationScopeOwner(document));
      CHECK(FocusAlgorithms::IsFocusNavigationScopeOwner(fixture.ById(u8"host")));
      CHECK_FALSE(FocusAlgorithms::IsFocusNavigationScopeOwner(fixture.ById(u8"a")));
      CHECK(FocusAlgorithms::AssociatedFocusNavigationOwner(*s1) == &fixture.ById(u8"host"));
      CHECK(FocusAlgorithms::AssociatedFocusNavigationOwner(fixture.ById(u8"a")) == &document);
      CHECK(Id(FocusAlgorithms::AssociatedFocusNavigationOwner(fixture.ById(u8"slotted"))) == "slot");
      CHECK(FocusAlgorithms::IsSequentiallyFocusable(fixture.ById(u8"c")));
      CHECK_FALSE(FocusAlgorithms::IsSequentiallyFocusable(fixture.ById(u8"d")));
      CHECK_FALSE(FocusAlgorithms::IsSequentiallyFocusable(fixture.ById(u8"g")));
    }

    SECTION("forward and backward, wrapping around")
    {
      // From the viewport: the first area, or the last.
      CHECK(next() == "e");
      CHECK(next() == "b");
      CHECK(fixture.ById(u8"b").HasFocusVisible());
      CHECK(previous() == "e");
      CHECK(previous() == "h");
      CHECK(next() == "e");
      for (auto expected : {"b", "a", "c", "s2", "s1", "slotted", "f", "h", "e"})
      {
        CHECK(next() == expected);
      }
    }

    SECTION("from an element that isn't sequentially focusable, in tree order")
    {
      HTML(fixture.ById(u8"d")).Focus();
      REQUIRE(focused() == "d");
      CHECK(next() == "e");
      HTML(fixture.ById(u8"d")).Focus();
      CHECK(previous() == "c");
    }

    SECTION("from the starting point inside the focused area")
    {
      // With the viewport focused, the starting point (where the user clicked) is inside it.
      document.SetSequentialFocusNavigationStartingPoint(&fixture.ById(u8"d"));
      CHECK(next() == "e");
      // It follows the focused element.
      CHECK(document.SequentialFocusNavigationStartingPoint() == &fixture.ById(u8"e"));
      HTML(fixture.ById(u8"e")).Blur();
      CHECK(document.SequentialFocusNavigationStartingPoint() == nullptr);
      HTML(fixture.ById(u8"e")).Focus();

      // A starting point outside the focused area is ignored.
      document.SetSequentialFocusNavigationStartingPoint(&fixture.ById(u8"wrap"));
      CHECK(next() == "b");

      // The document element unsets it.
      document.SetSequentialFocusNavigationStartingPoint(&fixture.ById(u8"d"));
      document.SetSequentialFocusNavigationStartingPoint(dom::TreeQueries::DocumentElement(document));
      CHECK(document.SequentialFocusNavigationStartingPoint() == nullptr);
    }

    SECTION("a removed starting point")
    {
      // After its previous sibling.
      document.SetSequentialFocusNavigationStartingPoint(&fixture.ById(u8"d"));
      REQUIRE_FALSE(fixture.ById(u8"d").Remove().HasException());
      CHECK(document.SequentialFocusNavigationStartingPoint() == &fixture.ById(u8"c"));
      CHECK(next() == "e");
      document.SetSequentialFocusNavigationStartingPoint(&fixture.ById(u8"e"));
      REQUIRE_FALSE(fixture.ById(u8"e").Remove().HasException());
      CHECK(previous() == "c");
      HTML(fixture.ById(u8"c")).Blur();

      // At the start of its parent.
      document.SetSequentialFocusNavigationStartingPoint(&fixture.ById(u8"first"));
      REQUIRE_FALSE(fixture.ById(u8"first").Remove().HasException());
      CHECK(document.SequentialFocusNavigationStartingPoint() == &fixture.ById(u8"wrap"));
      CHECK(next() == "h");
    }
  }
}
