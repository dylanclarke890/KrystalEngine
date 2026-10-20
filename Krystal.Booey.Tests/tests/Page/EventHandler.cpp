#include "Krystal.Booey/Page/EventHandler.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/FocusEvent.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Platform/WindowsKeyboardCodes.hpp"
#include <string>

namespace krys::boo::tests
{
  namespace
  {
    KRYS_NODISCARD std::string Name(const dom::EventTarget *target)
    {
      auto *node = DynamicDowncast<dom::Node>(target);
      auto *element = node ? DynamicDowncast<dom::Element>(*node) : nullptr;
      if (!element)
      {
        return "null";
      }
      auto id = element->GetAttribute(u8"id").value_or(dom::DOMString());
      auto name = id.empty() ? element->LocalName().View() : dom::DOMStringView(id);
      return std::string(name.begin(), name.end());
    }

    /// @brief Records the mouse and focus events it handles as `type:target`, with the related target of
    /// `mouseover`, `mouseout` and the focus events and the detail of presses, releases and clicks.
    struct MouseRecorder : public dom::EventListenerCallback
    {
      List<std::string> &Records;

      explicit MouseRecorder(List<std::string> &records) noexcept : Records(records)
      {
      }

      void HandleEvent(dom::Event &event) noexcept override
      {
        auto type = std::string(event.Type().View().begin(), event.Type().View().end());
        auto record = type + ":" + Name(event.Target().get());
        if (!event.IsMouseEvent() || type == "mouseover" || type == "mouseout")
        {
          auto related = event.IsMouseEvent() ? static_cast<dom::MouseEvent &>(event).RelatedTarget()
                                              : static_cast<dom::FocusEvent &>(event).RelatedTarget();
          record += "(" + Name(related.get()) + ")";
        }
        if (event.IsMouseEvent() && type != "mousemove" && type != "mouseover" && type != "mouseout"
            && type != "mouseenter" && type != "mouseleave")
        {
          record += "/" + std::to_string(static_cast<dom::MouseEvent &>(event).Detail());
        }
        Records.push_back(record);
      }
    };

    /// @brief A 200x100 view of @p markup with no body margin, which records the mouse events at its
    /// document.
    struct MouseFixture : PaintFixture
    {
      List<std::string> Records;

      explicit MouseFixture(utf8_stringview markup) noexcept
          : PaintFixture(u8"<!DOCTYPE html><style>body{margin:0;font:16px sans-serif}</style>"
                         + utf8_string(markup))
      {
        (void)Document->UpdateLayout();
        // Capturing at the document sees the events that don't bubble too.
        Ref<dom::EventListenerCallback> recorder = CreateRef<MouseRecorder>(Records);
        for (auto type : {u8"mousemove", u8"mouseover", u8"mouseout", u8"mouseenter", u8"mouseleave",
                          u8"mousedown", u8"mouseup", u8"click", u8"auxclick", u8"dblclick"})
        {
          Document->AddEventListener(dom::DOMStringAtom(type), Ref {recorder}, true);
        }
      }

      KRYS_NODISCARD EventHandler &Handler() noexcept
      {
        return Frame->GetEventHandler();
      }

      KRYS_NODISCARD static PlatformMouseEvent Event(PlatformEvent::Type type, double x, double y,
                                                     MouseButton button = MouseButton::None,
                                                     int32 clickCount = 0) noexcept
      {
        return PlatformMouseEvent(DoublePoint(x, y), DoublePoint(x, y), button, type, clickCount, {},
                                  MonotonicTime::Now());
      }

      bool Move(double x, double y) noexcept
      {
        return Handler().MouseMoved(Event(PlatformEvent::Type::MouseMoved, x, y));
      }

      bool Press(double x, double y, MouseButton button = MouseButton::Left, int32 clickCount = 1) noexcept
      {
        return Handler().HandleMousePressEvent(
          Event(PlatformEvent::Type::MousePressed, x, y, button, clickCount));
      }

      bool Release(double x, double y, MouseButton button = MouseButton::Left, int32 clickCount = 1) noexcept
      {
        return Handler().HandleMouseReleaseEvent(
          Event(PlatformEvent::Type::MouseReleased, x, y, button, clickCount));
      }

      /// @brief The records since the last call.
      KRYS_NODISCARD List<std::string> Take() noexcept
      {
        return std::exchange(Records, {});
      }

