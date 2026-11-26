#include "Krystal.Web/Editing/Editor.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Dom/DOMTestUtils.hpp"
#include "Krystal.Web.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Web/ClipboardApi/ClipboardEvent.hpp"
#include "Krystal.Web/Css/CSSOM/CSSStyleDeclaration.hpp"
#include "Krystal.Web/Dom/Events/EventListener.hpp"
#include "Krystal.Web/Dom/Events/EventNames.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/DOMImplementation.hpp"
#include "Krystal.Web/Dom/Nodes/Text.hpp"
#include "Krystal.Web/Dom/Nodes/XMLDocument.hpp"
#include "Krystal.Web/Editing/Commands/ApplyStyleCommand.hpp"
#include "Krystal.Web/Editing/Commands/UndoStep.hpp"
#include "Krystal.Web/Editing/EditingStyle.hpp"
#include "Krystal.Web/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Web/Events/UI/InputEvent.hpp"
#include "Krystal.Web/Events/UI/KeyboardEvent.hpp"
#include "Krystal.Web/FileApi/FileList.hpp"
#include "Krystal.Web/Html/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Web/Html/DragAndDrop/DataTransfer.hpp"
#include "Krystal.Web/Html/Elements/HTMLInputElement.hpp"
#include "Krystal.Web/Html/Window.hpp"
#include "Krystal.Web/Infra/Namespaces.hpp"
#include "Krystal.Web/Input/EventHandler.hpp"
#include "Krystal.Web/Input/LegacyKeyCodes.hpp"
#include "Krystal.Web/Page/Clients/EditorClient.hpp"
#include "Krystal.Web/Page/Frame.hpp"
#include "Krystal.Web/Page/FrameView.hpp"
#include "Krystal.Web/Page/Settings.hpp"
#include "Krystal.Web/Platform/Pasteboard/Pasteboard.hpp"
#include "Krystal.Web/Platform/Pasteboard/PasteboardCustomData.hpp"
#include "Krystal.Web/Platform/Pasteboard/PasteboardStrategy.hpp"
#include "Krystal.Web/Platform/PlatformStrategies.hpp"

