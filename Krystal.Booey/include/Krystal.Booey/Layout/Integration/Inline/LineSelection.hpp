#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief The extent of a line for selection and carets: from the bottom of the line above to the bottom of
  /// this one's content, so that lines' selections meet; and how much of the selection the line holds.
  class LineSelection
  {
  public:
    KRYS_NODISCARD static float LogicalTop(const inline_iterator::LineBox &lineBox) noexcept
    {
      return lineBox.ContentLogicalTopAdjustedForPrecedingLineBox();
    }
    KRYS_NODISCARD static float LogicalBottom(const inline_iterator::LineBox &lineBox) noexcept
    {
      return lineBox.ContentLogicalBottomAdjustedForFollowingLineBox();
    }

    KRYS_NODISCARD static FloatRect LogicalRect(const inline_iterator::LineBox &lineBox) noexcept
    {
      return {
        FloatPoint {lineBox.ContentLogicalLeft(), lineBox.ContentLogicalTopAdjustedForPrecedingLineBox()},
        FloatPoint {lineBox.ContentLogicalRight(),
                    lineBox.ContentLogicalBottomAdjustedForFollowingLineBox()}};
    }

    KRYS_NODISCARD static FloatRect PhysicalRect(const inline_iterator::LineBox &lineBox) noexcept
    {
      auto physicalRect = LogicalRect(lineBox);
      if (!lineBox.IsHorizontal())
      {
        physicalRect = physicalRect.TransposedRect();
      }
      lineBox.FormattingContextRoot().FlipForWritingMode(physicalRect);
      return physicalRect;
    }

    KRYS_NODISCARD static float
      LogicalTopAdjustedForPrecedingBlock(const inline_iterator::LineBox &lineBox) noexcept
    {
      // FIXME: Move AdjustEnclosingTopForPrecedingBlock from RenderBlockFlow to here.
      return static_cast<float>(lineBox.FormattingContextRoot().AdjustEnclosingTopForPrecedingBlock(
        LayoutUnit {lineBox.ContentLogicalTopAdjustedForPrecedingLineBox()}));
    }

    KRYS_NODISCARD static RenderObject::HighlightState
      SelectionState(const inline_iterator::LineBox &lineBox) noexcept
    {
      using HighlightState = RenderObject::HighlightState;

      const auto &root = lineBox.FormattingContextRoot();
      if (root.SelectionState() == HighlightState::None)
      {
        return HighlightState::None;
      }

      auto lineState = HighlightState::None;
      for (auto box = lineBox.LineLeftmostLeafBox(); box; box.TraverseLineRightwardOnLine())
      {
        auto boxState = box->SelectionState();
        if (lineState == HighlightState::None)
        {
          lineState = boxState;
        }
        else if (lineState == HighlightState::Start)
        {
          if (boxState == HighlightState::End || boxState == HighlightState::None)
          {
            lineState = HighlightState::Both;
          }
        }
        else if (lineState == HighlightState::Inside)
        {
          if (boxState == HighlightState::Start || boxState == HighlightState::End)
          {
            lineState = boxState;
          }
          else if (boxState == HighlightState::None)
          {
            lineState = HighlightState::End;
          }
        }
        else if (lineState == HighlightState::End)
        {
          if (boxState == HighlightState::Start)
          {
            lineState = HighlightState::Both;
          }
        }

        if (lineState == HighlightState::Both)
        {
          break;
        }
      }
      return lineState;
    }
  };
}
