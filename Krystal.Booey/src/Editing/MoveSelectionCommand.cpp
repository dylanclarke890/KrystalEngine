#include "Krystal.Booey/Editing/MoveSelectionCommand.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/Editing/DeleteSelectionCommand.hpp"
#include "Krystal.Booey/Editing/ReplaceSelectionCommand.hpp"
#include "Krystal.Booey/Editing/VisiblePosition.hpp"

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

    ApplyCommandToComposite(ReplaceSelectionCommand::Create(
      GetDocument(), RefPtr<dom::DocumentFragment>(_fragment), options, EditAction::InsertFromDrop));
  }

  EditAction MoveSelectionCommand::EditingAction() const noexcept
  {
    return EditAction::DeleteByDrag;
  }
}