namespace krys::web::tests
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
      fixture.Selection().MoveTo(CreateContainerOffsetPosition(ShareRefPtr(&text), 4), Affinity::Downstream);
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
      fixture.Selection().MoveTo(CreateContainerOffsetPosition(ShareRefPtr(&text), 4), Affinity::Downstream);
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
      fixture.Selection().MoveTo(CreateContainerOffsetPosition(ShareRefPtr(&text), 4), Affinity::Downstream);
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
    auto beforeCut = fixture.Listen(event_names::BeforeCut);
    auto cut = fixture.Listen(event_names::Cut);

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
    auto paste = fixture.Listen(event_names::Paste);
    auto textInput = fixture.Listen(event_names::TextInput);
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

  TEST_CASE_METHOD(css::tests::ValueFixture, "Editor: pasted text keeps the font it was copied with",
                   "[Editing][Editor]")
  {
    EditorFixture fixture(u8"<style>#s { font: italic bold 20px serif }</style>"
                          u8"<p><span id=s>styled</span></p><div id=e contenteditable>ab</div>");
    auto &editor = fixture.GetEditor();
    fixture.Select(fixture.TextIn(u8"s"), 0, 6);
    editor.Copy();
    REQUIRE(Narrow(fixture.Pasteboard.Markup).contains("styled"));

    fixture.SetCaret(fixture.TextIn(u8"e"), 1);
    editor.Paste();
    auto &root = fixture.ById(u8"e");

    // The pasted text is the one text node containing it, under whichever element carries its style.
    Element *styled = nullptr;
    Func<void(Node &)> find = [&](Node &node)
    {
      if (auto *text = DynamicDowncast<Text>(node); text && Narrow(text->Data()) == "styled")
      {
        styled = text->ParentElement();
      }
      for (auto *child = node.FirstChild(); child; child = child->NextSibling())
      {
        find(*child);
      }
    };
    find(root);
    REQUIRE(styled);
    auto style = fixture.Document->Window()->GetComputedStyle(*styled);
    auto value = [&](utf8_stringview property)
    {
      return Narrow(style->GetPropertyValue(CSSOMString(property)));
    };
    CHECK(value(u8"font-style") == "italic");
    CHECK(value(u8"font-weight") == "700");
    CHECK(value(u8"font-size") == "20px");
    CHECK(value(u8"font-family") == "serif");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Editor: editing text styled with the font shorthand",
                   "[Editing][Editor]")
  {
    EditorFixture fixture(
      u8"<div id=e contenteditable><span id=s style=\"font: italic bold 20px serif\">styled</span>"
      u8"</div><div id=t contenteditable>ab</div>");
    auto &editor = fixture.GetEditor();
    auto computed = [&](Element &element, utf8_stringview property)
    {
      return Narrow(
        fixture.Document->Window()->GetComputedStyle(element)->GetPropertyValue(CSSOMString(property)));
    };
    // The element holding the text "styled" under @p root, wherever editing has moved it.
    auto styledIn = [](Node &root)
    {
      Element *found = nullptr;
      Func<void(Node &)> find = [&](Node &node)
      {
        if (auto *text = DynamicDowncast<Text>(node); text && Narrow(text->Data()) == "styled")
        {
          found = text->ParentElement();
        }
        for (auto *child = node.FirstChild(); child; child = child->NextSibling())
        {
          find(*child);
        }
      };
      find(root);
      return found;
    };

    SECTION("Copying and pasting it keeps its font")
    {
      fixture.Select(fixture.TextIn(u8"s"), 0, 6);
      editor.Copy();
      fixture.SetCaret(fixture.TextIn(u8"t"), 1);
      editor.Paste();
      auto &target = fixture.ById(u8"t");
      auto *pasted = styledIn(target);
      REQUIRE(pasted);
      CHECK(computed(*pasted, u8"font-style") == "italic");
      CHECK(computed(*pasted, u8"font-weight") == "700");
      CHECK(computed(*pasted, u8"font-size") == "20px");
      CHECK(computed(*pasted, u8"font-family") == "serif");
    }

    SECTION("Restyling it changes only the property applied")
    {
      fixture.Select(fixture.TextIn(u8"s"), 0, 6);
      auto normal = EditingStyle::Create(css::PropertyId::FontWeight, css::ValueId::Normal);
      ApplyStyleCommand::Create(*fixture.Document, normal.get(), EditAction::Bold)->Apply();
      auto &root = fixture.ById(u8"e");
      (void)root;
      auto *element = styledIn(root);
      REQUIRE(element);
      CHECK(computed(*element, u8"font-weight") == "400");
      CHECK(computed(*element, u8"font-style") == "italic");
      CHECK(computed(*element, u8"font-size") == "20px");
      CHECK(computed(*element, u8"font-family") == "serif");
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
                              keyCode, false, false, false, modifiers, MonotonicTime::Now()));
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
    void ListenWith(EditorFixture &fixture, utf8_stringview type, Func<void(Event &)> &&function) noexcept
    {
      fixture.Document->AddEventListener(
        DOMStringAtom(type), Ref<EventListenerCallback>(CreateRef<FunctionListener>(krys::move(function))),
        true);
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
      RefPtr<DataTransfer> clipboardData;
      ListenWith(fixture, event_names::Copy,
                 [&](Event &event)
                 {
                   clipboardData = ShareRefPtr(Downcast<ClipboardEvent>(event).ClipboardData());
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
        fixture, event_names::Copy, [&](Event &event)
        { Downcast<ClipboardEvent>(event).ClipboardData()->SetData(document, u8"text", u8"ignored"); });
      fixture.Select(fixture.TextIn(u8"p"), 4, 9);
      editor.Copy();
      CHECK(fixture.Pasteboard.Text == u8"quick");
    }

    SECTION("Listeners of the events before copying can't read or write")
    {
      int32 events = 0;
      ListenWith(fixture, event_names::BeforeCopy,
                 [&](Event &event)
                 {
                   ++events;
                   auto *clipboardData = Downcast<ClipboardEvent>(event).ClipboardData();
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
      List<DOMString> types;
      DOMString text;
      DOMString markup;
      ListenWith(fixture, event_names::Paste,
                 [&](Event &event)
                 {
                   auto *clipboardData = Downcast<ClipboardEvent>(event).ClipboardData();
                   REQUIRE(clipboardData);
                   CHECK_FALSE(clipboardData->CanWriteData());
                   types = clipboardData->Types(document);
                   text = clipboardData->GetData(document, u8"text");
                   markup = clipboardData->GetData(document, u8"text/html");
                   event.PreventDefault();
                 });
      fixture.SetCaret(fixture.TextIn(u8"e"), 1);
      editor.Paste();
      CHECK(types == List<DOMString> {u8"text/plain", u8"text/html"});
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
      ListenWith(fixture, event_names::Copy,
                 [&](Event &event)
                 {
                   auto *clipboardData = Downcast<ClipboardEvent>(event).ClipboardData();
                   clipboardData->SetData(document, u8"application/x-custom", u8"custom");
                   event.PreventDefault();
                 });
      DOMString custom;
      List<DOMString> types;
      ListenWith(fixture, event_names::Paste,
                 [&](Event &event)
                 {
                   auto *clipboardData = Downcast<ClipboardEvent>(event).ClipboardData();
                   types = clipboardData->Types(document);
                   custom = clipboardData->GetData(document, u8"application/x-custom");
                 });
      fixture.Select(fixture.TextIn(u8"p"), 4, 9);
      editor.Copy();
      fixture.SetCaret(fixture.TextIn(u8"e"), 1);
      editor.Paste();
      CHECK(types == List<DOMString> {u8"application/x-custom"});
      CHECK(custom == u8"custom");

      EditorFixture other(u8"<div contenteditable>x</div>");
      auto otherData = DataTransfer::CreateForCopyAndPaste(*other.Document, DataTransfer::StoreMode::Readonly,
                                                           Pasteboard::CreateForCopyAndPaste());
      other.Pasteboard.CustomData = fixture.Pasteboard.CustomData;
      CHECK(otherData->Types(*other.Document).empty());
      CHECK(otherData->GetData(*other.Document, u8"application/x-custom").empty());
    }

    SECTION("Pastes' input events carry what's pasted, except in text fields")
    {
      RefPtr<DataTransfer> dataTransfer;
      int32 events = 0;
      ListenWith(fixture, event_names::BeforeInput,
                 [&](Event &event)
                 {
                   ++events;
                   dataTransfer = Downcast<InputEvent>(event).GetDataTransfer();
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
      ListenWith(fixture, event_names::Paste,
                 [&](Event &event)
                 {
                   auto &files = Downcast<ClipboardEvent>(event).ClipboardData()->Files(document);
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
    auto &field = Downcast<HTMLInputElement>(fixture.ById(u8"field"));
    bool hadDataTransfer = true;
    int32 events = 0;
    ListenWith(fixture, event_names::BeforeInput,
               [&](Event &event)
               {
                 ++events;
                 hadDataTransfer = static_cast<bool>(Downcast<InputEvent>(event).GetDataTransfer());
               });
    field.Focus();
    fixture.Pasteboard.Text = u8"XY";
    fixture.GetEditor().Paste();
    CHECK(events == 1);
    CHECK_FALSE(hadDataTransfer);
    CHECK(Narrow(field.Value()).contains("XY"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Document: designMode",
                   "[Editing][Document][html:designMode][html:design-mode-enabled][html:editing-host]")
  {
    EditorFixture fixture(
      u8"<div id=one>hello</div><div id=locked contenteditable=false>no<b id=in>pe</b></div>"
      u8"<div id=host></div><svg id=svg></svg>");
    auto &document = *fixture.Document;
    auto &one = fixture.ById(u8"one");
    auto &locked = fixture.ById(u8"locked");
    auto readWrite = [](Element &element)
    {
      return element.Matches(u8":read-write").ReleaseValue();
    };

    SECTION("the setter takes on and off in any case, and ignores anything else")
    {
      CHECK(document.DesignMode() == u8"off");
      document.DesignMode(u8"foo");
      CHECK(document.DesignMode() == u8"off");
      document.DesignMode(u8"oN");
      CHECK(document.DesignMode() == u8"on");
      document.DesignMode(u8"foo");
      CHECK(document.DesignMode() == u8"on");
      document.DesignMode(u8"");
      CHECK(document.DesignMode() == u8"on");
      document.DesignMode(u8"OFF");
      CHECK(document.DesignMode() == u8"off");
    }

    SECTION("the document tree is editable while it's on")
    {
      CHECK_FALSE(one.IsContentEditable());
      CHECK_FALSE(readWrite(one));

      document.DesignMode(u8"on");
      CHECK(one.IsContentEditable());
      CHECK(readWrite(one));
      CHECK(readWrite(fixture.Body()));
      CHECK(document.HasEditableStyle());

      document.DesignMode(u8"off");
      CHECK_FALSE(one.IsContentEditable());
      CHECK_FALSE(readWrite(one));
    }

    SECTION("but not what's marked otherwise, nor what's outside the document tree")
    {
      auto shadow = fixture.ById(u8"host").AttachShadow({}).ReleaseValue();
      auto inShadow = document.CreateElement(u8"span").ReleaseValue();
      REQUIRE_FALSE(shadow->AppendChild(*inShadow).HasException());
      auto detached = document.CreateElement(u8"span").ReleaseValue();

      document.DesignMode(u8"on");
      CHECK_FALSE(locked.IsContentEditable());
      CHECK_FALSE(readWrite(locked));
      CHECK_FALSE(fixture.ById(u8"in").IsContentEditable());
      CHECK_FALSE(inShadow->IsContentEditable());
      CHECK_FALSE(readWrite(*inShadow));
      CHECK_FALSE(detached->IsContentEditable());
      CHECK_FALSE(readWrite(*detached));
      CHECK_FALSE(readWrite(fixture.ById(u8"svg")));
    }

    SECTION("turning it on puts a caret at the start and focuses the document element")
    {
      auto &text = fixture.TextIn(u8"one");
      fixture.Select(text, 1, 3);
      document.DesignMode(u8"on");
      CHECK(fixture.Selection().IsCaret());
      CHECK(fixture.Selection().Selection().Start().GetContainerNode() == &text);
      CHECK(fixture.Selection().Selection().Start().DeprecatedEditingOffset() == 0);
      CHECK(document.FocusedElement() == document.DocumentElement());

      // Typing can start straight away.
      CHECK(fixture.GetEditor().InsertText(u8"Z", nullptr));
      CHECK(Narrow(fixture.InnerHTML(one)) == "Zhello");
    }

    SECTION("only turning it on moves the selection and the focus")
    {
      auto &text = fixture.TextIn(u8"one");
      document.DesignMode(u8"on");
      fixture.Select(text, 1, 3);
      auto &field = fixture.ById(u8"host");
      REQUIRE_FALSE(field.SetAttribute(u8"tabindex", u8"0").HasException());
      html::FocusAlgorithms::FocusingSteps(field);
      REQUIRE(document.FocusedElement() == &field);

      document.DesignMode(u8"on");
      CHECK(fixture.Selection().Selection().Start().DeprecatedEditingOffset() == 1);
      CHECK(document.FocusedElement() == &field);

      document.DesignMode(u8"off");
      CHECK(fixture.Selection().Selection().Start().DeprecatedEditingOffset() == 1);
      CHECK(document.FocusedElement() == &field);
    }

    SECTION("text can be typed where it's editable")
    {
      document.DesignMode(u8"on");
      (void)document.UpdateLayout();
      auto &text = fixture.TextIn(u8"one");
      fixture.Selection().MoveTo(CreateContainerOffsetPosition(ShareRefPtr(&text), 2), Affinity::Downstream);
      CHECK(fixture.GetEditor().InsertText(u8"ZZ", nullptr));
      CHECK(Narrow(fixture.InnerHTML(one)) == "heZZllo");

      auto &lockedText = fixture.TextIn(u8"locked");
      fixture.Selection().MoveTo(CreateContainerOffsetPosition(ShareRefPtr(&lockedText), 1),
                                 Affinity::Downstream);
      CHECK_FALSE(fixture.GetEditor().InsertText(u8"Q", nullptr));
      CHECK(Narrow(fixture.InnerHTML(locked)) == "no<b id=\"in\">pe</b>");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Editor: commands from script", "[Editing][Editor]")
  {
    EditorFixture fixture(u8"<div id=e contenteditable>abc</div><p id=p>Not editable</p>");
    auto &document = *fixture.Document;
    auto &text = fixture.TextIn(u8"e");

    SECTION("Script can run only the commands supported from the DOM, ignoring ASCII case")
    {
      CHECK(document.QueryCommandSupported(u8"selectall").Value());
      CHECK(document.QueryCommandSupported(u8"insertText").Value());
      CHECK_FALSE(document.QueryCommandSupported(u8"MoveRight").Value());
      CHECK_FALSE(document.QueryCommandSupported(u8"NoSuchCommand").Value());
      CHECK_FALSE(document.ExecCommand(u8"MoveRight").Value());
      CHECK_FALSE(document.QueryCommandEnabled(u8"NoSuchCommand").Value());
    }

    SECTION("A command runs on the selection when it's enabled")
    {
      fixture.Select(fixture.TextIn(u8"p"), 0, 3);
      CHECK_FALSE(document.QueryCommandEnabled(u8"insertText").Value());
      CHECK_FALSE(document.ExecCommand(u8"insertText", false, u8"X").Value());

      fixture.Select(text, 1, 1);
      CHECK(document.QueryCommandEnabled(u8"insertText").Value());
      CHECK_FALSE(document.QueryCommandEnabled(u8"undo").Value());
      CHECK(document.ExecCommand(u8"InsertText", false, u8"X").Value());
      CHECK(fixture.ById(u8"e").TextContent() == u8"aXbc");
      CHECK(document.QueryCommandEnabled(u8"undo").Value());
      CHECK(document.ExecCommand(u8"undo").Value());
      CHECK(fixture.ById(u8"e").TextContent() == u8"abc");
    }

    SECTION("Commands without a state or value are false and empty")
    {
      fixture.Select(text, 0, 3);
      CHECK_FALSE(document.QueryCommandState(u8"selectAll").Value());
      CHECK_FALSE(document.QueryCommandIndeterm(u8"selectAll").Value());
      CHECK(document.QueryCommandValue(u8"selectAll").Value().empty());
      CHECK(document.QueryCommandValue(u8"NoSuchCommand").Value().empty());
    }

    SECTION("As in Chrome, a command doesn't run from within another")
    {
      fixture.Select(text, 1, 1);
      Maybe<bool> nested;
      ListenWith(fixture, u8"input",
                 [&](Event &)
                 {
                   if (!nested)
                   {
                     nested = document.ExecCommand(u8"insertText", false, u8"Y").Value();
                   }
                 });
      CHECK(document.ExecCommand(u8"insertText", false, u8"X").Value());
      REQUIRE(nested);
      CHECK_FALSE(*nested);
      CHECK(fixture.ById(u8"e").TextContent() == u8"aXbc");

      // It runs again once the first is done.
      CHECK(document.ExecCommand(u8"insertText", false, u8"Z").Value());
      CHECK(fixture.ById(u8"e").TextContent() == u8"aXZbc");
    }

    SECTION("Only HTML documents have commands")
    {
      auto svg =
        document.Implementation().CreateDocument(infra::namespaces::SVG, u8"svg", nullptr).ReleaseValue();
      auto threw = [](const auto &result)
      {
        return result.HasException() && result.GetException().Code() == ExceptionCode::InvalidStateError;
      };
      CHECK(threw(svg->ExecCommand(u8"selectAll")));
      CHECK(threw(svg->QueryCommandEnabled(u8"selectAll")));
      CHECK(threw(svg->QueryCommandIndeterm(u8"selectAll")));
      CHECK(threw(svg->QueryCommandState(u8"selectAll")));
      CHECK(threw(svg->QueryCommandSupported(u8"selectAll")));
      CHECK(threw(svg->QueryCommandValue(u8"selectAll")));
    }
  }
}
