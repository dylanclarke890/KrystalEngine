#include "Krystal.Booey/Rendering/LegacyRootInlineBox.hpp"

#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"

namespace krys::boo
{
  LegacyRootInlineBox::LegacyRootInlineBox(RenderBlockFlow &block) noexcept : LegacyInlineFlowBox(block)
  {
    SetIsHorizontal(block.IsHorizontalWritingMode());
  }

  void LegacyRootInlineBox::AdjustPosition(float dx, float dy) noexcept
  {
    LegacyInlineFlowBox::AdjustPosition(dx, dy);
    LayoutUnit blockDirectionDelta {IsHorizontal() ? dy : dx}; // The block direction delta is a LayoutUnit.
    _lineTop += blockDirectionDelta;
    _lineBottom += blockDirectionDelta;
    _lineBoxTop += blockDirectionDelta;
    _lineBoxBottom += blockDirectionDelta;
  }

  RenderObject::HighlightState LegacyRootInlineBox::SelectionState() const noexcept
  {
    using HighlightState = RenderObject::HighlightState;

    // Walk over all of the selected boxes.
    HighlightState state = HighlightState::None;
    for (auto *box = FirstLeafDescendant(); box; box = box->NextLeafOnLine())
    {
      HighlightState boxState = box->SelectionState();
      if ((boxState == HighlightState::Start && state == HighlightState::End)
          || (boxState == HighlightState::End && state == HighlightState::Start))
      {
        state = HighlightState::Both;
      }
      else if (state == HighlightState::None
               || ((boxState == HighlightState::Start || boxState == HighlightState::End)
                   && (state == HighlightState::None || state == HighlightState::Inside)))
      {
        state = boxState;
      }
      else if (boxState == HighlightState::None && state == HighlightState::Start)
      {
        // We are past the end of the selection.
        state = HighlightState::Both;
      }
      if (state == HighlightState::Both)
      {
        break;
      }
    }

    return state;
  }

  const LegacyInlineBox *LegacyRootInlineBox::FirstSelectedBox() const noexcept
  {
    for (auto *box = FirstLeafDescendant(); box; box = box->NextLeafOnLine())
    {
      if (box->SelectionState() != RenderObject::HighlightState::None)
      {
        return box;
      }
    }
    return nullptr;
  }

  const LegacyInlineBox *LegacyRootInlineBox::LastSelectedBox() const noexcept
  {
    for (auto *box = LastLeafDescendant(); box; box = box->PreviousLeafOnLine())
    {
      if (box->SelectionState() != RenderObject::HighlightState::None)
      {
        return box;
      }
    }
    return nullptr;
  }

  LayoutUnit LegacyRootInlineBox::SelectionTop() const noexcept
  {
    LayoutUnit selectionTop = _lineTop;

    if (Renderer().Style().GetWritingMode().IsLineInverted())
    {
      return selectionTop;
    }

    LayoutUnit prevBottom;
    if (auto *previousBox = PrevRootBox())
    {
      prevBottom = previousBox->SelectionBottom();
    }
    else
    {
      prevBottom = selectionTop;
    }

    return prevBottom;
  }

  LayoutUnit LegacyRootInlineBox::SelectionTopAdjustedForPrecedingBlock() const noexcept
  {
    return BlockFlow().AdjustEnclosingTopForPrecedingBlock(SelectionTop());
  }

  LayoutUnit LegacyRootInlineBox::SelectionBottom() const noexcept
  {
    LayoutUnit selectionBottom = _lineBottom;

    if (!Renderer().Style().GetWritingMode().IsLineInverted() || !NextRootBox())
    {
      return selectionBottom;
    }

    return NextRootBox()->SelectionTop();
  }

  RenderBlockFlow &LegacyRootInlineBox::BlockFlow() const noexcept
  {
    return Downcast<RenderBlockFlow>(Renderer());
  }

  void LegacyRootInlineBox::RemoveLineBoxFromRenderObject() noexcept
  {
  }

  LayoutUnit LegacyRootInlineBox::LineBoxWidth() const noexcept
  {
    return BlockFlow().AvailableLogicalWidthForLine(LineBoxTop(), LineBoxHeight());
  }
}
