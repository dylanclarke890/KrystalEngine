#include "Krystal.UI/EditorClient.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Platform/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Platform/WindowsKeyboardCodes.hpp"
#include <array>

namespace krys::ui
{
  namespace
  {
    namespace vk = boo::vk;

    constexpr uint32 CtrlKey = 1 << 0;
    constexpr uint32 AltKey = 1 << 1;
    constexpr uint32 ShiftKey = 1 << 2;

    struct KeyDownEntry
    {
      int32 VirtualKey;
      uint32 Modifiers;
      utf8_stringview Name;
    };

    struct KeyPressEntry
    {
      int32 CharCode;
      uint32 Modifiers;
      utf8_stringview Name;
    };

    constexpr auto KeyDownEntries = std::to_array<KeyDownEntry>({
      {vk::Left, 0, u8"MoveLeft"},
      {vk::Left, ShiftKey, u8"MoveLeftAndModifySelection"},
      {vk::Left, CtrlKey, u8"MoveWordLeft"},
      {vk::Left, CtrlKey | ShiftKey, u8"MoveWordLeftAndModifySelection"},
      {vk::Right, 0, u8"MoveRight"},
      {vk::Right, ShiftKey, u8"MoveRightAndModifySelection"},
      {vk::Right, CtrlKey, u8"MoveWordRight"},
      {vk::Right, CtrlKey | ShiftKey, u8"MoveWordRightAndModifySelection"},
      {vk::Up, 0, u8"MoveUp"},
      {vk::Up, ShiftKey, u8"MoveUpAndModifySelection"},
      {vk::Prior, ShiftKey, u8"MovePageUpAndModifySelection"},
      {vk::Down, 0, u8"MoveDown"},
      {vk::Down, ShiftKey, u8"MoveDownAndModifySelection"},
      {vk::Next, ShiftKey, u8"MovePageDownAndModifySelection"},
      {vk::Prior, 0, u8"MovePageUp"},
      {vk::Next, 0, u8"MovePageDown"},
      {vk::Home, 0, u8"MoveToBeginningOfLine"},
      {vk::Home, ShiftKey, u8"MoveToBeginningOfLineAndModifySelection"},
      {vk::Home, CtrlKey, u8"MoveToBeginningOfDocument"},
      {vk::Home, CtrlKey | ShiftKey, u8"MoveToBeginningOfDocumentAndModifySelection"},

      {vk::End, 0, u8"MoveToEndOfLine"},
      {vk::End, ShiftKey, u8"MoveToEndOfLineAndModifySelection"},
      {vk::End, CtrlKey, u8"MoveToEndOfDocument"},
      {vk::End, CtrlKey | ShiftKey, u8"MoveToEndOfDocumentAndModifySelection"},

      {vk::Back, 0, u8"DeleteBackward"},
      {vk::Back, ShiftKey, u8"DeleteBackward"},
      {vk::Delete, 0, u8"DeleteForward"},
      {vk::Back, CtrlKey, u8"DeleteWordBackward"},
      {vk::Delete, CtrlKey, u8"DeleteWordForward"},

      {'B', CtrlKey, u8"ToggleBold"},
      {'I', CtrlKey, u8"ToggleItalic"},

      {vk::Escape, 0, u8"Cancel"},
      {vk::OemPeriod, CtrlKey, u8"Cancel"},
      {vk::Tab, 0, u8"InsertTab"},
      {vk::Tab, ShiftKey, u8"InsertBacktab"},
      {vk::Return, 0, u8"InsertNewline"},
      {vk::Return, CtrlKey, u8"InsertNewline"},
      {vk::Return, AltKey, u8"InsertNewline"},
      {vk::Return, ShiftKey, u8"InsertNewline"},
      {vk::Return, AltKey | ShiftKey, u8"InsertNewline"},

      // It's not quite clear whether clipboard shortcuts and Undo/Redo should be handled
      // in the application or in the engine. We chose the engine.
      {'C', CtrlKey, u8"Copy"},
      {'V', CtrlKey, u8"Paste"},
      {'X', CtrlKey, u8"Cut"},
      {'A', CtrlKey, u8"SelectAll"},
      {vk::Insert, CtrlKey, u8"Copy"},
      {vk::Delete, ShiftKey, u8"Cut"},
      {vk::Insert, ShiftKey, u8"Paste"},
      {'Z', CtrlKey, u8"Undo"},
      {'Z', CtrlKey | ShiftKey, u8"Redo"},
    });

    constexpr auto KeyPressEntries = std::to_array<KeyPressEntry>({
      {'\t', 0, u8"InsertTab"},
      {'\t', ShiftKey, u8"InsertBacktab"},
      {'\r', 0, u8"InsertNewline"},
      {'\r', CtrlKey, u8"InsertNewline"},
      {'\r', AltKey, u8"InsertNewline"},
      {'\r', ShiftKey, u8"InsertNewline"},
      {'\r', AltKey | ShiftKey, u8"InsertNewline"},
    });
  }

  bool EditorClient::ShouldChangeSelectedRange(const Maybe<boo::dom::SimpleRange> &,
                                               const Maybe<boo::dom::SimpleRange> &, boo::Affinity,
                                               bool) noexcept
  {
    return true;
  }

  bool EditorClient::CanCopyCut(boo::LocalFrame *, bool defaultValue) const noexcept
  {
    return defaultValue;
  }

  bool EditorClient::CanPaste(boo::LocalFrame *, bool defaultValue) const noexcept
  {
    return defaultValue;
  }

