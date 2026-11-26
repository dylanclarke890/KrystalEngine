/*
 * Copyright (C) 2014 Igalia S.L.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/EditorClient.hpp"
#include "Krystal.Web/Dom/Events/EventNames.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/Node.hpp"
#include "Krystal.Web/Editing/Editor.hpp"
#include "Krystal.Web/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Web/Events/UI/KeyboardEvent.hpp"
#include "Krystal.Web/Input/LegacyKeyCodes.hpp"
#include "Krystal.Web/Input/PlatformKeyboardEvent.hpp"
#include "Krystal.Web/Page/Frame.hpp"
#include <array>

namespace krys::boo
{
  namespace
  {
    namespace vk = web::vk;

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

  bool EditorClient::ShouldChangeSelectedRange(const Maybe<web::dom::SimpleRange> &,
                                               const Maybe<web::dom::SimpleRange> &, web::Affinity,
                                               bool) noexcept
  {
    return true;
  }

  bool EditorClient::CanCopyCut(web::Frame *, bool defaultValue) const noexcept
  {
    return defaultValue;
  }

  bool EditorClient::CanPaste(web::Frame *, bool defaultValue) const noexcept
  {
    return defaultValue;
  }

  utf8_stringview EditorClient::InterpretKeyEvent(const web::KeyboardEvent &event) noexcept
  {
    krys_debug_assert(event.Type() == web::event_names::KeyDown
                      || event.Type() == web::event_names::KeyPress);

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

    if (event.Type() == web::event_names::KeyDown)
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

  bool EditorClient::HandleKeyboardEvent(web::KeyboardEvent &event) noexcept
  {
    auto *node = DynamicDowncast<web::Node>(event.Target().get());
    if (!node)
    {
      return false;
    }

    const auto *keyEvent = event.UnderlyingPlatformEvent();
    if (!keyEvent || keyEvent->IsSystemKey()) // Do not treat this as text input if it's a system key event.
    {
      return false;
    }

    if (event.Type() != web::event_names::KeyDown && event.Type() != web::event_names::KeyPress)
    {
      return false;
    }

    auto command = node->NodeDocument().GetEditor().GetCommand(InterpretKeyEvent(event));

    if (keyEvent->GetType() == web::PlatformEvent::Type::RawKeyDown)
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

  void EditorClient::RespondToChangedSelection(web::Frame *frame) noexcept
  {
    if (!frame || !frame->Document())
    {
      return;
    }

    // Abandon the current inline input session if selection changed for any other reason but an input method
    // direct action, including to no selection, or if the page's edits took the composition's text away (the
    // editor's composition is a live range, so it follows them; one that loses its text otherwise ends at the
    // input method's next update).
    auto &document = *frame->Document();
    auto &editor = document.GetEditor();
    if ((editor.HasComposition() || editor.HasLostComposition()) && !editor.IgnoreSelectionChanges())
    {
      editor.CancelComposition();
      DiscardedComposition(document);
    }
  }

  void EditorClient::DiscardedComposition(const web::Document &) noexcept
  {
    if (_discardedCompositionHandler)
    {
      _discardedCompositionHandler();
    }
  }

  void EditorClient::HandleInputMethodKeydown(web::KeyboardEvent &event) noexcept
  {
    auto *platformEvent = event.UnderlyingPlatformEvent();
    if (platformEvent && platformEvent->HandledByInputMethod())
    {
      event.SetDefaultHandled();
    }
  }

  void EditorClient::DidDispatchInputMethodKeydown(web::KeyboardEvent &event) noexcept
  {
    auto *platformEvent = event.UnderlyingPlatformEvent();
    krys_debug_assert(event.Target());
    auto &editor = Downcast<web::Node>(*event.Target()).NodeDocument().GetEditor();

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

  void EditorClient::SetInputMethodState(web::Element *element) noexcept
  {
    _acceptsTextInput = element && element->ShouldUseInputMethod();
  }

  void EditorClient::RegisterUndoStep(web::UndoStep &step) noexcept
  {
    // A new edit can't be redone over; a redone one can.
    if (!_isInRedo)
    {
      _redoStack.clear();
    }
    _undoStack.push_back(ShareRef(step));
  }

  void EditorClient::RegisterRedoStep(web::UndoStep &step) noexcept
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
