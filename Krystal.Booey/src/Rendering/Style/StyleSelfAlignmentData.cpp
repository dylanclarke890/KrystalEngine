/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Style/StyleSelfAlignmentData.hpp"

#include "Krystal.Booey/Graphics/Geometry/BoxSides.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Booey/Text/WritingMode.hpp"

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
