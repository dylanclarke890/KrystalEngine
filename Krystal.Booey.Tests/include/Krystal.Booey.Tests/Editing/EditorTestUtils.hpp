#pragma once

#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/Markup.hpp"
#include "Krystal.Booey/Editing/UndoStep.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/Page/EditorClient.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Platform/PasteboardStrategy.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Booey/Platform/WindowsKeyboardCodes.hpp"
#include <string>

namespace krys::boo::tests
{
  /// @brief @p text as a `std::string`, which Catch2 can print.
  KRYS_NODISCARD inline std::string Narrow(utf8_stringview text) noexcept
  {
    return std::string(reinterpret_cast<const char *>(text.data()), text.size());
  }

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

    KRYS_NODISCARD bool CanPaste(LocalFrame *, bool defaultValue) const noexcept final
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

    void RespondToChangedContents() noexcept final
    {
    }

    KRYS_NODISCARD bool SmartInsertDeleteEnabled() noexcept final
    {
      return false;
    }

    List<Ref<UndoStep>> UndoStack;
    List<Ref<UndoStep>> RedoStack;
    bool IsInRedo {false};

    void RegisterUndoStep(UndoStep &step) noexcept final
    {
      if (!IsInRedo)
      {
        RedoStack.clear();
      }
      UndoStack.push_back(ShareRef(step));
    }

    void RegisterRedoStep(UndoStep &step) noexcept final
    {
      RedoStack.push_back(ShareRef(step));
    }

    void ClearUndoRedoOperations() noexcept final
    {
      UndoStack.clear();
      RedoStack.clear();
    }

    KRYS_NODISCARD bool CanUndo() const noexcept final
    {
      return !UndoStack.empty();
    }

    KRYS_NODISCARD bool CanRedo() const noexcept final
    {
      return !RedoStack.empty();
    }

    void Undo() noexcept final
    {
      if (!CanUndo())
      {
        return;
      }
      auto step = krys::move(UndoStack.back());
      UndoStack.pop_back();
      step->Unapply();
    }

    void Redo() noexcept final
    {
      if (!CanRedo())
      {
        return;
      }
      auto step = krys::move(RedoStack.back());
      RedoStack.pop_back();
      IsInRedo = true;
      step->Reapply();
      IsInRedo = false;
    }

    /// @brief The text control notifications, in order.
    List<std::string> TextControlNotifications;

    void TextFieldDidBeginEditing(boo::dom::Element &) noexcept final
    {
      TextControlNotifications.emplace_back("begin editing");
    }

    void TextFieldDidEndEditing(boo::dom::Element &) noexcept final
    {
      TextControlNotifications.emplace_back("end editing");
    }

    void TextDidChangeInTextField(boo::dom::Element &) noexcept final
    {
      TextControlNotifications.emplace_back("field changed");
    }

    bool DoTextFieldCommandFromEvent(boo::dom::Element &, boo::dom::KeyboardEvent *) noexcept final
    {
      return false;
    }

    void TextWillBeDeletedInTextField(boo::dom::Element &) noexcept final
    {
      TextControlNotifications.emplace_back("will delete");
    }

    void TextDidChangeInTextArea(boo::dom::Element &) noexcept final
    {
      TextControlNotifications.emplace_back("area changed");
    }
  };

  /// @brief Records what's written to the clipboard, and reads back @ref Contents.
  struct RecordingPasteboardStrategy final : PasteboardStrategy
  {
    List<std::pair<utf8_string, utf8_string>> Writes;
    utf8_string Contents;

    void WriteToPasteboard(utf8_stringview pasteboardType, utf8_stringview text) noexcept final
    {
      Writes.emplace_back(utf8_string(pasteboardType), utf8_string(text));
      Contents = utf8_string(text);
    }

    KRYS_NODISCARD utf8_string ReadStringFromPasteboard(utf8_stringview pasteboardType) noexcept final
    {
      return pasteboardType.starts_with(u8"text/plain") ? Contents : utf8_string();
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

    KRYS_NODISCARD html::HTMLBodyElement &Body() noexcept
    {
      return *Document->Body();
    }

    /// @brief The markup of @p element's children.
    KRYS_NODISCARD utf8_string InnerHTML(const dom::Node &node) noexcept
    {
      return SerializeFragment(node, SerializedNodes::SubtreesOfChildren);
    }

    KRYS_NODISCARD dom::SimpleRange CollapsedRange(dom::Node &node, uint64 offset) noexcept
    {
      return {{ShareRef(node), offset}, {ShareRef(node), offset}};
    }

    void SetCaret(dom::Node &node, uint32 offset) noexcept
    {
      Selection().MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&node), offset), Affinity::Downstream);
    }

    /// @brief Runs the command named @p name, as if from a key binding.
    bool Run(utf8_stringview name, utf8_stringview value = {}) noexcept
    {
      return GetEditor().GetCommand(name).Execute(value);
    }

    KRYS_NODISCARD Ref<CountingListener> Listen(utf8_stringview type) noexcept
    {
      auto listener = CreateRef<CountingListener>();
      Document->AddEventListener(dom::DOMStringAtom(type), Ref<dom::EventListenerCallback> {listener}, true);
      return listener;
    }
  };
}
