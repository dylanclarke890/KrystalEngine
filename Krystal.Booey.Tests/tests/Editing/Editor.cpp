#include "Krystal.Booey/Editing/Editor.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/ClipboardEvent.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
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
  namespace
  {
    /// @brief An editor client that allows selection changes and binds Shift+Right to extending the
    /// selection.
    struct TestEditorClient final : EditorClient
    {
      int32 KeyEvents {0};

      KRYS_NODISCARD bool ShouldChangeSelectedRange(const Maybe<dom::SimpleRange> &,
                                                    const Maybe<dom::SimpleRange> &, Affinity,
                                                    bool) noexcept final
      {
        return true;
      }

      KRYS_NODISCARD bool CanCopyCut(LocalFrame *, bool defaultValue) const noexcept final
      {
        return defaultValue;
      }

      bool HandleKeyboardEvent(dom::KeyboardEvent &event) noexcept final
      {
        ++KeyEvents;
        if (event.Type() != EventNames::KeyDown || event.KeyCode() != vk::Right || !event.ShiftKey())
        {
          return false;
        }
        auto &document = Downcast<dom::Node>(*event.Target()).NodeDocument();
        return document.GetEditor().GetCommand(u8"MoveRightAndModifySelection").Execute(&event);
      }
    };

    /// @brief Records what's written to the clipboard.
    struct RecordingPasteboardStrategy final : PasteboardStrategy
    {
      List<std::pair<utf8_string, utf8_string>> Writes;

      void WriteToPasteboard(utf8_stringview pasteboardType, utf8_stringview text) noexcept final
      {
        Writes.emplace_back(utf8_string(pasteboardType), utf8_string(text));
      }
    };

    /// @brief Counts the events of a type, cancelling them if asked.
    struct CountingListener : public dom::EventListenerCallback
    {
      int32 Count {0};
      bool Cancel {false};
      bool LastWasTrustedClipboardEvent {false};

      void HandleEvent(dom::Event &event) noexcept override
      {
        ++Count;
        LastWasTrustedClipboardEvent = event.IsClipboardEvent() && event.IsTrusted() && event.Bubbles()
                                       && event.Cancelable() && event.Composed();
        if (Cancel)
        {
          event.PreventDefault();
        }
      }
    };

    /// @brief A document in a page whose editor client is a `TestEditorClient`, with a pasteboard strategy
    /// that records writes.
    struct EditorFixture : style::tests::ResolverFixture
    {
      RefPtr<Page> TestPage;
      TestEditorClient *Client {nullptr};
      RecordingPasteboardStrategy Pasteboard;

      explicit EditorFixture(utf8_stringview markup) noexcept
          : ResolverFixture(u8"<!DOCTYPE html><style>body{margin:0;font:10px/20px sans-serif}</style>"
                              + utf8_string(markup),
                            style::tests::WithView::No)
      {
        auto client = CreateUnique<TestEditorClient>();
        Client = client.get();
        TestPage = Page::Create(nullptr, krys::move(client));
        Frame = ShareRefPtr(&TestPage->MainFrame());
        Frame->CreateView({400, 300});
        Frame->SetDocument(ShareRefPtr<dom::Document>(Document.get()));
        (void)Document->UpdateLayout();
        PlatformStrategies::Singleton().SetPasteboardStrategy(&Pasteboard);
      }

      ~EditorFixture() noexcept
      {
        PlatformStrategies::Singleton().SetPasteboardStrategy(nullptr);
      }

      KRYS_NODISCARD dom::Text &TextIn(dom::DOMStringView id) noexcept
      {
        return Downcast<dom::Text>(*ById(id).FirstChild());
      }

      KRYS_NODISCARD Editor &GetEditor() noexcept
      {
        return Document->GetEditor();
      }

      KRYS_NODISCARD FrameSelection &Selection() noexcept
      {
        return Document->Selection();
      }

      void Select(dom::Node &node, uint32 start, uint32 end) noexcept
      {
        Selection().MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&node), start),
                           dom::MakeContainerOffsetPosition(ShareRefPtr(&node), end), Affinity::Downstream);
      }

      KRYS_NODISCARD Ref<CountingListener> Listen(utf8_stringview type) noexcept
      {
        auto listener = CreateRef<CountingListener>();
        Document->AddEventListener(dom::DOMStringAtom(type), Ref<dom::EventListenerCallback> {listener},
                                   true);
        return listener;
      }
    };
  }

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
