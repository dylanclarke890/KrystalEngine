#include "Krystal.Booey/Rendering/Style/StyleSelfAlignmentData.hpp"

#include "Krystal.Booey/Core/BoxSides.hpp"
#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"

namespace krys::boo
{
  LayoutUnit StyleSelfAlignmentData::AdjustmentFromStartEdge(LayoutUnit extraSpace,
                                                             ItemPosition alignmentPosition,
                                                             LogicalBoxAxis containerAxis,
                                                             WritingMode containerWritingMode,
                                                             WritingMode selfWritingMode) noexcept
  {
    krys_debug_assert(ItemPosition::Auto != alignmentPosition);

    switch (alignmentPosition)
    {
      case ItemPosition::Normal:
      case ItemPosition::Stretch:
      case ItemPosition::Start:
      case ItemPosition::FlexStart:    return 0_lu;

      case ItemPosition::Center:
      case ItemPosition::AnchorCenter: return extraSpace / 2;

      case ItemPosition::End:
      case ItemPosition::FlexEnd:      return extraSpace;

      case ItemPosition::SelfStart:
        if (LogicalBoxAxis::Inline == containerAxis)
        {
          return containerWritingMode.IsInlineMatchingAny(selfWritingMode) ? 0_lu : extraSpace;
        }
        return containerWritingMode.IsBlockMatchingAny(selfWritingMode) ? 0_lu : extraSpace;
      case ItemPosition::SelfEnd:
        if (LogicalBoxAxis::Inline == containerAxis)
        {
          return containerWritingMode.IsInlineMatchingAny(selfWritingMode) ? extraSpace : 0_lu;
        }
        return containerWritingMode.IsBlockMatchingAny(selfWritingMode) ? extraSpace : 0_lu;

      case ItemPosition::Left:
        if (LogicalBoxAxis::Inline == containerAxis)
        {
          return containerWritingMode.IsBidiLTR() ? 0_lu : extraSpace;
        }
        return containerWritingMode.IsBlockLeftToRight() ? 0_lu : extraSpace;

      case ItemPosition::Right:
        if (LogicalBoxAxis::Inline == containerAxis)
        {
          return containerWritingMode.IsBidiLTR() ? extraSpace : 0_lu;
        }
        return containerWritingMode.IsBlockLeftToRight() ? extraSpace : 0_lu;

      case ItemPosition::Baseline:
        // Self-start if self block axis; else fall back to start.
        if (!selfWritingMode.IsOrthogonal(containerWritingMode))
        {
          if (LogicalBoxAxis::Inline == containerAxis)
          {
            return 0_lu;
          }
          return containerWritingMode.IsBlockOpposing(selfWritingMode) ? extraSpace : 0_lu;
        }
        if (LogicalBoxAxis::Inline == containerAxis) // Self block axis.
        {
          return containerWritingMode.IsInlineMatchingAny(selfWritingMode) ? 0_lu : extraSpace;
        }
        return 0_lu;

      case ItemPosition::LastBaseline:
        // Self-end if self block axis; else fall back to end.
        if (!selfWritingMode.IsOrthogonal(containerWritingMode))
        {
          if (LogicalBoxAxis::Inline == containerAxis)
          {
            return extraSpace;
          }
          return containerWritingMode.IsBlockOpposing(selfWritingMode) ? 0_lu : extraSpace;
        }
        if (LogicalBoxAxis::Inline == containerAxis) // Self block axis.
        {
          return containerWritingMode.IsInlineMatchingAny(selfWritingMode) ? extraSpace : 0_lu;
        }
        return extraSpace;

      default: krys_debug_assert_msg(false, "Unreachable"); return 0_lu;
    }
  }
}
