#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief The extent of a line for selection and carets: from the bottom of the line above to the bottom of
  /// this one's content, so that lines' selections meet.
  /// FIXME: The line's selection state (`selectionState`) and `logicalTopAdjustedForPrecedingBlock`, once
  /// there's selection.
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
  };
}