  utf8_stringview EditorClient::InterpretKeyEvent(const boo::dom::KeyboardEvent &event) noexcept
  {
    krys_debug_assert(event.Type() == boo::EventNames::KeyDown || event.Type() == boo::EventNames::KeyPress);

    uint32 modifiers = 0;
    if (event.ShiftKey())
    {
      modifiers |= ShiftKey;
    }
    if (event.AltKey())
    {
      modifiers |= AltKey;
    }
    if (event.CtrlKey())
    {
      modifiers |= CtrlKey;
    }

    if (event.Type() == boo::EventNames::KeyDown)
    {
      for (const auto &entry : KeyDownEntries)
      {
        if (entry.VirtualKey == event.KeyCode() && entry.Modifiers == modifiers)
        {
          return entry.Name;
        }
      }
      return {};
    }

    for (const auto &entry : KeyPressEntries)
    {
      if (entry.CharCode == event.CharCode() && entry.Modifiers == modifiers)
      {
        return entry.Name;
      }
    }
    return {};
  }

  bool EditorClient::HandleKeyboardEvent(boo::dom::KeyboardEvent &event) noexcept
  {
    auto *node = DynamicDowncast<boo::dom::Node>(event.Target().get());
    if (!node)
    {
      return false;
    }

    const auto *keyEvent = event.UnderlyingPlatformEvent();
    if (!keyEvent || keyEvent->IsSystemKey()) // Do not treat this as text input if it's a system key event.
    {
      return false;
    }

    if (event.Type() != boo::EventNames::KeyDown && event.Type() != boo::EventNames::KeyPress)
    {
      return false;
    }

    auto command = node->NodeDocument().GetEditor().GetCommand(InterpretKeyEvent(event));

    if (keyEvent->GetType() == boo::PlatformEvent::Type::RawKeyDown)
    {
      // The engine doesn't have enough information about mode to decide
      // how commands that just insert text if executed via Editor
      // should be treated, so we leave it upon it to either
      // handle them immediately (e.g. Tab that changes focus) or
      // let a keypress event be generated (e.g. Tab that inserts a
      // Tab character, or Enter).
      return !command.IsTextInsertion() && command.Execute(&event);
    }

    if (command.Execute(&event))
    {
      return true;
    }

    // Don't insert null or control characters as they can result in unexpected behaviour.
    if (event.CharCode() < ' ')
    {
      return false;
    }

    return node->NodeDocument().GetEditor().InsertText(keyEvent->Text(), &event);
  }

  void EditorClient::RespondToChangedSelection(boo::LocalFrame *frame) noexcept
  {
    if (!frame || !frame->Document())
    {
      return;
    }

    // Abandon the current inline input session if selection changed for any other reason but an input method
    // direct action.
    // FIXME: Many changes that affect composition node do not go through RespondToChangedSelection(). We need
    // to do something when DOM manipulation affects the composition, because otherwise input method's idea
    // about it will be different from Editor's.
    // FIXME: We can't cancel composition when selection changes to NoSelection, but we probably should.
    auto &document = *frame->Document();
    auto &editor = document.GetEditor();
    if (editor.HasComposition() && !editor.IgnoreSelectionChanges() && !document.Selection().IsNone())
    {
      editor.CancelComposition();
      DiscardedComposition(document);
    }
  }

  void EditorClient::DiscardedComposition(const boo::dom::Document &) noexcept
  {
    if (_discardedCompositionHandler)
    {
      _discardedCompositionHandler();
    }
  }

  void EditorClient::HandleInputMethodKeydown(boo::dom::KeyboardEvent &event) noexcept
  {
    auto *platformEvent = event.UnderlyingPlatformEvent();
    if (platformEvent && platformEvent->HandledByInputMethod())
    {
      event.SetDefaultHandled();
    }
  }

  void EditorClient::DidDispatchInputMethodKeydown(boo::dom::KeyboardEvent &event) noexcept
  {
    auto *platformEvent = event.UnderlyingPlatformEvent();
    krys_debug_assert(event.Target());
    auto &editor = Downcast<boo::dom::Node>(*event.Target()).NodeDocument().GetEditor();

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

  void EditorClient::SetInputMethodState(boo::dom::Element *element) noexcept
  {
    _acceptsTextInput = element && element->ShouldUseInputMethod();
  }

  void EditorClient::RegisterUndoStep(boo::UndoStep &step) noexcept
  {
    // A new edit can't be redone over; a redone one can.
    if (!_isInRedo)
    {
      _redoStack.clear();
    }
    _undoStack.push_back(ShareRef(step));
  }

  void EditorClient::RegisterRedoStep(boo::UndoStep &step) noexcept
  {
    _redoStack.push_back(ShareRef(step));
  }

  void EditorClient::ClearUndoRedoOperations() noexcept
  {
    _undoStack.clear();
    _redoStack.clear();
  }

  bool EditorClient::CanUndo() const noexcept
  {
    return !_undoStack.empty();
  }

  bool EditorClient::CanRedo() const noexcept
  {
    return !_redoStack.empty();
  }

  void EditorClient::Undo() noexcept
  {
    if (!CanUndo())
    {
      return;
    }
    auto step = krys::move(_undoStack.back());
    _undoStack.pop_back();
    step->Unapply();
  }

  void EditorClient::Redo() noexcept
  {
    if (!CanRedo())
    {
      return;
    }
    auto step = krys::move(_redoStack.back());
    _redoStack.pop_back();
    krys_debug_assert(!_isInRedo);
    _isInRedo = true;
    step->Reapply();
    _isInRedo = false;
  }
}
