#include "Krystal.Booey/DOM/Events/UIEvent.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/DOM/Events/CompositionEvent.hpp"
#include "Krystal.Booey/DOM/Events/FocusEvent.hpp"
#include "Krystal.Booey/DOM/Events/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Events/MouseEvent.hpp"
#include "Krystal.Booey/DOM/Events/TextEvent.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include <map>
#include <string>

namespace krys::boo::tests
{
  namespace
  {
    /// @brief Records whether each type of event had the window as its view.
    struct ViewRecorder : dom::EventListenerCallback
    {
      std::map<std::string, bool> &Records;
      LocalDOMWindow *Window;

      ViewRecorder(std::map<std::string, bool> &records, LocalDOMWindow *window) noexcept
          : Records(records), Window(window)
      {
      }

      void HandleEvent(dom::Event &event) noexcept override
      {
        auto *uiEvent = DynamicDowncast<dom::UIEvent>(event);
        auto type = std::string(event.Type().View().begin(), event.Type().View().end());
        Records[type] = uiEvent && uiEvent->View() == Window;
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "UIEvent: view", "[DOM][Events][uievents:dom-uievent-view]")
  {
    EditorFixture fixture(u8"<style>body{height:3000px}</style><input id=field><button id=button>b</button>"
                          u8"<div id=editable contenteditable>text</div>");
    auto *window = fixture.Document->Window();
    REQUIRE(window);
    dom::DOMStringAtom type(u8"x");

    SECTION("it's what the dictionary says, and null without one")
    {
      CHECK_FALSE(CreateRef<dom::UIEvent>(type)->View());
      dom::UIEventInit init;
      init.View = window;
      CHECK(CreateRef<dom::UIEvent>(type, init)->View() == window);

      dom::FocusEventInit focusInit;
      focusInit.View = window;
      CHECK(CreateRef<dom::FocusEvent>(type, focusInit)->View() == window);
    }

    SECTION("the legacy initializers set it")
    {
      auto event = CreateRef<dom::UIEvent>(type);
      event->InitUIEvent(type, true, true, window, 3);
      CHECK(event->View() == window);
      CHECK(event->Detail() == 3);
      event->InitUIEvent(type, false, false, nullptr, 4);
      CHECK_FALSE(event->View());
      CHECK(event->Detail() == 4);

      auto keyboard = CreateRef<dom::KeyboardEvent>(type);
      keyboard->InitKeyboardEvent(type, false, false, window, u8"a", 0, false, false, false, false);
      CHECK(keyboard->View() == window);

      auto composition = CreateRef<dom::CompositionEvent>(type);
      composition->InitCompositionEvent(type, false, false, window, u8"a");
      CHECK(composition->View() == window);
    }

    SECTION("a mouse event's page position needs it, for the scroll offset")
    {
      window->ScrollTo(0, 50);
      dom::MouseEventInit init;
      init.ClientX = 5;
      init.ClientY = 6;
      auto withoutView = CreateRef<dom::MouseEvent>(type, init);
      CHECK_FALSE(withoutView->View());
      CHECK(withoutView->PageY() == 6);

      init.View = window;
      auto withView = CreateRef<dom::MouseEvent>(type, init);
      CHECK(withView->View() == window);
      CHECK(withView->PageY() == 56);

      auto initialized = CreateRef<dom::MouseEvent>(type);
      initialized->InitMouseEvent(type, false, false, window, 1, 0, 0, 5, 6, false, false, false, false, 0,
                                  nullptr);
      CHECK(initialized->View() == window);
      CHECK(initialized->PageY() == 56);
      initialized->InitMouseEvent(type, false, false, nullptr, 1, 0, 0, 5, 6, false, false, false, false, 0,
                                  nullptr);
      CHECK_FALSE(initialized->View());
      CHECK(initialized->PageY() == 6);
    }

    SECTION("the events the user agent fires have the window")
    {
      std::map<std::string, bool> records;
      Ref<dom::EventListenerCallback> recorder = CreateRef<ViewRecorder>(records, window);
      for (auto name : {u8"focus", u8"focusin", u8"blur", u8"focusout", u8"click", u8"keydown", u8"keyup",
                        u8"beforeinput", u8"input", u8"textInput", u8"compositionstart",
                        u8"compositionupdate", u8"compositionend"})
      {
        window->AddEventListener(dom::DOMStringAtom(name), Ref {recorder}, true);
      }
      auto all = [&](std::initializer_list<const char *> names)
      {
        for (auto *name : names)
        {
          INFO(name);
          REQUIRE(records.contains(name));
          CHECK(records[name]);
        }
      };

      html::FocusAlgorithms::FocusingSteps(fixture.ById(u8"field"));
      html::FocusAlgorithms::FocusingSteps(fixture.ById(u8"button"));
      all({"focus", "focusin", "blur", "focusout"});

      Downcast<html::HTMLElement>(fixture.ById(u8"button")).Click();
      all({"click"});

      using Type = PlatformEvent::Type;
      (void)fixture.Frame->GetEventHandler().KeyEvent(
        PlatformKeyboardEvent(Type::RawKeyDown, u8"", u8"", u8"Shift", u8"ShiftLeft", u8"", vk::Shift, false,
                              false, false, {}, MonotonicTime::Now()));
      (void)fixture.Frame->GetEventHandler().KeyEvent(
        PlatformKeyboardEvent(Type::KeyUp, u8"", u8"", u8"Shift", u8"ShiftLeft", u8"", vk::Shift, false,
                              false, false, {}, MonotonicTime::Now()));
      all({"keydown", "keyup"});

      auto &text = fixture.TextIn(u8"editable");
      html::FocusAlgorithms::FocusingSteps(fixture.ById(u8"editable"));
      fixture.Selection().MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&text), 1),
                                 Affinity::Downstream);
      CHECK(fixture.GetEditor().InsertText(u8"z", nullptr));
      all({"beforeinput", "textInput", "input"});

      fixture.GetEditor().SetComposition(u8"k", {}, 1, 1);
      fixture.GetEditor().SetComposition(u8"ka", {}, 2, 2);
      fixture.GetEditor().ConfirmComposition(u8"ka");
      all({"compositionstart", "compositionupdate", "compositionend"});
    }
  }
}
