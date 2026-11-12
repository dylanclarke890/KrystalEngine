/*
 * Copyright (C) 2017-2020 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationPagination.hpp"
#include "Krystal.Booey/Layout/Floats/PlacedFloats.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationInlineContent.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBox.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Types/HashMap.hpp"

namespace krys::boo::layout_integration
{
  Pair<List<LineAdjustment>, Maybe<LayoutRestartLine>>
    ComputeAdjustmentsForPagination(const InlineContent &inlineContent,
                                    const layout::PlacedFloats &placedFloats, bool allowLayoutRestart,
                                    RenderBlockFlow &flow) noexcept
  {
    auto lineCount = static_cast<uint64>(inlineContent.DisplayContent().LineList.size());
    List<LineAdjustment> adjustments(lineCount);

    HashMap<uint64, LayoutUnit> lineFloatBottomMap;
    for (const auto &floatBox : placedFloats.GetList())
    {
      if (!floatBox.LayoutBox())
      {
        continue;
      }

      auto &renderer = Downcast<RenderBox>(*floatBox.LayoutBox()->RendererForIntegration());
      bool isUnsplittable =
        renderer.IsUnsplittableForPagination() || renderer.Style().GetBreakInside() == BreakInside::Avoid;

      auto placedByLine = floatBox.PlacedByLine();
      if (!placedByLine)
      {
        if (isUnsplittable)
        {
          auto rect = floatBox.AbsoluteRectWithMargin();
          flow.UpdateMinimumPageHeight(rect.Top(), rect.Height());
        }
        continue;
      }

      auto floatMinimumBottom = [&]
      {
        if (isUnsplittable)
        {
          return floatBox.AbsoluteRectWithMargin().Bottom();
        }

        if (auto *block = DynamicDowncast<RenderBlockFlow>(renderer))
        {
          if (auto firstLine = inline_iterator::FirstLineBoxFor(*block))
          {
            return LayoutUnit {firstLine->LogicalBottom()};
          }
        }
        return 0_lu;
      }();

      auto [iterator, isNewEntry] = lineFloatBottomMap.try_emplace(*placedByLine, floatMinimumBottom);
      if (!isNewEntry)
      {
        iterator->second = std::max(floatMinimumBottom, iterator->second);
      }
    }

    Maybe<uint64> previousPageBreakIndex;
    Maybe<LayoutRestartLine> layoutRestartLine;

    auto widowsValue = flow.Style().GetWidows().TryValue();
    auto orphansValue = flow.Style().GetOrphans().TryValue();
    uint64 widows = widowsValue ? widowsValue->Value : 0;
    uint64 orphans = orphansValue ? orphansValue->Value : 2;

    auto accumulatedOffset = 0_lu;
    for (uint64 lineIndex = 0; lineIndex < lineCount;)
    {
      auto line = inline_iterator::LineBoxFor(inlineContent, lineIndex);
      auto floatBottom = lineFloatBottomMap.find(lineIndex);
      auto floatMinimumBottom = floatBottom != lineFloatBottomMap.end() ? floatBottom->second : 0_lu;

      auto adjustment = flow.ComputeLineAdjustmentForPagination(line, accumulatedOffset, floatMinimumBottom);
      if (layoutRestartLine && layoutRestartLine->Index == lineIndex)
      {
        krys_debug_assert(!layoutRestartLine->Offset);
        layoutRestartLine->Offset = adjustment.Strut;
      }

      if (adjustment.IsFirstAfterPageBreak)
      {
        auto remainingLines = lineCount - lineIndex;
        // Ignore the last line if it is completely empty.
        if (inlineContent.DisplayContent().LineList.back().LineBoxRect().IsEmpty())
        {
          remainingLines--;
        }

        // See if there are enough lines left to meet the widow requirement.
        if (remainingLines < widows && allowLayoutRestart && !layoutRestartLine)
        {
          auto previousPageLineCount = lineIndex - previousPageBreakIndex.value_or(0);
          auto neededLines = widows - remainingLines;
          auto availableLines = previousPageLineCount > orphans ? previousPageLineCount - orphans : 0;
          auto linesToMove = std::min(neededLines, availableLines);
          if (linesToMove)
          {
            auto breakIndex = lineIndex - linesToMove;
            // Set the widow break and recompute the adjustments starting from that line.
            flow.SetBreakAtLineToAvoidWidow(static_cast<int32>(breakIndex + 1));
            lineIndex = breakIndex;
            // We need to redo the layout starting from the break for things like intrusive floats.
            layoutRestartLine = LayoutRestartLine {breakIndex, {}};
            continue;
          }
        }

        previousPageBreakIndex = lineIndex;
      }

      accumulatedOffset += adjustment.Strut;

      if (adjustment.IsFirstAfterPageBreak && !lineIndex)
      {
        accumulatedOffset += LayoutUnit {inlineContent.ClearGapBeforeFirstLine()};
      }

      adjustments[lineIndex] = LineAdjustment {accumulatedOffset, adjustment.IsFirstAfterPageBreak};

      ++lineIndex;
    }

    flow.ClearDidBreakAtLineToAvoidWidow();

    if (!previousPageBreakIndex)
    {
      return {};
    }

    return {krys::move(adjustments), layoutRestartLine};
  }

  void AdjustLinePositionsForPagination(InlineContent &inlineContent,
                                        const List<LineAdjustment> &adjustments) noexcept
  {
    if (adjustments.empty())
    {
      return;
    }

    bool isHorizontalWritingMode =
      inlineContent.FormattingContextRoot().Style().GetWritingMode().IsHorizontal();

    auto &displayContent = inlineContent.DisplayContent();
    for (uint64 lineIndex = 0; lineIndex < displayContent.LineList.size(); ++lineIndex)
    {
      auto &line = displayContent.LineList[lineIndex];
      const auto &adjustment = adjustments[lineIndex];
      line.MoveInBlockDirection(adjustment.Offset.ToFloat());
      if (adjustment.IsFirstAfterPageBreak)
      {
        line.SetIsFirstAfterPageBreak();
      }
    }
    for (auto &box : displayContent.BoxList)
    {
      auto offset = adjustments[box.LineIndex()].Offset.ToFloat();
      if (isHorizontalWritingMode)
      {
        box.MoveVertically(offset);
      }
      else
      {
        box.MoveHorizontally(offset);
      }
    }
  }
}
