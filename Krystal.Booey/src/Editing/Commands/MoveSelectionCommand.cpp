/*
 * Copyright (C) 2005, 2006 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Editing/Commands/MoveSelectionCommand.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/Editing/Commands/DeleteSelectionCommand.hpp"
#include "Krystal.Booey/Editing/Commands/ReplaceSelectionCommand.hpp"
#include "Krystal.Booey/Editing/Selection/VisiblePosition.hpp"

namespace krys::boo
{
  MoveSelectionCommand::MoveSelectionCommand(Ref<dom::DocumentFragment> &&fragment,
                                             const dom::Position &position, bool smartInsert,
                                             bool smartDelete) noexcept
      : CompositeEditCommand(position.AnchorNode()->NodeDocument()), _fragment(krys::move(fragment)),
        _position(position), _smartInsert(smartInsert), _smartDelete(smartDelete)
  {
  }

  Ref<MoveSelectionCommand> MoveSelectionCommand::Create(Ref<dom::DocumentFragment> &&fragment,
                                                         const dom::Position &position, bool smartInsert,
                                                         bool smartDelete) noexcept
  {
    return CreateRef<MoveSelectionCommand>(krys::move(fragment), position, smartInsert, smartDelete);
  }

  void MoveSelectionCommand::DoApply() noexcept
  {
    krys_debug_assert(EndingSelection().IsNonOrphanedRange());

    dom::Position pos = _position;
    if (pos.IsNull())
    {
      return;
    }

    // Update the position otherwise it may become invalid after the selection is deleted.
    dom::Position selectionEnd = EndingSelection().End();
    if (pos.GetAnchorType() == dom::Position::AnchorType::PositionIsOffsetInAnchor
        && selectionEnd.GetAnchorType() == dom::Position::AnchorType::PositionIsOffsetInAnchor
        && selectionEnd.GetContainerNode() == pos.GetContainerNode()
        && selectionEnd.OffsetInContainerNode() < pos.OffsetInContainerNode())
    {
      pos.MoveToOffset(
        static_cast<uint32>(pos.OffsetInContainerNode() - selectionEnd.OffsetInContainerNode()));

      dom::Position selectionStart = EndingSelection().Start();
      if (selectionStart.GetAnchorType() == dom::Position::AnchorType::PositionIsOffsetInAnchor
          && selectionStart.GetContainerNode() == pos.GetContainerNode())
      {
        pos.MoveToOffset(
          static_cast<uint32>(pos.OffsetInContainerNode() + selectionStart.OffsetInContainerNode()));
      }
    }

    ApplyCommandToComposite(DeleteSelectionCommand::Create(GetDocument(), _smartDelete, true, false, true,
                                                           true, EditAction::DeleteByDrag));

    // If the node for the destination has been removed as a result of the deletion,
    // set the destination to the ending point after the deletion.
    if (!pos.AnchorNode()->IsConnected())
    {
      pos = EndingSelection().Start();
    }

    CleanupAfterDeletion(VisiblePosition(pos));

    SetEndingSelection(
      VisibleSelection(pos, EndingSelection().GetAffinity(), EndingSelection().GetDirectionality()));
    SetStartingSelection(EndingSelection());
    if (!pos.AnchorNode()->IsConnected())
    {
      // Document was modified out from under us.
      return;
    }
    EnumSet<ReplaceSelectionCommand::CommandOption> options {
      ReplaceSelectionCommand::CommandOption::SelectReplacement,
      ReplaceSelectionCommand::CommandOption::PreventNesting};
    if (_smartInsert)
    {
      options.Add(ReplaceSelectionCommand::CommandOption::SmartReplace);
    }

    ApplyCommandToComposite(
      ReplaceSelectionCommand::Create(GetDocument(), _fragment, options, EditAction::InsertFromDrop));
  }

  EditAction MoveSelectionCommand::EditingAction() const noexcept
  {
    return EditAction::DeleteByDrag;
  }
}
