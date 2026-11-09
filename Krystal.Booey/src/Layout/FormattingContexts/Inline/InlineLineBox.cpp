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

#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineBox.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontMetrics.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingUtils.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::layout
{
  LineBox::LineBox(const Box &rootLayoutBox, InlineLayoutUnit contentLogicalLeft,
                   InlineLayoutUnit contentLogicalWidth, size_t lineIndex, bool isFirstFormattedLine,
                   size_t nonSpanningInlineLevelBoxCount) noexcept
      : _lineIndex(lineIndex), _isFirstFormattedLine(isFirstFormattedLine),
        _rootInlineBox(InlineLevelBox::CreateRootInlineBox(
          rootLayoutBox, isFirstFormattedLine ? rootLayoutBox.FirstLineStyle() : rootLayoutBox.Style(),
          contentLogicalLeft, contentLogicalWidth))
  {
    _nonRootInlineLevelBoxList.reserve(nonSpanningInlineLevelBoxCount);
    _nonRootInlineLevelBoxMap.reserve(nonSpanningInlineLevelBoxCount);
    _rootInlineBox.SetTextEmphasis(
      InlineFormattingUtils::TextEmphasisForInlineBox(rootLayoutBox, Downcast<ElementBox>(rootLayoutBox)));
  }

  void LineBox::AddInlineLevelBox(InlineLevelBox &&inlineLevelBox) noexcept
  {
    _boxTypes.Add(inlineLevelBox.GetType());
    _nonRootInlineLevelBoxMap.insert_or_assign(&inlineLevelBox.LayoutBox(),
                                               _nonRootInlineLevelBoxList.size());
    _nonRootInlineLevelBoxList.push_back(krys::move(inlineLevelBox));
  }

  InlineRect LineBox::LogicalRectForTextRun(const Line::Run &run) const noexcept
  {
    krys_debug_assert(run.IsText() || run.IsSoftLineBreak());
    auto *ancestorInlineBox = &ParentInlineBox(run);
    krys_debug_assert(ancestorInlineBox->IsInlineBox());
    auto runlogicalTop =
      ancestorInlineBox->LogicalTop() - ancestorInlineBox->InlineBoxContentOffsetForTextBoxTrim();
    auto &metricsOfPrimaryFont = ancestorInlineBox->PrimarymetricsOfPrimaryFont();
    auto logicalHeight =
      InlineFormattingUtils::SnapToInt(metricsOfPrimaryFont.Ascent(), *ancestorInlineBox)
      + InlineFormattingUtils::SnapToInt(metricsOfPrimaryFont.Descent(), *ancestorInlineBox);

    while (ancestorInlineBox != &_rootInlineBox && !ancestorInlineBox->HasLineBoxRelativeAlignment())
    {
      ancestorInlineBox = &ParentInlineBox(*ancestorInlineBox);
      krys_debug_assert(ancestorInlineBox->IsInlineBox());
      runlogicalTop += ancestorInlineBox->LogicalTop();
    }
    return {runlogicalTop, _rootInlineBox.LogicalLeft() + run.LogicalLeft(), run.LogicalWidth(),
            logicalHeight};
  }

  InlineRect LineBox::LogicalRectForLineBreakBox(const Box &layoutBox) const noexcept
  {
    krys_debug_assert(layoutBox.IsLineBreakBox());
    return LogicalRectForInlineLevelBox(layoutBox);
  }

  InlineLayoutUnit LineBox::InlineLevelBoxAbsoluteTop(const InlineLevelBox &inlineLevelBox) const noexcept
  {
    // Inline level boxes are relative to their parent unless the vertical alignment makes them relative to
    // the line box (e.g. top, bottom).
    auto top = inlineLevelBox.LogicalTop();
    if (inlineLevelBox.IsRootInlineBox() || inlineLevelBox.HasLineBoxRelativeAlignment())
    {
      return top;
    }

    // Fast path for inline level boxes on the root inline box (e.g <div><img></div>).
    if (&inlineLevelBox.LayoutBox().Parent() == &_rootInlineBox.LayoutBox())
    {
      return top + _rootInlineBox.LogicalTop();
    }

    // Nested inline content e.g <div><span><img></span></div>
    auto *ancestorInlineBox = &inlineLevelBox;
    while (ancestorInlineBox != &_rootInlineBox && !ancestorInlineBox->HasLineBoxRelativeAlignment())
    {
      ancestorInlineBox = &ParentInlineBox(*ancestorInlineBox);
      krys_debug_assert(ancestorInlineBox->IsInlineBox());
      top += ancestorInlineBox->LogicalTop();
    }
    return top;
  }

  InlineRect LineBox::LogicalRectForInlineLevelBox(const Box &layoutBox) const noexcept
  {
    // FIXME: Blocks in inline. Refactor and remove the block test.
    krys_debug_assert(layoutBox.IsInlineLevelBox() || layoutBox.IsLineBreakBox()
                      || (layoutBox.IsBlockLevelBox() && layoutBox.IsInFlow()));
    auto *inlineBox = InlineLevelBoxFor(layoutBox);
    if (!inlineBox)
    {
      krys_debug_assert(false);
      return {};
    }
    auto &inlineBoxLogicalRect = inlineBox->LogicalRect();
    return InlineRect {InlineLevelBoxAbsoluteTop(*inlineBox), inlineBoxLogicalRect.Left(),
                       inlineBoxLogicalRect.Width(), inlineBoxLogicalRect.Height()};
  }

  InlineRect LineBox::LogicalBorderBoxForAtomicInlineBox(const Box &layoutBox,
                                                         const BoxGeometry &boxGeometry) const noexcept
  {
    // FIXME: Blocks in inline. Refactor and remove the block test.
    krys_debug_assert(layoutBox.IsAtomicInlineBox() || (layoutBox.IsBlockLevelBox() && layoutBox.IsInFlow()));
    auto logicalRect = LogicalRectForInlineLevelBox(layoutBox);
    // Inline level boxes use their margin box for vertical alignment. Let's covert them to border boxes.
    logicalRect.MoveVertically(boxGeometry.MarginBefore().ToFloat());
    auto verticalMargin = (boxGeometry.MarginBefore() + boxGeometry.MarginAfter()).ToFloat();
    logicalRect.ExpandVertically(-verticalMargin);

    return logicalRect;
  }

  InlineRect LineBox::LogicalBorderBoxForInlineBox(const Box &layoutBox,
                                                   const BoxGeometry &boxGeometry) const noexcept
  {
    auto logicalRect = LogicalRectForInlineLevelBox(layoutBox);
    // This logical rect is as tall as the "text" content is. Let's adjust with vertical border and padding.
    logicalRect.ExpandVertically(boxGeometry.VerticalBorderAndPadding().ToFloat());
    logicalRect.MoveVertically(-boxGeometry.BorderAndPaddingBefore().ToFloat());
    return logicalRect;
  }

  InlineRect LineBox::LogicalContentBoxForInlineBox(const Box &layoutBox) const noexcept
  {
    return LogicalRectForInlineLevelBox(layoutBox);
  }
}
