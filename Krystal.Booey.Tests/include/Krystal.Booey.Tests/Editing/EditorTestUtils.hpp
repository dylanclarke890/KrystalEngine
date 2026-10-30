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
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/EditorClient.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Platform/Pasteboard.hpp"
#include "Krystal.Booey/Platform/PasteboardCustomData.hpp"
#include "Krystal.Booey/Platform/PasteboardStrategy.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Booey/Platform/SelectionData.hpp"
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

    int32 DiscardedCompositions {0};
    int32 CompositionUpdates {0};
    /// @brief Whether the host's input method would be on, for the focused element.
    bool InputMethodEnabled {false};

    // Abandons the composition if the selection changed for any reason but the input method's own action.
    void RespondToChangedSelection(LocalFrame *frame) noexcept final
    {
      if (!frame || !frame->Document())
      {
        return;
      }
      auto &document = *frame->Document();
      auto &editor = document.GetEditor();
      if (editor.HasComposition() && !editor.IgnoreSelectionChanges() && !document.Selection().IsNone())
      {
        editor.CancelComposition();
        DiscardedComposition(document);
      }
    }

    void DiscardedComposition(const dom::Document &) noexcept final
    {
      ++DiscardedCompositions;
    }

    void DidUpdateComposition() noexcept final
    {
      ++CompositionUpdates;
    }

    void HandleInputMethodKeydown(dom::KeyboardEvent &event) noexcept final
    {
      auto *platformEvent = event.UnderlyingPlatformEvent();
      if (platformEvent && platformEvent->HandledByInputMethod())
      {
        event.SetDefaultHandled();
      }
    }

    void DidDispatchInputMethodKeydown(dom::KeyboardEvent &event) noexcept final
    {
      auto *platformEvent = event.UnderlyingPlatformEvent();
      auto &editor = Downcast<dom::Node>(*event.Target()).NodeDocument().GetEditor();
      if (const auto &underlines = platformEvent->PreeditUnderlines())
      {
        auto rangeStart = static_cast<uint32>(platformEvent->PreeditSelectionRangeStart().value_or(0));
        auto rangeLength = static_cast<uint32>(platformEvent->PreeditSelectionRangeLength().value_or(0));
        editor.SetComposition(platformEvent->Text(), *underlines, rangeStart, rangeStart + rangeLength);
      }
      else
      {
        editor.ConfirmComposition(platformEvent->Text());
      }
    }

    void SetInputMethodState(dom::Element *element) noexcept final
    {
      InputMethodEnabled = element && element->ShouldUseInputMethod();
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

  /// @brief A clipboard in memory, holding text, markup, files and pages' custom data as a host's would, and
  /// counting what's written to it.
  struct RecordingPasteboardStrategy final : PasteboardStrategy
  {
    int32 Writes {0};
    int64 Changes {0};
    utf8_string Text;
    utf8_string Markup;
    List<utf8_string> FilePaths;
    RefPtr<SharedBuffer> CustomData;

    KRYS_NODISCARD List<utf8_string> Types(utf8_stringview) noexcept final
    {
      List<utf8_string> types;
      if (!Text.empty())
      {
        types.emplace_back(u8"text/plain");
      }
      if (!Markup.empty())
      {
        types.emplace_back(u8"text/html");
      }
      if (!FilePaths.empty())
      {
        types.emplace_back(u8"text/uri-list");
      }
      if (CustomData)
      {
        types.emplace_back(PasteboardCustomData::BooeyType());
      }
      return types;
    }

    KRYS_NODISCARD utf8_string ReadTextFromClipboard(utf8_stringview, utf8_stringview type) noexcept final
    {
      if (type.starts_with(u8"text/plain"))
      {
        return Text;
      }
      return type == u8"text/html" ? Markup : utf8_string();
    }

    KRYS_NODISCARD List<utf8_string> ReadFilePathsFromClipboard(utf8_stringview) noexcept final
    {
      return FilePaths;
    }

    KRYS_NODISCARD RefPtr<SharedBuffer> ReadBufferFromClipboard(utf8_stringview,
                                                                utf8_stringview type) noexcept final
    {
      return type == PasteboardCustomData::BooeyType() ? CustomData : nullptr;
    }

    void WriteToClipboard(utf8_stringview pasteboardName, SelectionData &&selectionData) noexcept final
    {
      ClearClipboard(pasteboardName);
      ++Writes;
      Text = selectionData.Text();
      Markup = selectionData.Markup();
      CustomData = selectionData.CustomData();
    }

    void ClearClipboard(utf8_stringview) noexcept final
    {
      ++Changes;
      Text.clear();
      Markup.clear();
      FilePaths.clear();
      CustomData = nullptr;
    }

    KRYS_NODISCARD int64 ChangeCount(utf8_stringview) noexcept final
    {
      return Changes;
    }

    KRYS_NODISCARD List<utf8_string> TypesSafeForDOMToReadAndWrite(utf8_stringview pasteboardName,
                                                                   utf8_stringview origin) noexcept final
    {
      List<utf8_string> domTypes;
      auto add = [&](utf8_stringview type)
      {
        if (std::ranges::find(domTypes, type) == domTypes.end())
        {
          domTypes.emplace_back(type);
        }
      };
      if (CustomData)
      {
        auto customData = PasteboardCustomData::FromSharedBuffer(*CustomData);
        if (customData.Origin() == origin)
        {
          for (auto &type : customData.OrderedTypes())
          {
            add(type);
          }
        }
      }
      for (auto &type : Types(pasteboardName))
      {
        if (Pasteboard::IsSafeTypeForDOMToReadAndWrite(type))
        {
          add(type);
        }
      }
      return domTypes;
    }

    int64 WriteCustomData(const List<PasteboardCustomData> &data,
                          utf8_stringview pasteboardName) noexcept final
    {
      if (data.size() != 1)
      {
        return Changes;
      }

      SelectionData selectionData;
      const auto &customData = data[0];
      customData.ForEachPlatformString(
        [&](auto &type, auto &string)
        {
          if (type == u8"text/plain")
          {
            selectionData.SetText(string);
          }
          else if (type == u8"text/html")
          {
            selectionData.SetMarkup(string);
          }
        });
      if (customData.HasSameOriginCustomData() || !customData.Origin().empty())
      {
        selectionData.SetCustomData(customData.CreateSharedBuffer());
      }
      WriteToClipboard(pasteboardName, krys::move(selectionData));
      return Changes;
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
  /// that records writes, and the chrome client given, if any.
  struct EditorFixture : style::tests::ResolverFixture
  {
    RefPtr<Page> TestPage;
    TestEditorClient *Client {nullptr};
    RecordingPasteboardStrategy Pasteboard;

    explicit EditorFixture(utf8_stringview markup, UniquePtr<ChromeClient> chromeClient = nullptr) noexcept
        : ResolverFixture(u8"<!DOCTYPE html><style>body{margin:0;font:10px/20px sans-serif}</style>"
                            + utf8_string(markup),
                          style::tests::WithView::No)
    {
      auto client = CreateUnique<TestEditorClient>();
      Client = client.get();
      TestPage = Page::Create(krys::move(chromeClient), krys::move(client));
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
