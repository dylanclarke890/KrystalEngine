/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Editing/Commands/TextInsertionBaseCommand.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Events/BeforeTextInsertedEvent.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"

namespace krys::boo
{
  TextInsertionBaseCommand::TextInsertionBaseCommand(dom::Document &document,
                                                     EditAction editingAction) noexcept
      : CompositeEditCommand(document, editingAction)
  {
  }

  void TextInsertionBaseCommand::ApplyTextInsertionCommand(LocalFrame *frame,
                                                           TextInsertionBaseCommand &command,
                                                           const VisibleSelection &selectionForInsertion,
                                                           const VisibleSelection &endingSelection) noexcept
  {
    bool changeSelection = selectionForInsertion != endingSelection;
    if (changeSelection)
    {
      command.SetStartingSelection(selectionForInsertion);
      command.SetEndingSelection(selectionForInsertion);
    }
    command.Apply();
    if (changeSelection)
    {
      command.SetEndingSelection(endingSelection);
      frame->Document()->Selection().SetSelection(endingSelection);
    }
  }

  utf8_string DispatchBeforeTextInsertedEvent(utf8_stringview text,
                                              const VisibleSelection &selectionForInsertion,
                                              bool insertionIsForUpdatingComposition) noexcept
  {
    if (insertionIsForUpdatingComposition)
    {
      return utf8_string(text);
    }

    utf8_string newText(text);
    if (RefPtr<dom::Node> startNode = ShareRefPtr(selectionForInsertion.Start().GetContainerNode()))
    {
      if (RefPtr<dom::Element> rootEditableElement = ShareRefPtr(startNode->RootEditableElement()))
      {
        // Send BeforeTextInsertedEvent. The event handler will update text if necessary.
        auto event = dom::BeforeTextInsertedEvent::Create(text);
        (void)dom::EventDispatcher::DispatchToTarget(*event, rootEditableElement.get());
        newText = event->Text();
      }
    }
    return newText;
  }

  bool CanAppendNewLineFeedToSelection(const VisibleSelection &selection) noexcept
  {
    RefPtr<dom::Element> node = ShareRefPtr(selection.RootEditableElement());
    if (!node)
    {
      return false;
    }

    auto event = dom::BeforeTextInsertedEvent::Create(u8"\n");
    (void)dom::EventDispatcher::DispatchToTarget(*event, node.get());
    return !event->Text().empty();
  }
}
