#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
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
}
