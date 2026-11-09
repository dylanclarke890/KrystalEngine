/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
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
