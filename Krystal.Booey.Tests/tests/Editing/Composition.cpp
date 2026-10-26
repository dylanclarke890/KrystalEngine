#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/DOM/CompositionEvent.hpp"
#include "Krystal.Booey/DOM/InputEvent.hpp"
#include "Krystal.Booey/DOM/TextEvent.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Platform/PlatformKeyboardEvent.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief Records the composition, text, input and key events as "type data", in order.
    struct EventLog : public dom::EventListenerCallback
    {
      List<std::string> Events;

      void HandleEvent(dom::Event &event) noexcept override
      {
        auto type = Narrow(event.Type().View());
        if (event.IsCompositionEvent())
        {
          Events.push_back(type + " " + Narrow(Downcast<dom::CompositionEvent>(event).Data()));
        }
        else if (event.IsInputEvent())
        {
          auto &input = Downcast<dom::InputEvent>(event);
          Events.push_back(type + " " + Narrow(input.InputType())
                           + (input.IsComposing() ? " composing" : ""));
        }
        else if (event.IsTextEvent())
        {
          Events.push_back(type + " " + Narrow(Downcast<dom::TextEvent>(event).Data()));
        }
        else if (event.IsKeyboardEvent())
        {
          auto &key = Downcast<dom::KeyboardEvent>(event);
          Events.push_back(type + " " + std::to_string(key.KeyCode())
                           + (key.IsComposing() ? " composing" : ""));
        }
      }
    };

    /// @brief Listens for the events @ref EventLog records, on @p fixture's document.
    KRYS_NODISCARD Ref<EventLog> LogEvents(EditorFixture &fixture) noexcept
    {
      auto log = CreateRef<EventLog>();
      for (auto type : {EventNames::CompositionStart, EventNames::CompositionUpdate,
                        EventNames::CompositionEnd, EventNames::BeforeInput, EventNames::Input,
                        EventNames::TextInput, EventNames::KeyDown, EventNames::KeyPress})
      {
        fixture.Document->AddEventListener(dom::DOMStringAtom(type), Ref<dom::EventListenerCallback> {log},
                                           true);
      }
      return log;
    }

    KRYS_NODISCARD CompositionUnderline Underline(uint32 start, uint32 end, bool thick = false) noexcept
    {
      return {start, end, CompositionUnderlineColor::TextColor, {}, thick};
    }

    /// @brief A key press an input method handled, with what it made of it: @p text composed, with
    /// @p underlines, or confirmed, without them.
    struct InputMethodKeyEvent : PlatformKeyboardEvent
    {
      InputMethodKeyEvent(utf8_string text, Maybe<List<CompositionUnderline>> underlines) noexcept
          : PlatformKeyboardEvent(Type::KeyDown, text, text, u8"Process", u8"KeyA", u8"U+0041", 0x41, false,
                                  false, false, {}, MonotonicTime::Now())
      {
        _handledByInputMethod = true;
        if (underlines)
        {
          _preeditSelectionRangeStart = text.size();
          _preeditSelectionRangeLength = 0;
        }
        _preeditUnderlines = krys::move(underlines);
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Editor: composing text in editable content",
                   "[Editing][Editor][Composition]")
  {
    EditorFixture fixture(u8"<div id=e contenteditable>ab</div><p id=p>xyz</p>");
    auto &editor = fixture.GetEditor();
    auto &root = fixture.ById(u8"e");
    Downcast<html::HTMLElement>(root).Focus();
    fixture.SetCaret(fixture.TextIn(u8"e"), 2);
    auto log = LogEvents(fixture);

    // "ka", then "kanji": three and six bytes.
    const utf8_string ka = u8"か";
    const utf8_string kanji = u8"漢字";

    editor.SetComposition(ka, {Underline(0, 3)}, 3, 3);

    SECTION("Starting a composition inserts its text, selected as asked, with its events")
    {
      CHECK(log->Events
            == List<std::string> {"compositionstart ", "compositionupdate " + Narrow(ka),
                                  "beforeinput insertCompositionText composing",
                                  "input insertCompositionText composing"});
      CHECK(Narrow(fixture.InnerHTML(root)) == "ab" + Narrow(ka));
      REQUIRE(editor.HasComposition());
      CHECK(editor.CompositionNode() == &fixture.TextIn(u8"e"));
      CHECK(editor.CompositionStart() == 2);
      CHECK(editor.CompositionEnd() == 5);
      // The underlines are moved to the composition's place in its text node.
      REQUIRE(editor.CustomCompositionUnderlines().size() == 1);
      CHECK(editor.CustomCompositionUnderlines()[0].StartOffset == 2);
      CHECK(editor.CustomCompositionUnderlines()[0].EndOffset == 5);
      CHECK(fixture.Selection().Selection().IsCaret());
      CHECK(fixture.Selection().Selection().Start().DeprecatedEditingOffset() == 5);
      CHECK(fixture.Client->CompositionUpdates == 1);
    }

    SECTION("Updating replaces the composition's text; the selection is relative to it")
    {
      log->Events.clear();
      editor.SetComposition(kanji, {Underline(0, 3, true), Underline(3, 6)}, 3, 6);
      CHECK(log->Events
            == List<std::string> {"compositionupdate " + Narrow(kanji),
                                  "beforeinput insertCompositionText composing",
                                  "input insertCompositionText composing"});
      CHECK(Narrow(fixture.InnerHTML(root)) == "ab" + Narrow(kanji));
      CHECK(editor.CompositionEnd() == 8);
      CHECK(editor.CustomCompositionUnderlines()[0].Thick);
      CHECK(fixture.Selection().Selection().Start().DeprecatedEditingOffset() == 5);
      CHECK(fixture.Selection().Selection().End().DeprecatedEditingOffset() == 8);

      // The selection within the composition is relative to its start (both ends).
      uint32 start = 0;
      uint32 end = 0;
      REQUIRE(editor.GetCompositionSelection(start, end));
      CHECK(start == 3);
      CHECK(end == 6);
    }

    SECTION("Confirming keeps the text, as a final insertion, and ends the composition")
    {
      log->Events.clear();
      editor.ConfirmComposition();
      CHECK_FALSE(editor.HasComposition());
      CHECK(Narrow(fixture.InnerHTML(root)) == "ab" + Narrow(ka));
      CHECK(log->Events
            == List<std::string> {"beforeinput deleteCompositionText composing",
                                  "input deleteCompositionText composing", "textInput " + Narrow(ka),
                                  "beforeinput insertFromComposition composing",
                                  "input insertFromComposition composing", "compositionend " + Narrow(ka)});
      CHECK(fixture.Selection().Selection().IsCaret());
      CHECK(fixture.Selection().Selection().Start().DeprecatedEditingOffset() == 5);
    }

    SECTION("Confirming with other text puts it in the composition's place")
    {
      editor.ConfirmComposition(kanji);
      CHECK_FALSE(editor.HasComposition());
      CHECK(Narrow(fixture.InnerHTML(root)) == "ab" + Narrow(kanji));
      CHECK(log->Events.back() == "compositionend " + Narrow(kanji));
    }

    SECTION("Cancelling drops the composition, leaving its text")
    {
      log->Events.clear();
      editor.CancelComposition();
      CHECK_FALSE(editor.HasComposition());
      CHECK(Narrow(fixture.InnerHTML(root)) == "ab" + Narrow(ka));
      CHECK(log->Events.back() == "compositionend ");
      CHECK(fixture.Client->UndoStack.size() == 1);
    }

    SECTION("Empty text cancels the composition")
    {
      log->Events.clear();
      editor.SetComposition({}, {}, 0, 0);
      CHECK_FALSE(editor.HasComposition());
      CHECK(Narrow(fixture.InnerHTML(root)) == "ab");
      CHECK(log->Events.back() == "compositionend ");
    }

    SECTION("Moving the selection out of the composition cancels it, through the editor client")
    {
      fixture.SetCaret(fixture.TextIn(u8"e"), 0);
      CHECK_FALSE(editor.HasComposition());
      CHECK(Narrow(fixture.InnerHTML(root)) == "ab" + Narrow(ka));
      CHECK(fixture.Client->DiscardedCompositions == 1);
    }

    SECTION("A confirmed composition is undone")
    {
      editor.ConfirmComposition();
      CHECK(fixture.Run(u8"Undo"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "ab");
    }

    SECTION("The composition's rect is its text's; a collapsed range's is the caret's, without its width")
    {
      auto range = editor.CompositionRange();
      REQUIRE(range);
      auto rect = editor.FirstRectForRange(*range);
      CHECK(rect.Width() > 0);
      CHECK(rect.Height() > 0);
      auto caret = editor.FirstRectForRange(fixture.CollapsedRange(fixture.TextIn(u8"e"), 2));
      CHECK(caret.Width() == 0);
      CHECK(caret.X() == rect.X());
      CHECK(caret.Height() > 0);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Editor: recomposing a selection replaces it",
                   "[Editing][Editor][Composition]")
  {
    EditorFixture fixture(u8"<div id=e contenteditable>abcd</div>");
    auto &editor = fixture.GetEditor();
    auto &root = fixture.ById(u8"e");
    Downcast<html::HTMLElement>(root).Focus();
    fixture.Select(fixture.TextIn(u8"e"), 1, 3);
    auto log = LogEvents(fixture);

    editor.SetComposition(u8"XY", {Underline(0, 2)}, 0, 2);
    // The composition starts with the text it replaces.
    CHECK(log->Events.front() == "beforeinput deleteByComposition composing");
    CHECK(std::ranges::find(log->Events, "compositionstart bc") != log->Events.end());
    CHECK(Narrow(fixture.InnerHTML(root)) == "aXYd");
    CHECK(editor.HasComposition());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Editor: composing after a no-break space",
                   "[Editing][Editor][Composition]")
  {
    // The no-break space before the composition becomes a space, a byte shorter: the composition is still
    // where its text is.
    EditorFixture fixture(u8"<div id=e contenteditable>ab&nbsp;</div>");
    auto &editor = fixture.GetEditor();
    auto &root = fixture.ById(u8"e");
    Downcast<html::HTMLElement>(root).Focus();
    auto &text = fixture.TextIn(u8"e");
    fixture.SetCaret(text, static_cast<uint32>(text.Length()));

    editor.SetComposition(u8"kanji", {Underline(0, 5)}, 5, 5);
    CHECK(Narrow(fixture.InnerHTML(root)) == "ab kanji");
    CHECK(editor.CompositionStart() == 3);
    CHECK(editor.CompositionEnd() == 8);
    CHECK(editor.CustomCompositionUnderlines()[0].StartOffset == 3);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Editor: composing text in a text field",
                   "[Editing][Editor][Composition]")
  {
    EditorFixture fixture(u8"<input id=field value=ab>");
    auto &editor = fixture.GetEditor();
    auto &field = Downcast<html::HTMLInputElement>(fixture.ById(u8"field"));
    field.Focus();
    field.SetSelectionRange(2, 2);
    auto log = LogEvents(fixture);

    editor.SetComposition(u8"xy", {Underline(0, 2)}, 2, 2);
    CHECK(Narrow(field.Value()) == "abxy");
    CHECK(editor.HasComposition());
    CHECK(log->Events.back() == "input insertCompositionText composing");
    // Events go to the field, not its inner element: the composition events cross its shadow root.
    CHECK(log->Events.front() == "compositionstart ");

    editor.ConfirmComposition();
    CHECK_FALSE(editor.HasComposition());
    CHECK(Narrow(field.Value()) == "abxy");
    CHECK(field.SelectionStart() == 4);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "EventHandler: keys an input method handled",
                   "[Editing][Editor][Composition]")
  {
    EditorFixture fixture(u8"<div id=e contenteditable>ab</div>");
    auto &editor = fixture.GetEditor();
    auto &root = fixture.ById(u8"e");
    Downcast<html::HTMLElement>(root).Focus();
    fixture.SetCaret(fixture.TextIn(u8"e"), 2);
    auto log = LogEvents(fixture);
    auto &eventHandler = fixture.Frame->GetEventHandler();

    // The key goes out as key code 229 with no keypress, and then the composition is set.
    CHECK(eventHandler.KeyEvent(InputMethodKeyEvent(u8"k", List<CompositionUnderline> {Underline(0, 1)})));
    CHECK(log->Events.front() == "keydown 229");
    CHECK(std::ranges::none_of(log->Events, [](auto &event) { return event.starts_with("keypress"); }));
    CHECK(editor.HasComposition());
    CHECK(Narrow(fixture.InnerHTML(root)) == "abk");

    // While composing, keys are composing.
    log->Events.clear();
    CHECK(eventHandler.KeyEvent(InputMethodKeyEvent(u8"か", List<CompositionUnderline> {Underline(0, 3)})));
    CHECK(log->Events.front() == "keydown 229 composing");
    CHECK(Narrow(fixture.InnerHTML(root)) == "abか");

    // A key without a preedit confirms its text.
    log->Events.clear();
    CHECK(eventHandler.KeyEvent(InputMethodKeyEvent(u8"か", null)));
    CHECK_FALSE(editor.HasComposition());
    CHECK(Narrow(fixture.InnerHTML(root)) == "abか");
    CHECK(log->Events.back() == "compositionend か");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Editor client: the input method follows the focus",
                   "[Editing][Editor][Composition]")
  {
    EditorFixture fixture(u8"<input id=text><input id=password type=password><input id=check type=checkbox>"
                          u8"<textarea id=area></textarea><div id=editable contenteditable>a</div>"
                          u8"<div id=plain tabindex=0>b</div>");
    auto focus = [&](utf8_stringview id)
    {
      Downcast<html::HTMLElement>(fixture.ById(id)).Focus();
      return fixture.Client->InputMethodEnabled;
    };

    CHECK_FALSE(fixture.Client->InputMethodEnabled);
    CHECK(focus(u8"text"));
    CHECK_FALSE(focus(u8"password"));
    CHECK_FALSE(focus(u8"check"));
    CHECK(focus(u8"area"));
    CHECK(focus(u8"editable"));
    CHECK_FALSE(focus(u8"plain"));

    // Losing the focus, by blurring or removal, turns it off.
    CHECK(focus(u8"text"));
    Downcast<html::HTMLElement>(fixture.ById(u8"text")).Blur();
    CHECK_FALSE(fixture.Client->InputMethodEnabled);
    CHECK(focus(u8"area"));
    fixture.ById(u8"area").Remove();
    CHECK_FALSE(fixture.Client->InputMethodEnabled);
  }
}
