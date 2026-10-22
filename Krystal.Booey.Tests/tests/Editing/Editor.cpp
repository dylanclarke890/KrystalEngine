#include "Krystal.Booey/Editing/Editor.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/ClipboardEvent.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/UndoStep.hpp"
#include "Krystal.Booey/Page/EditorClient.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/PasteboardStrategy.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Booey/Platform/WindowsKeyboardCodes.hpp"

namespace krys::boo::tests
{
  TEST_CASE_METHOD(css::tests::ValueFixture, "Editor: commands", "[Editing][Editor]")
  {
    EditorFixture fixture(u8"<p id=a>The quick brown fox</p>");
    auto &editor = fixture.GetEditor();
    auto &text = fixture.TextIn(u8"a");

    SECTION("Names ignore ASCII case, and unknown commands aren't supported")
    {
      CHECK(Editor::CommandIsSupportedFromMenuOrKeyBinding(u8"selectall"));
      CHECK(Editor::CommandIsSupportedFromMenuOrKeyBinding(u8"MoveRightAndModifySelection"));
      CHECK_FALSE(Editor::CommandIsSupportedFromMenuOrKeyBinding(u8"NoSuchCommand"));
      CHECK_FALSE(Editor::CommandIsSupportedFromMenuOrKeyBinding(u8""));
      CHECK_FALSE(editor.GetCommand(u8"NoSuchCommand").IsSupported());
      CHECK_FALSE(editor.GetCommand(u8"NoSuchCommand").Execute());

      // Key bindings can run moves; the DOM (execCommand) can't.
      CHECK(editor.GetCommand(u8"MoveRight").IsSupported());
      CHECK_FALSE(editor.GetCommand(u8"MoveRight", EditorCommandSource::DOM).IsSupported());
      CHECK(editor.GetCommand(u8"SelectAll", EditorCommandSource::DOM).IsSupported());
    }

    SECTION("Moves need editable text; extending needs a range outside it")
    {
      fixture.Selection().MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&text), 4),
                                 Affinity::Downstream);
      CHECK_FALSE(editor.GetCommand(u8"MoveRight").IsEnabled());
      CHECK_FALSE(editor.GetCommand(u8"MoveRightAndModifySelection").IsEnabled());
      CHECK_FALSE(editor.GetCommand(u8"MoveRightAndModifySelection").Execute());

      fixture.Select(text, 4, 9);
      CHECK(editor.GetCommand(u8"MoveRightAndModifySelection").IsEnabled());
      CHECK(editor.GetCommand(u8"MoveRightAndModifySelection").Execute());
      CHECK(fixture.Selection().Selection().End().DeprecatedEditingOffset() == 10);
      CHECK(editor.GetCommand(u8"MoveWordRightAndModifySelection").Execute());
      CHECK(fixture.Selection().Selection().End().DeprecatedEditingOffset() == 16);
      CHECK(editor.GetCommand(u8"MoveToBeginningOfLineAndModifySelection").Execute());
      CHECK(fixture.Selection().Selection().Start().DeprecatedEditingOffset() == 0);
      CHECK(fixture.Selection().Selection().End().DeprecatedEditingOffset() == 4);
    }

    SECTION("Caret browsing enables moving a caret anywhere")
    {
      fixture.Document->GetSettings().CaretBrowsingEnabled = true;
      fixture.Selection().MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&text), 4),
                                 Affinity::Downstream);
      CHECK(editor.GetCommand(u8"MoveRight").Execute());
      CHECK(fixture.Selection().Selection().Start().DeprecatedEditingOffset() == 5);
      fixture.Document->GetSettings().CaretBrowsingEnabled = false;
    }

    SECTION("Selecting all, words and nothing")
    {
      CHECK(editor.GetCommand(u8"SelectAll").Execute());
      CHECK(fixture.Selection().IsRange());
      CHECK(editor.SelectedText() == u8"The quick brown fox");

      fixture.Select(text, 5, 6);
      CHECK(editor.GetCommand(u8"SelectWord").Execute());
      CHECK(editor.SelectedText() == u8"quick");

      CHECK(editor.GetCommand(u8"Unselect").Execute());
      CHECK(fixture.Selection().IsNone());
      CHECK_FALSE(editor.GetCommand(u8"Unselect").IsEnabled());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Editor: copying", "[Editing][Editor]")
  {
    EditorFixture fixture(u8"<p id=a>The quick brown fox</p>");
    auto &editor = fixture.GetEditor();
    auto &text = fixture.TextIn(u8"a");
    auto beforeCopy = fixture.Listen(u8"beforecopy");
    auto copy = fixture.Listen(u8"copy");

    fixture.Select(text, 4, 15);

    SECTION("Copying writes the selected text as plain text")
    {
      auto command = editor.GetCommand(u8"Copy");
      CHECK(command.IsEnabled());
      CHECK(beforeCopy->Count == 1);
      CHECK(beforeCopy->LastWasTrustedClipboardEvent);

      CHECK(command.Execute());
      CHECK(copy->Count == 1);
      CHECK(copy->LastWasTrustedClipboardEvent);
      REQUIRE(fixture.Pasteboard.Writes.size() == 1);
      CHECK(fixture.Pasteboard.Writes[0].first == u8"text/plain;charset=utf-8");
      CHECK(fixture.Pasteboard.Writes[0].second == u8"quick brown");
    }

    SECTION("A cancelled copy event does the copy itself")
    {
      copy->Cancel = true;
      editor.Copy();
      CHECK(copy->Count == 1);
      CHECK(fixture.Pasteboard.Writes.empty());
    }

    SECTION("A caret outside editable text copies nothing, but still copies")
    {
      fixture.Selection().MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&text), 4),
                                 Affinity::Downstream);
      CHECK(editor.CanCopy());
      editor.Copy();
      REQUIRE(fixture.Pasteboard.Writes.size() == 1);
      CHECK(fixture.Pasteboard.Writes[0].second.empty());
    }

    SECTION("Without a pasteboard strategy, copying does nothing")
    {
      PlatformStrategies::Singleton().SetPasteboardStrategy(nullptr);
      editor.Copy();
      CHECK(copy->Count == 1);
      CHECK(fixture.Pasteboard.Writes.empty());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Editor: cutting", "[Editing][Editor]")
  {
    EditorFixture fixture(u8"<div id=e contenteditable>The quick brown fox</div><p id=p>Not editable</p>");
    auto &editor = fixture.GetEditor();
    auto &root = fixture.ById(u8"e");
    auto beforeCut = fixture.Listen(EventNames::BeforeCut);
    auto cut = fixture.Listen(EventNames::Cut);

    SECTION("Cutting copies the selected text, then deletes it")
    {
      fixture.Select(fixture.TextIn(u8"e"), 4, 10);
      auto command = editor.GetCommand(u8"Cut");
      CHECK(command.IsEnabled());
      CHECK(beforeCut->Count == 1);
      CHECK(beforeCut->LastWasTrustedClipboardEvent);

      CHECK(command.Execute());
      CHECK(cut->Count == 1);
      CHECK(cut->LastWasTrustedClipboardEvent);
      REQUIRE(fixture.Pasteboard.Writes.size() == 1);
      CHECK(Narrow(fixture.Pasteboard.Writes[0].second) == "quick ");
      CHECK(Narrow(fixture.InnerHTML(root)) == "The brown fox");

      CHECK(fixture.Run(u8"Undo"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "The quick brown fox");
    }

    SECTION("A cancelled cut event does the cut itself")
    {
      fixture.Select(fixture.TextIn(u8"e"), 4, 10);
      cut->Cancel = true;
      editor.Cut();
      CHECK(fixture.Pasteboard.Writes.empty());
      CHECK(Narrow(fixture.InnerHTML(root)) == "The quick brown fox");
    }

    SECTION("Text outside editable content can't be cut")
    {
      fixture.Select(fixture.TextIn(u8"p"), 0, 3);
      CHECK_FALSE(editor.CanCut());
      CHECK_FALSE(editor.GetCommand(u8"Cut").IsEnabled());
      editor.Cut();
      CHECK(fixture.Pasteboard.Writes.empty());
      CHECK(Narrow(fixture.InnerHTML(fixture.ById(u8"p"))) == "Not editable");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Editor: pasting", "[Editing][Editor]")
  {
    EditorFixture fixture(u8"<div id=e contenteditable>abc</div><p id=p>Not editable</p>");
    auto &editor = fixture.GetEditor();
    auto &root = fixture.ById(u8"e");
    auto paste = fixture.Listen(EventNames::Paste);
    auto textInput = fixture.Listen(EventNames::TextInput);
    fixture.SetCaret(fixture.TextIn(u8"e"), 1);

    SECTION("Pasting inserts the clipboard's text at the selection")
    {
      fixture.Pasteboard.Contents = u8"XY";
      auto command = editor.GetCommand(u8"Paste");
      CHECK(command.IsEnabled());
      CHECK(command.Execute());
      CHECK(paste->Count == 1);
      CHECK(paste->LastWasTrustedClipboardEvent);
      CHECK(textInput->Count == 1);
      CHECK(Narrow(fixture.InnerHTML(root)) == "aXYbc");

      CHECK(fixture.Run(u8"Undo"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "abc");
    }

    SECTION("Pasted lines are merged into the paragraphs around them")
    {
      // The last line moves into the text after the selection, keeping the break between them.
      fixture.Pasteboard.Contents = u8"one\ntwo";
      editor.Paste();
      CHECK(Narrow(fixture.InnerHTML(root)) == "aone<br>twobc");
    }

    SECTION("Pasting as plain text")
    {
      fixture.Pasteboard.Contents = u8"XY";
      CHECK(fixture.Run(u8"PasteAsPlainText"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "aXYbc");
    }

    SECTION("What's cut can be pasted back")
    {
      fixture.Select(fixture.TextIn(u8"e"), 0, 1);
      editor.Cut();
      fixture.SetCaret(fixture.TextIn(u8"e"), 2);
      editor.Paste();
      CHECK(Narrow(fixture.InnerHTML(root)) == "bca");
    }

    SECTION("A cancelled paste event does the paste itself")
    {
      fixture.Pasteboard.Contents = u8"XY";
      paste->Cancel = true;
      editor.Paste();
      CHECK(Narrow(fixture.InnerHTML(root)) == "abc");
      CHECK(textInput->Count == 0);
    }

    SECTION("Nothing is pasted outside editable content")
    {
      fixture.Pasteboard.Contents = u8"XY";
      fixture.SetCaret(fixture.TextIn(u8"p"), 1);
      CHECK_FALSE(editor.CanPaste());
      editor.Paste();
      CHECK(Narrow(fixture.InnerHTML(fixture.ById(u8"p"))) == "Not editable");
    }

    SECTION("The DOM can't paste")
    {
      CHECK(editor.GetCommand(u8"Paste").IsSupported());
      CHECK_FALSE(editor.GetCommand(u8"Paste", EditorCommandSource::DOM).IsSupported());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Editor: keys go to the editor client first",
                   "[Editing][Editor]")
  {
    EditorFixture fixture(u8"<p id=a>The quick brown fox</p>");
    auto &text = fixture.TextIn(u8"a");
    fixture.Select(text, 4, 9);

    auto key = [&](int32 keyCode, EnumSet<PlatformEvent::Modifier> modifiers)
    {
      return fixture.Frame->GetEventHandler().KeyEvent(
        PlatformKeyboardEvent(PlatformEvent::Type::RawKeyDown, u8"", u8"", u8"ArrowRight", u8"ArrowRight",
                              u8"Right", keyCode, false, false, false, modifiers, MonotonicTime::Now()));
    };

    CHECK(key(vk::Right, {PlatformEvent::Modifier::ShiftKey}));
    CHECK(fixture.Client->KeyEvents == 1);
    CHECK(fixture.Selection().Selection().End().DeprecatedEditingOffset() == 10);

    // A key the client doesn't handle gets its default action.
    (void)key(vk::Right, {});
    CHECK(fixture.Client->KeyEvents == 2);
    CHECK(fixture.Selection().Selection().End().DeprecatedEditingOffset() == 10);
  }
}