      KRYS_NODISCARD Cursor::Type CursorAt(double x, double y) noexcept
      {
        (void)Move(x, y);
        return Handler().CurrentMouseCursor().GetType();
      }
    };

    /// @brief Calls @p action when it handles an event.
    template <typename TAction>
    struct ActionListener : public dom::EventListenerCallback
    {
      TAction Action;

      explicit ActionListener(TAction action) noexcept : Action(krys::move(action))
      {
      }

      void HandleEvent(dom::Event &event) noexcept override
      {
        Action(event);
      }
    };

    template <typename TAction>
    void Listen(dom::EventTarget &target, utf8_stringview type, TAction action)
    {
      Ref<dom::EventListenerCallback> listener = CreateRef<ActionListener<TAction>>(krys::move(action));
      target.AddEventListener(dom::DOMStringAtom(type), krys::move(listener), false);
    }

    using Strings = List<std::string>;

    /// @brief Records the keyboard events it handles as `type:target/key/keyCode/charCode`.
    struct KeyRecorder : public dom::EventListenerCallback
    {
      List<std::string> &Records;

      explicit KeyRecorder(List<std::string> &records) noexcept : Records(records)
      {
      }

      void HandleEvent(dom::Event &event) noexcept override
      {
        auto &key = static_cast<dom::KeyboardEvent &>(event);
        auto type = std::string(key.Type().View().begin(), key.Type().View().end());
        Records.push_back(type + ":" + Name(event.Target().get()) + "/"
                          + std::string(key.Key().begin(), key.Key().end()) + "/"
                          + std::to_string(key.KeyCode()) + "/" + std::to_string(key.CharCode()));
      }
    };

