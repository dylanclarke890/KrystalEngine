#include "Krystal.Booey/Editing/TypingCommand.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/DOM/InputEvent.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief Records the input types of the `beforeinput` and `input` events, cancelling them if asked.
    struct InputTypeListener : public dom::EventListenerCallback
    {
      List<utf8_string> InputTypes;
      bool Cancel {false};

      void HandleEvent(dom::Event &event) noexcept override
      {
        if (event.IsInputEvent())
        {
          InputTypes.emplace_back(Downcast<dom::InputEvent>(event).InputType());
        }
        if (Cancel)
        {
          event.PreventDefault();
        }
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "TypingCommand: typing text", "[Editing][TypingCommand]")
  {
    EditorFixture fixture(u8"<div id=e contenteditable>abc</div><p id=p>xyz</p>");
    auto &editor = fixture.GetEditor();
    auto &root = fixture.ById(u8"e");
    auto &text = fixture.TextIn(u8"e");
    fixture.SetCaret(text, 3);

    SECTION("Typed text is inserted at the caret, which follows it")
    {
      CHECK(editor.InsertText(u8"d", nullptr));
      CHECK(editor.InsertText(u8"e", nullptr));
      CHECK(Narrow(fixture.InnerHTML(root)) == "abcde");
      CHECK(fixture.Selection().Selection().IsCaret());
      CHECK(fixture.Selection().Selection().Start().DeprecatedEditingOffset() == 5);
    }

    SECTION("Typing replaces a range")
    {
      fixture.Select(text, 1, 2);
      CHECK(editor.InsertText(u8"XY", nullptr));
      CHECK(Narrow(fixture.InnerHTML(root)) == "aXYc");
    }

    SECTION("Typing is undone and redone as one step")
    {
      (void)editor.InsertText(u8"d", nullptr);
      (void)editor.InsertText(u8"e", nullptr);
      CHECK(fixture.Client->UndoStack.size() == 1);
      CHECK(editor.CanUndo());
      CHECK_FALSE(editor.CanRedo());

      CHECK(fixture.Run(u8"Undo"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "abc");
      CHECK(editor.CanRedo());

      CHECK(fixture.Run(u8"Redo"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "abcde");
      CHECK_FALSE(editor.CanRedo());
    }

    SECTION("Moving the selection closes the typing, so the next typing is a new step")
    {
      (void)editor.InsertText(u8"d", nullptr);
      fixture.SetCaret(fixture.TextIn(u8"e"), 0);
      (void)editor.InsertText(u8"0", nullptr);
      CHECK(Narrow(fixture.InnerHTML(root)) == "0abcd");
      CHECK(fixture.Client->UndoStack.size() == 2);

      CHECK(fixture.Run(u8"Undo"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "abcd");
    }

    SECTION("Spaces stay visible as no-break spaces where they'd collapse")
    {
      (void)editor.InsertText(u8" ", nullptr);
      CHECK(Narrow(fixture.InnerHTML(root)) == "abc&nbsp;");
      (void)editor.InsertText(u8"d", nullptr);
      CHECK(Narrow(fixture.InnerHTML(root)) == "abc d");
    }

    SECTION("Text outside editable content isn't inserted")
    {
      fixture.SetCaret(fixture.TextIn(u8"p"), 1);
      CHECK_FALSE(editor.InsertText(u8"d", nullptr));
      CHECK(Narrow(fixture.InnerHTML(fixture.ById(u8"p"))) == "xyz");
      CHECK_FALSE(editor.CanUndo());
    }

    SECTION("The insertText command inserts its value")
    {
      CHECK(editor.GetCommand(u8"InsertText", EditorCommandSource::DOM).Execute(u8"!"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "abc!");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "TypingCommand: deleting", "[Editing][TypingCommand]")
  {
    EditorFixture fixture(u8"<div id=e contenteditable>one two</div>");
    auto &root = fixture.ById(u8"e");
    auto &text = fixture.TextIn(u8"e");

    SECTION("Backspace deletes the character before the caret")
    {
      fixture.SetCaret(text, 3);
      CHECK(fixture.Run(u8"DeleteBackward"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "on two");
      CHECK(fixture.Selection().Selection().Start().DeprecatedEditingOffset() == 2);
    }

    SECTION("Forward delete deletes the character after the caret")
    {
      fixture.SetCaret(text, 0);
      CHECK(fixture.Run(u8"DeleteForward"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "ne two");
    }

    SECTION("Deleting a range deletes just the range")
    {
      fixture.Select(text, 1, 5);
      CHECK(fixture.Run(u8"DeleteBackward"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "owo");
    }

    SECTION("Deleting a word")
    {
      fixture.SetCaret(text, 7);
      CHECK(fixture.Run(u8"DeleteWordBackward"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "one&nbsp;");

      // The caret is after the no-break space, which is longer than the space it replaced.
      CHECK(fixture.Selection().Selection().Start().DeprecatedEditingOffset() == 5);
      CHECK(fixture.GetEditor().InsertText(u8"x", nullptr));
      CHECK(Narrow(fixture.InnerHTML(root)) == "one x");
    }

    SECTION("Consecutive deletes are undone together")
    {
      fixture.SetCaret(text, 7);
      CHECK(fixture.Run(u8"DeleteBackward"));
      CHECK(fixture.Run(u8"DeleteBackward"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "one t");
      CHECK(fixture.Client->UndoStack.size() == 1);
      CHECK(fixture.Run(u8"Undo"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "one two");
    }

    SECTION("Backspace at the start of the editable root does nothing")
    {
      fixture.SetCaret(text, 0);
      (void)fixture.Run(u8"DeleteBackward");
      CHECK(Narrow(fixture.InnerHTML(root)) == "one two");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "TypingCommand: line and paragraph breaks",
                   "[Editing][TypingCommand]")
  {
    EditorFixture fixture(u8"<div id=e contenteditable><div id=a>abc</div></div>");
    auto &root = fixture.ById(u8"e");
    auto &text = fixture.TextIn(u8"a");
    fixture.SetCaret(text, 1);

    SECTION("A line break is a br")
    {
      CHECK(fixture.Run(u8"InsertLineBreak"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "<div id=\"a\">a<br>bc</div>");
    }

    SECTION("A paragraph break splits the block into a clone of it, attributes and all")
    {
      CHECK(fixture.Run(u8"InsertParagraph"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "<div id=\"a\">a</div><div id=\"a\">bc</div>");

      CHECK(fixture.Run(u8"Undo"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "<div id=\"a\">abc</div>");
    }

    SECTION("A paragraph break at the end adds an empty paragraph with a placeholder")
    {
      fixture.SetCaret(text, 3);
      CHECK(fixture.Run(u8"InsertNewline"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "<div id=\"a\">abc</div><div id=\"a\"><br></div>");
      CHECK(fixture.GetEditor().InsertText(u8"d", nullptr));
      CHECK(Narrow(fixture.InnerHTML(root)) == "<div id=\"a\">abc</div><div id=\"a\">d</div>");
    }

    SECTION("Backspace at the start of a paragraph merges it into the one before")
    {
      CHECK(fixture.Run(u8"InsertParagraph"));
      CHECK(fixture.Run(u8"DeleteBackward"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "<div id=\"a\">abc</div>");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "TypingCommand: paragraphs in the editable root",
                   "[Editing][TypingCommand]")
  {
    EditorFixture fixture(u8"<div id=e contenteditable>abc</div>");
    auto &root = fixture.ById(u8"e");
    fixture.SetCaret(fixture.TextIn(u8"e"), 3);

    SECTION("A new paragraph is the default paragraph element")
    {
      CHECK(fixture.Run(u8"InsertParagraph"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "abc<div><br></div>");
    }

    SECTION("The paragraph separator can be a p")
    {
      fixture.GetEditor().SetDefaultParagraphSeparator(EditorParagraphSeparator::p);
      CHECK(fixture.Run(u8"InsertParagraph"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "abc<p><br></p>");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "TypingCommand: input events", "[Editing][TypingCommand]")
  {
    EditorFixture fixture(u8"<div id=e contenteditable>abc</div>");
    auto &editor = fixture.GetEditor();
    auto &root = fixture.ById(u8"e");
    fixture.SetCaret(fixture.TextIn(u8"e"), 3);

    auto beforeInput = CreateRef<InputTypeListener>();
    auto input = CreateRef<InputTypeListener>();
    fixture.Document->AddEventListener(dom::DOMStringAtom(EventNames::BeforeInput),
                                       Ref<dom::EventListenerCallback> {beforeInput}, true);
    fixture.Document->AddEventListener(dom::DOMStringAtom(EventNames::Input),
                                       Ref<dom::EventListenerCallback> {input}, true);
    auto textInput = fixture.Listen(EventNames::TextInput);

    SECTION("Typing fires textInput, beforeinput and input")
    {
      (void)editor.InsertText(u8"d", nullptr);
      CHECK(textInput->Count == 1);
      REQUIRE(beforeInput->InputTypes.size() == 1);
      CHECK(Narrow(beforeInput->InputTypes[0]) == "insertText");
      REQUIRE(input->InputTypes.size() == 1);
      CHECK(Narrow(input->InputTypes[0]) == "insertText");

      (void)fixture.Run(u8"DeleteBackward");
      (void)fixture.Run(u8"InsertParagraph");
      REQUIRE(input->InputTypes.size() == 3);
      CHECK(Narrow(input->InputTypes[1]) == "deleteContentBackward");
      CHECK(Narrow(input->InputTypes[2]) == "insertParagraph");
    }

    SECTION("Undoing fires historyUndo")
    {
      (void)editor.InsertText(u8"d", nullptr);
      (void)fixture.Run(u8"Undo");
      REQUIRE(input->InputTypes.size() == 2);
      CHECK(Narrow(input->InputTypes[1]) == "historyUndo");
    }

    SECTION("Cancelling beforeinput stops the edit")
    {
      beforeInput->Cancel = true;
      (void)editor.InsertText(u8"d", nullptr);
      CHECK(Narrow(fixture.InnerHTML(root)) == "abc");
      CHECK(input->InputTypes.empty());
    }

    SECTION("Cancelling textInput stops the insertion")
    {
      textInput->Cancel = true;
      (void)editor.InsertText(u8"d", nullptr);
      CHECK(Narrow(fixture.InnerHTML(root)) == "abc");
      CHECK(beforeInput->InputTypes.empty());
    }
  }
}
