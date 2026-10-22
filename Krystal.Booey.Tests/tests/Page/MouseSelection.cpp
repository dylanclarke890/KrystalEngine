#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief Counts `selectstart` events, cancelling them if asked.
    struct SelectStartListener : public dom::EventListenerCallback
    {
      int32 Count {0};
      bool Cancel {false};

      void HandleEvent(dom::Event &event) noexcept override
      {
        ++Count;
        if (Cancel)
        {
          event.PreventDefault();
        }
      }
    };

    /// @brief A 400x200 view of @p markup with no body margin and 10px text on 20px lines, selected with the
    /// mouse.
    struct MouseSelectionFixture : PaintFixture
    {
      Ref<SelectStartListener> SelectStart = CreateRef<SelectStartListener>();

      explicit MouseSelectionFixture(utf8_stringview markup) noexcept
          : PaintFixture(u8"<!DOCTYPE html><style>body{margin:0;font:10px/20px sans-serif}p{margin:0}</style>"
                           + utf8_string(markup),
                         {400, 200})
      {
        (void)Document->UpdateLayout();
        Document->AddEventListener(dom::DOMStringAtom(u8"selectstart"),
                                   Ref<dom::EventListenerCallback> {SelectStart}, true);
      }

      KRYS_NODISCARD EventHandler &Handler() noexcept
      {
        return Frame->GetEventHandler();
      }

      KRYS_NODISCARD FrameSelection &Selection() noexcept
      {
        return Document->Selection();
      }

      KRYS_NODISCARD dom::Text &TextIn(dom::DOMStringView id) noexcept
      {
        return Downcast<dom::Text>(*ById(id).FirstChild());
      }

      KRYS_NODISCARD static PlatformMouseEvent Event(PlatformEvent::Type type, double x, double y,
                                                     MouseButton button, int32 clickCount,
                                                     EnumSet<PlatformEvent::Modifier> modifiers = {}) noexcept
      {
        return PlatformMouseEvent(DoublePoint(x, y), DoublePoint(x, y), button, type, clickCount, modifiers,
                                  MonotonicTime::Now());
      }

      void Press(double x, double y, int32 clickCount = 1,
                 EnumSet<PlatformEvent::Modifier> modifiers = {}) noexcept
      {
        (void)Handler().HandleMousePressEvent(
          Event(PlatformEvent::Type::MousePressed, x, y, MouseButton::Left, clickCount, modifiers));
      }

      void Drag(double x, double y) noexcept
      {
        (void)Handler().MouseMoved(Event(PlatformEvent::Type::MouseMoved, x, y, MouseButton::Left, 0));
      }

      void Release(double x, double y, int32 clickCount = 1) noexcept
      {
        (void)Handler().HandleMouseReleaseEvent(
          Event(PlatformEvent::Type::MouseReleased, x, y, MouseButton::Left, clickCount));
      }

      void Click(double x, double y, int32 clickCount = 1,
                 EnumSet<PlatformEvent::Modifier> modifiers = {}) noexcept
      {
        Press(x, y, clickCount, modifiers);
        Release(x, y, clickCount);
      }

      KRYS_NODISCARD int32 StartOffset() noexcept
      {
        return Selection().Selection().Start().DeprecatedEditingOffset();
      }

      KRYS_NODISCARD int32 EndOffset() noexcept
      {
        return Selection().Selection().End().DeprecatedEditingOffset();
      }
    };
  }

  TEST_CASE("Mouse selection: dragging", "[Page][EventHandler][Selection]")
  {
    MouseSelectionFixture fixture(u8"<p id=a>The quick brown fox</p><p id=b>jumps over the lazy dog</p>");
    auto &first = fixture.TextIn(u8"a");
    auto &second = fixture.TextIn(u8"b");

    // A press places a caret, firing selectstart.
    fixture.Press(1, 10);
    CHECK(fixture.Selection().IsCaret());
    CHECK(fixture.Selection().Selection().Start().DeprecatedNode() == &first);
    CHECK(fixture.StartOffset() == 0);
    CHECK(fixture.SelectStart->Count == 1);

    // Dragging extends it to the point under the mouse.
    fixture.Drag(300, 10);
    CHECK(fixture.Selection().IsRange());
    CHECK(fixture.StartOffset() == 0);
    CHECK(fixture.EndOffset() == 19);

    // Onto the next line.
    fixture.Drag(1, 30);
    CHECK(fixture.Selection().Selection().VisibleEnd()
          == VisiblePosition(dom::MakeContainerOffsetPosition(ShareRefPtr(&second), 0)));

    fixture.Release(1, 30);
    CHECK(fixture.Selection().IsRange());
    CHECK(fixture.SelectStart->Count == 1);

    // Dragging backwards keeps the anchor where the press was.
    fixture.Press(300, 30);
    fixture.Drag(1, 10);
    fixture.Release(1, 10);
    CHECK(!fixture.Selection().Selection().IsBaseFirst());
    CHECK(fixture.Selection().Selection().Start().DeprecatedNode() == &first);
    CHECK(fixture.Selection().Selection().End().DeprecatedNode() == &second);
    CHECK(fixture.EndOffset() == 23);
  }

  TEST_CASE("Mouse selection: clicks", "[Page][EventHandler][Selection]")
  {
    MouseSelectionFixture fixture(u8"<p id=a>The quick brown fox</p><p id=b>jumps over the lazy dog</p>");
    auto &first = fixture.TextIn(u8"a");
    auto &second = fixture.TextIn(u8"b");

    SECTION("A double click selects a word")
    {
      fixture.Click(28, 10);
      fixture.Click(28, 10, 2);
      CHECK(fixture.Selection().IsRange());
      CHECK(fixture.Selection().Granularity() == TextGranularity::WordGranularity);
      CHECK(fixture.StartOffset() == 4);
      CHECK(fixture.EndOffset() == 9);

      // Dragging after it extends by words.
      fixture.Press(28, 10, 2);
      fixture.Drag(66, 10);
      fixture.Release(66, 10, 2);
      CHECK(fixture.StartOffset() == 4);
      CHECK(fixture.EndOffset() == 15);
    }

    SECTION("A triple click selects a paragraph")
    {
      fixture.Click(28, 10);
      fixture.Click(28, 10, 2);
      fixture.Click(28, 10, 3);
      CHECK(fixture.Selection().Granularity() == TextGranularity::ParagraphGranularity);
      CHECK(fixture.Selection().Selection().Start().DeprecatedNode() == &first);
      CHECK(fixture.StartOffset() == 0);
      CHECK(fixture.Selection().Selection().VisibleEnd()
            == VisiblePosition(dom::MakeContainerOffsetPosition(ShareRefPtr(&second), 0)));
    }

    SECTION("Shift extends the selection")
    {
      fixture.Click(1, 10);
      fixture.Click(28, 30, 1, {PlatformEvent::Modifier::ShiftKey});
      CHECK(fixture.Selection().IsRange());
      CHECK(fixture.StartOffset() == 0);
      CHECK(fixture.Selection().Selection().End().DeprecatedNode() == &second);
      CHECK(fixture.EndOffset() > 0);
    }

    SECTION("Clicking in the selection without moving clears it")
    {
      fixture.Press(1, 10);
      fixture.Drag(300, 10);
      fixture.Release(300, 10);
      REQUIRE(fixture.Selection().IsRange());
      fixture.Click(28, 10);
      CHECK(fixture.Selection().IsNone());
    }
  }

  TEST_CASE("Mouse selection: what can't be selected", "[Page][EventHandler][Selection]")
  {
    MouseSelectionFixture fixture(
      u8"<p id=a style='-webkit-user-select:none'>Not selectable</p><p id=b>Selectable text</p>");

    SECTION("-webkit-user-select: none")
    {
      fixture.Press(5, 10);
      fixture.Drag(60, 10);
      fixture.Release(60, 10);
      CHECK(!fixture.Selection().IsRange());
    }

    SECTION("A cancelled selectstart")
    {
      fixture.SelectStart->Cancel = true;
      fixture.Press(5, 30);
      fixture.Drag(60, 30);
      fixture.Release(60, 30);
      CHECK(fixture.SelectStart->Count >= 1);
      CHECK(fixture.Selection().IsNone());
    }
  }
}
