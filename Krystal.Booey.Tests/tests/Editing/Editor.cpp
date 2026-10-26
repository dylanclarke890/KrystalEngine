#include "Krystal.Booey/Editing/Editor.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/ClipboardEvent.hpp"
#include "Krystal.Booey/DOM/DataTransfer.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/InputEvent.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/UndoStep.hpp"
#include "Krystal.Booey/FileAPI/FileList.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Page/EditorClient.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/Pasteboard.hpp"
#include "Krystal.Booey/Platform/PasteboardCustomData.hpp"
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

    SECTION("Copying writes the selected text and markup")
    {
      auto command = editor.GetCommand(u8"Copy");
      CHECK(command.IsEnabled());
      CHECK(beforeCopy->Count == 1);
      CHECK(beforeCopy->LastWasTrustedClipboardEvent);

      CHECK(command.Execute());
      CHECK(copy->Count == 1);
      CHECK(copy->LastWasTrustedClipboardEvent);
      CHECK(fixture.Pasteboard.Writes == 1);
      CHECK(fixture.Pasteboard.Text == u8"quick brown");
      CHECK(Narrow(fixture.Pasteboard.Markup).contains("quick brown"));
      REQUIRE(fixture.Pasteboard.CustomData);
      CHECK(PasteboardCustomData::FromSharedBuffer(*fixture.Pasteboard.CustomData).Origin()
            == fixture.Document->OriginIdentifierForPasteboard());
    }

    SECTION("A cancelled copy event does the copy itself")
    {
      copy->Cancel = true;
      editor.Copy();
      CHECK(copy->Count == 1);
      CHECK(fixture.Pasteboard.Writes == 0);
    }

    SECTION("A caret outside editable text copies nothing, but still copies")
    {
      fixture.Selection().MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&text), 4),
                                 Affinity::Downstream);
      CHECK(editor.CanCopy());
      editor.Copy();
      CHECK(fixture.Pasteboard.Writes == 1);
      CHECK(fixture.Pasteboard.Text.empty());
    }

    SECTION("Without a pasteboard strategy, copying does nothing")
    {
      PlatformStrategies::Singleton().SetPasteboardStrategy(nullptr);
      editor.Copy();
      CHECK(copy->Count == 1);
      CHECK(fixture.Pasteboard.Writes == 0);
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
      CHECK(fixture.Pasteboard.Writes == 1);
      CHECK(Narrow(fixture.Pasteboard.Text) == "quick ");
      CHECK(Narrow(fixture.InnerHTML(root)) == "The brown fox");

      CHECK(fixture.Run(u8"Undo"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "The quick brown fox");
    }

    SECTION("A cancelled cut event does the cut itself")
    {
      fixture.Select(fixture.TextIn(u8"e"), 4, 10);
      cut->Cancel = true;
      editor.Cut();
      CHECK(fixture.Pasteboard.Writes == 0);
      CHECK(Narrow(fixture.InnerHTML(root)) == "The quick brown fox");
    }

    SECTION("Text outside editable content can't be cut")
    {
      fixture.Select(fixture.TextIn(u8"p"), 0, 3);
      CHECK_FALSE(editor.CanCut());
      CHECK_FALSE(editor.GetCommand(u8"Cut").IsEnabled());
      editor.Cut();
      CHECK(fixture.Pasteboard.Writes == 0);
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
      fixture.Pasteboard.Text = u8"XY";
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
      fixture.Pasteboard.Text = u8"one\ntwo";
      editor.Paste();
      CHECK(Narrow(fixture.InnerHTML(root)) == "aone<br>twobc");
    }

    SECTION("Pasting as plain text")
    {
      fixture.Pasteboard.Text = u8"XY";
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
      fixture.Pasteboard.Text = u8"XY";
      paste->Cancel = true;
      editor.Paste();
      CHECK(Narrow(fixture.InnerHTML(root)) == "abc");
      CHECK(textInput->Count == 0);
    }

    SECTION("Nothing is pasted outside editable content")
    {
      fixture.Pasteboard.Text = u8"XY";
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

  namespace
  {
    /// @brief Calls a function with each event it handles.
    struct FunctionListener final : dom::EventListenerCallback
    {
      Func<void(dom::Event &)> Function;

      explicit FunctionListener(Func<void(dom::Event &)> &&function) noexcept : Function(krys::move(function))
      {
      }

      void HandleEvent(dom::Event &event) noexcept override
      {
        Function(event);
      }
    };

    void ListenWith(EditorFixture &fixture, utf8_stringview type,
                    Func<void(dom::Event &)> &&function) noexcept
    {
      fixture.Document->AddEventListener(
        dom::DOMStringAtom(type),
        Ref<dom::EventListenerCallback>(CreateRef<FunctionListener>(krys::move(function))), true);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Editor: clipboard events' data", "[Editing][Editor]")
  {
    EditorFixture fixture(u8"<div id=e contenteditable>abc</div><p id=p>The quick brown fox</p>");
    auto &editor = fixture.GetEditor();
    auto &document = *fixture.Document;
    auto &root = fixture.ById(u8"e");

    SECTION("A cancelled copy puts what its listeners wrote on the clipboard")
    {
      RefPtr<dom::DataTransfer> clipboardData;
      ListenWith(fixture, EventNames::Copy,
                 [&](dom::Event &event)
                 {
                   clipboardData = ShareRefPtr(Downcast<dom::ClipboardEvent>(event).ClipboardData());
                   REQUIRE(clipboardData);
                   CHECK(clipboardData->CanWriteData());
                   clipboardData->SetData(document, u8"text/plain", u8"written");
                   clipboardData->SetData(document, u8"text/html", u8"<b>written</b><script>x</script>");
                   clipboardData->SetData(document, u8"application/x-custom", u8"custom");
                   event.PreventDefault();
                 });
      fixture.Select(fixture.TextIn(u8"p"), 4, 9);
      editor.Copy();

      CHECK(fixture.Pasteboard.Writes == 1);
      CHECK(fixture.Pasteboard.Text == u8"written");
      CHECK(Narrow(fixture.Pasteboard.Markup).contains("written"));
      CHECK_FALSE(Narrow(fixture.Pasteboard.Markup).contains("script"));
      REQUIRE(fixture.Pasteboard.CustomData);
      auto customData = PasteboardCustomData::FromSharedBuffer(*fixture.Pasteboard.CustomData);
      CHECK(customData.Origin() == document.OriginIdentifierForPasteboard());
      CHECK(customData.ReadStringInCustomData(u8"application/x-custom") == u8"custom");

      // The data can't be used once the event is over.
      CHECK(clipboardData->Types(document).empty());
    }

    SECTION("An uncancelled copy copies the selection, whatever its listeners wrote")
    {
      ListenWith(
        fixture, EventNames::Copy, [&](dom::Event &event)
        { Downcast<dom::ClipboardEvent>(event).ClipboardData()->SetData(document, u8"text", u8"ignored"); });
      fixture.Select(fixture.TextIn(u8"p"), 4, 9);
      editor.Copy();
      CHECK(fixture.Pasteboard.Text == u8"quick");
    }

    SECTION("Listeners of the events before copying can't read or write")
    {
      int32 events = 0;
      ListenWith(fixture, EventNames::BeforeCopy,
                 [&](dom::Event &event)
                 {
                   ++events;
                   auto *clipboardData = Downcast<dom::ClipboardEvent>(event).ClipboardData();
                   REQUIRE(clipboardData);
                   CHECK_FALSE(clipboardData->CanReadTypes());
                   clipboardData->SetData(document, u8"text", u8"x");
                   CHECK(clipboardData->GetData(document, u8"text").empty());
                 });
      fixture.Select(fixture.TextIn(u8"p"), 4, 9);
      (void)editor.GetCommand(u8"Copy").IsEnabled();
      CHECK(events == 1);
    }

    SECTION("Paste listeners read the clipboard")
    {
      fixture.Pasteboard.Text = u8"pasted";
      fixture.Pasteboard.Markup = u8"<i>pasted</i>";
      List<dom::DOMString> types;
      dom::DOMString text;
      dom::DOMString markup;
      ListenWith(fixture, EventNames::Paste,
                 [&](dom::Event &event)
                 {
                   auto *clipboardData = Downcast<dom::ClipboardEvent>(event).ClipboardData();
                   REQUIRE(clipboardData);
                   CHECK_FALSE(clipboardData->CanWriteData());
                   types = clipboardData->Types(document);
                   text = clipboardData->GetData(document, u8"text");
                   markup = clipboardData->GetData(document, u8"text/html");
                   event.PreventDefault();
                 });
      fixture.SetCaret(fixture.TextIn(u8"e"), 1);
      editor.Paste();
      CHECK(types == List<dom::DOMString> {u8"text/plain", u8"text/html"});
      CHECK(text == u8"pasted");
      // Markup from another application is sanitised: parsed apart, and serialised with its styles inlined.
      CHECK(Narrow(markup).starts_with("<i style=\""));
      CHECK(Narrow(markup).ends_with("\">pasted</i>"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "abc");
    }

    SECTION("Rich text is pasted as markup")
    {
      fixture.Pasteboard.Text = u8"bold";
      fixture.Pasteboard.Markup = u8"<b>bold</b>";
      fixture.SetCaret(fixture.TextIn(u8"e"), 1);
      editor.Paste();
      CHECK(Narrow(fixture.InnerHTML(root)) == "a<b>bold</b>bc");
    }

    SECTION("A page reads back the custom types it copied, and other pages don't")
    {
      ListenWith(fixture, EventNames::Copy,
                 [&](dom::Event &event)
                 {
                   auto *clipboardData = Downcast<dom::ClipboardEvent>(event).ClipboardData();
                   clipboardData->SetData(document, u8"application/x-custom", u8"custom");
                   event.PreventDefault();
                 });
      dom::DOMString custom;
      List<dom::DOMString> types;
      ListenWith(fixture, EventNames::Paste,
                 [&](dom::Event &event)
                 {
                   auto *clipboardData = Downcast<dom::ClipboardEvent>(event).ClipboardData();
                   types = clipboardData->Types(document);
                   custom = clipboardData->GetData(document, u8"application/x-custom");
                 });
      fixture.Select(fixture.TextIn(u8"p"), 4, 9);
      editor.Copy();
      fixture.SetCaret(fixture.TextIn(u8"e"), 1);
      editor.Paste();
      CHECK(types == List<dom::DOMString> {u8"application/x-custom"});
      CHECK(custom == u8"custom");

      EditorFixture other(u8"<div contenteditable>x</div>");
      auto otherData = dom::DataTransfer::CreateForCopyAndPaste(
        *other.Document, dom::DataTransfer::StoreMode::Readonly, Pasteboard::CreateForCopyAndPaste());
      other.Pasteboard.CustomData = fixture.Pasteboard.CustomData;
      CHECK(otherData->Types(*other.Document).empty());
      CHECK(otherData->GetData(*other.Document, u8"application/x-custom").empty());
    }

    SECTION("Pastes' input events carry what's pasted, except in text fields")
    {
      RefPtr<dom::DataTransfer> dataTransfer;
      int32 events = 0;
      ListenWith(fixture, EventNames::BeforeInput,
                 [&](dom::Event &event)
                 {
                   ++events;
                   dataTransfer = Downcast<dom::InputEvent>(event).GetDataTransfer();
                 });
      fixture.Pasteboard.Text = u8"XY";
      fixture.SetCaret(fixture.TextIn(u8"e"), 1);
      editor.Paste();
      CHECK(events == 1);
      REQUIRE(dataTransfer);
      CHECK(dataTransfer->GetData(document, u8"text/plain") == u8"XY");
      CHECK(Narrow(dataTransfer->GetData(document, u8"text/html")).contains("XY"));
      CHECK_FALSE(dataTransfer->CanWriteData());
    }

    SECTION("Files on the clipboard are given to paste listeners")
    {
      fixture.Pasteboard.FilePaths = {u8"C:\\files\\notes.txt"};
      utf8_string name;
      ListenWith(fixture, EventNames::Paste,
                 [&](dom::Event &event)
                 {
                   auto &files = Downcast<dom::ClipboardEvent>(event).ClipboardData()->Files(document);
                   REQUIRE(files.Length() == 1);
                   name = files.Item(0)->Name();
                   event.PreventDefault();
                 });
      fixture.SetCaret(fixture.TextIn(u8"e"), 1);
      editor.Paste();
      CHECK(name == u8"notes.txt");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Editor: input events in text fields have no data transfer",
                   "[Editing][Editor]")
  {
    EditorFixture fixture(u8"<input id=field value=ab>");
    auto &field = Downcast<html::HTMLInputElement>(fixture.ById(u8"field"));
    bool hadDataTransfer = true;
    int32 events = 0;
    ListenWith(fixture, EventNames::BeforeInput,
               [&](dom::Event &event)
               {
                 ++events;
                 hadDataTransfer = static_cast<bool>(Downcast<dom::InputEvent>(event).GetDataTransfer());
               });
    field.Focus();
    fixture.Pasteboard.Text = u8"XY";
    fixture.GetEditor().Paste();
    CHECK(events == 1);
    CHECK_FALSE(hadDataTransfer);
    CHECK(Narrow(field.Value()).contains("XY"));
  }
}