    /// @brief A key event of @p type for a key with the value @p key that types @p text.
    KRYS_NODISCARD PlatformKeyboardEvent KeyEvent(PlatformEvent::Type type, utf8_string key, utf8_string text,
                                                  int32 keyCode,
                                                  EnumSet<PlatformEvent::Modifier> modifiers = {}) noexcept
    {
      return PlatformKeyboardEvent(type, text, text, krys::move(key), u8"", u8"", keyCode, false, false,
                                   false, modifiers, MonotonicTime::Now());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "EventHandler: moves fire over, out, enter and leave",
                   "[Page][EventHandler]")
  {
    MouseFixture fixture {u8"<div id=outer style=padding:10px><div id=inner style=height:20px></div></div>"
                          u8"<div id=other style=height:20px></div>"};

    fixture.Move(15, 15);
    CHECK(fixture.Take()
          == Strings {"mouseover:inner(null)", "mouseenter:html", "mouseenter:body", "mouseenter:outer",
                      "mouseenter:inner", "mousemove:inner"});
    CHECK(fixture.Handler().ElementUnderMouse() == &fixture.ById(u8"inner"));
    CHECK(fixture.ById(u8"inner").Hovered());

    // Moving within the element only moves.
    fixture.Move(16, 16);
    CHECK(fixture.Take() == Strings {"mousemove:inner"});

    // Out to its parent: only the inner element is left.
    fixture.Move(5, 5);
    CHECK(
      fixture.Take()
      == Strings {"mouseout:inner(outer)", "mouseleave:inner", "mouseover:outer(inner)", "mousemove:outer"});
    CHECK_FALSE(fixture.ById(u8"inner").Hovered());

    // Across to a sibling.
    fixture.Move(5, 45);
    CHECK(fixture.Take()
          == Strings {"mouseout:outer(other)", "mouseleave:outer", "mouseover:other(outer)",
                      "mouseenter:other", "mousemove:other"});

    // Out of the view: everything is left, and there's nothing to move over.
    fixture.Move(-1, -1);
    CHECK(fixture.Take()
          == Strings {"mouseout:other(null)", "mouseleave:other", "mouseleave:body", "mouseleave:html"});
    CHECK(fixture.Handler().ElementUnderMouse() == nullptr);
    CHECK_FALSE(fixture.ById(u8"other").Hovered());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "EventHandler: presses, releases and clicks",
                   "[Page][EventHandler]")
  {
    MouseFixture fixture {
      u8"<div id=outer><div id=a style=height:20px></div><div id=b style=height:20px></div>"
      u8"</div>"};
    fixture.Move(5, 5);
    (void)fixture.Take();

    SECTION("a click")
    {
      CHECK_FALSE(fixture.Press(5, 5));
      CHECK(fixture.ById(u8"a").Active());
      CHECK_FALSE(fixture.Release(5, 5));
      CHECK_FALSE(fixture.ById(u8"a").Active());
      CHECK(fixture.Take() == Strings {"mousedown:a/1", "mouseup:a/1", "click:a/1"});
    }

    SECTION("a double click")
    {
      (void)fixture.Press(5, 5, MouseButton::Left, 1);
      (void)fixture.Release(5, 5, MouseButton::Left, 1);
      (void)fixture.Press(5, 5, MouseButton::Left, 2);
      (void)fixture.Release(5, 5, MouseButton::Left, 2);
      CHECK(fixture.Take()
            == Strings {"mousedown:a/1", "mouseup:a/1", "click:a/1", "mousedown:a/2", "mouseup:a/2",
                        "click:a/2", "dblclick:a/2"});
    }

    SECTION("a release elsewhere clicks the common ancestor")
    {
      (void)fixture.Press(5, 5);
      (void)fixture.Move(5, 25);
      (void)fixture.Release(5, 25);
      CHECK(fixture.Take()
            == Strings {"mousedown:a/1", "mouseout:a(b)", "mouseleave:a", "mouseover:b(a)", "mouseenter:b",
                        "mousemove:b", "mouseup:b/1", "click:outer/1"});
    }

    SECTION("other buttons fire auxclick")
    {
      (void)fixture.Press(5, 5, MouseButton::Right);
      (void)fixture.Release(5, 5, MouseButton::Right);
      (void)fixture.Press(5, 5, MouseButton::Middle);
      (void)fixture.Release(5, 5, MouseButton::Middle);
      CHECK(fixture.Take()
            == Strings {"mousedown:a/1", "mouseup:a/1", "auxclick:a/1", "mousedown:a/1", "mouseup:a/1",
                        "auxclick:a/1"});
    }

    SECTION("cancelling swallows the event")
    {
      Listen(fixture.ById(u8"a"), u8"mousedown", [](dom::Event &event) { event.PreventDefault(); });
      CHECK(fixture.Press(5, 5));
      CHECK_FALSE(fixture.Release(5, 5));
      CHECK(fixture.Take() == Strings {"mousedown:a/1", "mouseup:a/1", "click:a/1"});
    }

    SECTION("a press outside the document isn't a click")
    {
      (void)fixture.Press(-1, -1);
      (void)fixture.Release(5, 5);
      auto records = fixture.Take();
      CHECK(std::ranges::find(records, std::string("click:a/1")) == records.end());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "EventHandler: a press focuses", "[Page][EventHandler]")
  {
    MouseFixture fixture {
      u8"<style>#outer:focus{margin-right:100px}</style>"
      u8"<div id=outer tabindex=0 style=padding:10px><div id=inner style=height:20px></div></div>"
      u8"<div id=other tabindex=-1 style=height:20px></div><div id=plain style=height:20px></div>"};
    Ref<dom::EventListenerCallback> recorder = CreateRef<MouseRecorder>(fixture.Records);
    for (auto type : {u8"focus", u8"blur", u8"focusin", u8"focusout"})
    {
      fixture.Document->AddEventListener(dom::DOMStringAtom(type), Ref {recorder}, true);
    }
    auto &outer = fixture.ById(u8"outer");
    auto &other = fixture.ById(u8"other");
    auto pressAt = [&](double x, double y)
    {
      (void)fixture.Move(x, y);
      (void)fixture.Take();
      (void)fixture.Press(x, y);
      (void)fixture.Release(x, y);
      return fixture.Take();
    };

    // The nearest focusable ancestor of the element pressed takes the focus, between the press and the
    // release; a click focus isn't focus visible.
    CHECK(pressAt(15, 15)
          == Strings {"mousedown:inner/1", "focus:outer(null)", "focusin:outer(null)", "mouseup:inner/1",
                      "click:inner/1"});
    CHECK(fixture.Document->FocusedElement() == &outer);
    CHECK(outer.Focused());
    CHECK_FALSE(outer.HasFocusVisible());
    CHECK(outer.HasFocusWithin());
    // `:focus` restyles.
    (void)fixture.Document->UpdateLayout();
    CHECK(outer.OffsetWidth() == 100);

    SECTION("pressing another focusable element moves the focus")
    {
      CHECK(pressAt(5, 45)
            == Strings {"mousedown:other/1", "blur:outer(other)", "focusout:outer(other)",
                        "focus:other(outer)", "focusin:other(outer)", "mouseup:other/1", "click:other/1"});
      CHECK(fixture.Document->FocusedElement() == &other);
      (void)fixture.Document->UpdateLayout();
      CHECK(outer.OffsetWidth() == 200);
    }

    SECTION("pressing nothing focusable focuses the viewport")
    {
      CHECK(pressAt(5, 65)
            == Strings {"mousedown:plain/1", "blur:outer(null)", "focusout:outer(null)", "mouseup:plain/1",
                        "click:plain/1"});
      CHECK(fixture.Document->FocusedElement() == nullptr);
      CHECK_FALSE(outer.Focused());
    }

    SECTION("a cancelled press keeps the focus")
    {
      Listen(other, u8"mousedown", [](dom::Event &event) { event.PreventDefault(); });
      CHECK(pressAt(5, 45) == Strings {"mousedown:other/1", "mouseup:other/1", "click:other/1"});
      CHECK(fixture.Document->FocusedElement() == &outer);
    }

    SECTION("the window losing and regaining focus")
    {
      // The focused element gets blur, without focusout, stays focused and stops matching `:focus`.
      html::FocusAlgorithms::SystemFocusChanged(*fixture.Document, false);
      CHECK(fixture.Take() == Strings {"blur:outer(null)"});
      CHECK(fixture.Document->FocusedElement() == &outer);
      CHECK_FALSE(fixture.Document->HasFocus());
      (void)fixture.Document->UpdateLayout();
      CHECK(outer.OffsetWidth() == 200);

      // Losing it again changes nothing.
      html::FocusAlgorithms::SystemFocusChanged(*fixture.Document, false);
      CHECK(fixture.Take().empty());

      html::FocusAlgorithms::SystemFocusChanged(*fixture.Document, true);
      CHECK(fixture.Take() == Strings {"focus:outer(null)"});
      (void)fixture.Document->UpdateLayout();
      CHECK(outer.OffsetWidth() == 100);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "EventHandler: keys", "[Page][EventHandler]")
  {
    MouseFixture fixture {u8"<div id=box tabindex=0 style=height:20px></div><div id=other tabindex=0></div>"};
    Ref<dom::EventListenerCallback> recorder = CreateRef<KeyRecorder>(fixture.Records);
    for (auto type : {u8"keydown", u8"keypress", u8"keyup"})
    {
      fixture.Document->AddEventListener(dom::DOMStringAtom(type), Ref {recorder}, true);
    }
    auto &handler = fixture.Handler();
    auto &box = fixture.ById(u8"box");
    using Type = PlatformEvent::Type;

    SECTION("a key that types text goes to the body when nothing is focused")
    {
      CHECK_FALSE(handler.KeyEvent(KeyEvent(Type::KeyDown, u8"a", u8"a", 'A')));
      CHECK_FALSE(handler.KeyEvent(KeyEvent(Type::KeyUp, u8"a", u8"", 'A')));
      CHECK(fixture.Take() == Strings {"keydown:body/a/65/0", "keypress:body/a/97/97", "keyup:body/a/65/0"});
    }

    SECTION("a key that types nothing has no keypress")
    {
      (void)handler.KeyEvent(KeyEvent(Type::KeyDown, u8"ArrowLeft", u8"", 0x25));
      CHECK(fixture.Take() == Strings {"keydown:body/ArrowLeft/37/0"});
    }

    SECTION("raw key downs and chars are dispatched as they are")
    {
      (void)handler.KeyEvent(KeyEvent(Type::RawKeyDown, u8"b", u8"b", 'B'));
      (void)handler.KeyEvent(KeyEvent(Type::Char, u8"b", u8"b", 'B'));
      CHECK(fixture.Take() == Strings {"keydown:body/b/66/0", "keypress:body/b/98/98"});
    }

    SECTION("a cancelled keydown has no keypress")
    {
      Listen(*fixture.Document, u8"keydown", [](dom::Event &event) { event.PreventDefault(); });
      CHECK(handler.KeyEvent(KeyEvent(Type::KeyDown, u8"a", u8"a", 'A')));
      CHECK(fixture.Take() == Strings {"keydown:body/a/65/0"});
    }

    SECTION("the focused element gets the keys and becomes focus visible")
    {
      (void)fixture.Move(5, 5);
      (void)fixture.Press(5, 5);
      (void)fixture.Release(5, 5);
      REQUIRE(box.Focused());
      CHECK_FALSE(box.HasFocusVisible());
      (void)fixture.Take();

      // Only a modifier isn't an interaction.
      (void)handler.KeyEvent(
        KeyEvent(Type::KeyDown, u8"Shift", u8"", vk::LShift, {PlatformEvent::Modifier::ShiftKey}));
      CHECK_FALSE(box.HasFocusVisible());
      // Shift and a key that types text is.
      (void)handler.KeyEvent(KeyEvent(Type::KeyDown, u8"A", u8"A", 'A', {PlatformEvent::Modifier::ShiftKey}));
      CHECK(box.HasFocusVisible());
      CHECK(fixture.Take()
            == Strings {"keydown:box/Shift/16/0", "keydown:box/A/65/0", "keypress:box/A/65/65"});
    }

    SECTION("a keydown that moves the focus sends the keypress to the new focused element")
    {
      auto &other = fixture.ById(u8"other");
      Listen(*fixture.Document, u8"keydown",
             [&](dom::Event &) { html::FocusAlgorithms::FocusingSteps(other); });
      (void)handler.KeyEvent(KeyEvent(Type::KeyDown, u8"x", u8"x", 'X'));
      CHECK(fixture.Take() == Strings {"keydown:body/x/88/0", "keypress:other/x/120/120"});
      CHECK(other.HasFocusVisible());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "EventHandler: removing the element under the mouse",
                   "[Page][EventHandler]")
  {
    MouseFixture fixture {u8"<div id=outer style=padding:10px><div id=inner style=height:20px></div></div>"};
    auto &outer = fixture.ById(u8"outer");
    Ref<dom::Element> inner = ShareRef(fixture.ById(u8"inner"));

    // The mousemove after a mouseover that removes its target goes to the target's parent.
    Listen(*inner, u8"mouseover", [&](dom::Event &) { (void)outer.RemoveChild(*inner); });
    fixture.Move(15, 15);
    auto records = fixture.Take();
    REQUIRE_FALSE(records.empty());
    CHECK(records.back() == "mousemove:outer");

    // The removed element was still the one under the mouse, so it's left (out of the document's sight) and
    // is the related target of the next mouseover.
    (void)fixture.Document->UpdateLayout();
    fixture.Move(5, 5);
    CHECK(fixture.Take() == Strings {"mouseover:outer(inner)", "mousemove:outer"});
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "EventHandler: chooses the cursor", "[Page][EventHandler]")
  {
    MouseFixture fixture {
      u8"<style>div{height:20px}</style>"
      u8"<div id=box></div><div>text</div><div><a href=x>link</a></div>"
      u8"<div style=cursor:pointer></div><div style=cursor:crosshair><span>child</span></div>"
      u8"<div style=cursor:default>text</div>"};
    // Auto: the arrow over boxes, the I-beam over text and the hand over links.
    CHECK(fixture.CursorAt(100, 10) == Cursor::Type::Pointer);
    CHECK(fixture.CursorAt(3, 30) == Cursor::Type::IBeam);
    CHECK(fixture.CursorAt(3, 50) == Cursor::Type::Hand);
    // The style's cursor, inherited.
    CHECK(fixture.CursorAt(100, 70) == Cursor::Type::Hand);
    CHECK(fixture.CursorAt(3, 90) == Cursor::Type::Cross);

    SECTION("default is the arrow, even over text")
    {
      MouseFixture text {u8"<div style=cursor:default>text</div><div style=cursor:none>text</div>"};
      CHECK(text.CursorAt(3, 10) == Cursor::Type::Pointer);
      CHECK(text.CursorAt(3, 28) == Cursor::Type::None);
    }

    SECTION("an update chooses again for a still mouse")
    {
      (void)fixture.Move(100, 10);
      (void)fixture.ById(u8"box").SetAttribute(dom::DOMStringAtom(u8"style"),
                                               dom::DOMString(u8"cursor:wait"));
      (void)fixture.Document->UpdateLayout();
      CHECK(fixture.Handler().CurrentMouseCursor().GetType() == Cursor::Type::Pointer);
      fixture.Handler().UpdateCursor();
      CHECK(fixture.Handler().CurrentMouseCursor().GetType() == Cursor::Type::Wait);
    }
  }
}
