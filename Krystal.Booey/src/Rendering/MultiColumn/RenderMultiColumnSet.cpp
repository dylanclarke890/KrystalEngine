/*
 * Copyright (C) 2012-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2015 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/MultiColumn/RenderMultiColumnSet.hpp"
#include "Krystal.Booey/DOM/Ranges/Position.hpp"
#include "Krystal.Booey/Graphics/Geometry/IntRect.hpp"
#include "Krystal.Booey/Rendering/MultiColumn/RenderMultiColumnSpannerPlaceholder.hpp"
#include "Krystal.Booey/Rendering/Painting/BorderPainter.hpp"
#include "Krystal.Booey/Rendering/Painting/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <cmath>

namespace krys::boo
{
  LayoutUnit RenderMultiColumnSet::ContentRun::ColumnLogicalHeight(LayoutUnit startOffset) const noexcept
  {
    return LayoutUnit(std::ceil(float(_breakOffset - startOffset) / float(_assumedImplicitBreaks + 1)));
  }

  RenderMultiColumnSet::RenderMultiColumnSet(RenderFragmentedFlow &fragmentedFlow,
                                             RenderStyle &&style) noexcept
      : RenderFragmentContainerSet(Type::MultiColumnSet, fragmentedFlow.GetDocument(), krys::move(style),
                                   fragmentedFlow),
        _maxColumnHeight(RenderFragmentedFlow::MaxLogicalHeight()),
        _minSpaceShortage(RenderFragmentedFlow::MaxLogicalHeight())
  {
    krys_debug_assert(IsRenderMultiColumnSet());
  }

  RenderMultiColumnSet::~RenderMultiColumnSet() noexcept = default;

  RenderBlockFlow *RenderMultiColumnSet::MultiColumnBlockFlow() const noexcept
  {
    return Downcast<RenderBlockFlow>(Parent());
  }

  RenderMultiColumnFlow *RenderMultiColumnSet::MultiColumnFlow() const noexcept
  {
    return static_cast<RenderMultiColumnFlow *>(FragmentedFlow());
  }

  RenderMultiColumnSet *RenderMultiColumnSet::NextSiblingMultiColumnSet() const noexcept
  {
    for (auto *sibling = NextSibling(); sibling; sibling = sibling->NextSibling())
    {
      if (auto *multiColumnSet = DynamicDowncast<RenderMultiColumnSet>(*sibling))
      {
        return multiColumnSet;
      }
    }
    return nullptr;
  }

  RenderMultiColumnSet *RenderMultiColumnSet::PreviousSiblingMultiColumnSet() const noexcept
  {
    for (auto *sibling = PreviousSibling(); sibling; sibling = sibling->PreviousSibling())
    {
      if (auto *multiColumnSet = DynamicDowncast<RenderMultiColumnSet>(*sibling))
      {
        return multiColumnSet;
      }
    }
    return nullptr;
  }

  RenderObject *RenderMultiColumnSet::FirstRendererInFragmentedFlow() const noexcept
  {
    if (auto *sibling = RenderMultiColumnFlow::PreviousColumnSetOrSpannerSiblingOf(this))
    {
      // Adjacent sets should not occur. Currently we would have no way of figuring out what each
      // of them contains then.
      krys_debug_assert(!sibling->IsRenderMultiColumnSet());
      if (auto *placeholder = MultiColumnFlow()->FindColumnSpannerPlaceholder(*sibling))
      {
        return placeholder->NextInPreOrderAfterChildren();
      }
      krys_debug_assert_msg(false, "A spanner beside a column set has a placeholder.");
    }
    return FragmentedFlow()->FirstChild();
  }

  RenderObject *RenderMultiColumnSet::LastRendererInFragmentedFlow() const noexcept
  {
    if (auto *sibling = RenderMultiColumnFlow::NextColumnSetOrSpannerSiblingOf(this))
    {
      // Adjacent sets should not occur. Currently we would have no way of figuring out what each
      // of them contains then.
      krys_debug_assert(!sibling->IsRenderMultiColumnSet());
      if (auto *placeholder = MultiColumnFlow()->FindColumnSpannerPlaceholder(*sibling))
      {
        return placeholder->PreviousInPreOrder();
      }
      krys_debug_assert_msg(false, "A spanner beside a column set has a placeholder.");
    }
    return FragmentedFlow()->LastLeafChild();
  }

  static bool PrecedesRendererInPreOrder(const RenderObject *renderer, const RenderObject *boundary) noexcept
  {
    for (; renderer; renderer = renderer->NextInPreOrder())
    {
      if (renderer == boundary)
      {
        return true;
      }
    }
    return false;
  }

  bool RenderMultiColumnSet::ContainsRendererInFragmentedFlow(const RenderObject &renderer) const noexcept
  {
    if (!PreviousSiblingMultiColumnSet() && !NextSiblingMultiColumnSet())
    {
      // There is only one set. This is easy, then.
      return renderer.IsDescendantOf(FragmentedFlow());
    }

    auto *firstRenderer = FirstRendererInFragmentedFlow();
    auto *lastRenderer = LastRendererInFragmentedFlow();
    krys_debug_assert(firstRenderer);
    krys_debug_assert(lastRenderer);

    // This is SLOW! But luckily very uncommon.
    return PrecedesRendererInPreOrder(firstRenderer, &renderer)
           && PrecedesRendererInPreOrder(&renderer, lastRenderer);
  }

  void RenderMultiColumnSet::SetLogicalTopInFragmentedFlow(LayoutUnit logicalTop) noexcept
  {
    LayoutRect rect = FragmentedFlowPortionRect();
    if (IsHorizontalWritingMode())
    {
      rect.SetY(logicalTop);
    }
    else
    {
      rect.SetX(logicalTop);
    }
    SetFragmentedFlowPortionRect(rect);
  }

  void RenderMultiColumnSet::SetLogicalBottomInFragmentedFlow(LayoutUnit logicalBottom) noexcept
  {
    LayoutRect rect = FragmentedFlowPortionRect();
    if (IsHorizontalWritingMode())
    {
      rect.ShiftMaxYEdgeTo(logicalBottom);
    }
    else
    {
      rect.ShiftMaxXEdgeTo(logicalBottom);
    }
    SetFragmentedFlowPortionRect(rect);
  }

  LayoutUnit RenderMultiColumnSet::HeightAdjustedForSetOffset(LayoutUnit height) const noexcept
  {
    const auto &multicolBlock = Downcast<RenderBlockFlow>(*Parent());
    LayoutUnit contentLogicalTop = LogicalTop() - multicolBlock.BorderAndPaddingBefore();

    height -= contentLogicalTop;
    // Let's avoid zero height, as that would probably cause an infinite amount of columns to be created.
    return std::max(height, 1_lu);
  }

  LayoutUnit RenderMultiColumnSet::PageLogicalTopForOffset(LayoutUnit offset) const noexcept
  {
    uint32 columnIndex = ColumnIndexAtOffset(offset, ColumnIndexCalculationMode::AssumeNewColumns);
    return LogicalTopInFragmentedFlow() + columnIndex * ComputedColumnHeight();
  }

  void RenderMultiColumnSet::SetAndConstrainColumnHeight(LayoutUnit newHeight) noexcept
  {
    _computedColumnHeight = std::min(newHeight, _maxColumnHeight);
    _availableColumnHeight = _computedColumnHeight;
    _columnHeightComputed = true;

    // FIXME: the height may also be affected by the enclosing pagination context, if any.
  }

  uint32 RenderMultiColumnSet::FindRunWithTallestColumns() const noexcept
  {
    krys_debug_assert(!_contentRuns.empty());
    uint32 indexWithLargestHeight = 0;
    LayoutUnit largestHeight;
    LayoutUnit previousOffset;
    for (uint32 i = 0; i < _contentRuns.size(); ++i)
    {
      const auto &run = _contentRuns[i];
      LayoutUnit height = run.ColumnLogicalHeight(previousOffset);
      if (largestHeight < height)
      {
        largestHeight = height;
        indexWithLargestHeight = i;
      }
      previousOffset = run.BreakOffset();
    }
    return indexWithLargestHeight;
  }

  void RenderMultiColumnSet::DistributeImplicitBreaks() noexcept
  {
#if KRYS_DEBUG
    // There should be no implicit breaks assumed at this point.
    for (const auto &run : _contentRuns)
    {
      krys_debug_assert(!run.AssumedImplicitBreaks());
    }
#endif

    // Insert a final content run to encompass all content. This will include overflow if this is
    // the last set.
    AddForcedBreak(LogicalBottomInFragmentedFlow());
    uint32 breakCount = ForcedBreaksCount();

    // If there is room for more breaks (to reach the used value of column-count), imagine that we
    // insert implicit breaks at suitable locations. At any given time, the content run with the
    // currently tallest columns will get another implicit break "inserted", which will increase its
    // column count by one and shrink its columns' height. Repeat until we have the desired total
    // number of breaks. The largest column height among the runs will then be the initial column
    // height for the balancer to use.
    while (breakCount < _computedColumnCount)
    {
      _contentRuns[FindRunWithTallestColumns()].AssumeAnotherImplicitBreak();
      ++breakCount;
    }
  }

  LayoutUnit RenderMultiColumnSet::CalculateBalancedHeight(bool initial) const noexcept
  {
    if (initial)
    {
      // Start with the lowest imaginable column height.
      uint32 index = FindRunWithTallestColumns();
      LayoutUnit startOffset =
        index > 0 ? _contentRuns[index - 1].BreakOffset() : LogicalTopInFragmentedFlow();
      return std::max<LayoutUnit>(_contentRuns[index].ColumnLogicalHeight(startOffset), _minimumColumnHeight);
    }

    LayoutUnit sizeContainmentShortage = std::max<LayoutUnit>(0_lu, _spaceShortageForSizeContainment);
    if (ColumnCount() <= ComputedColumnCount())
    {
      // With the current column height, the content fits without creating overflowing columns. We're done.
      return _computedColumnHeight + sizeContainmentShortage;
    }

    if (ForcedBreaksCount() >= ComputedColumnCount())
    {
      // Too many forced breaks to allow any implicit breaks. Initial balancing should already
      // have set a good height. There's nothing more we should do.
      return _computedColumnHeight + sizeContainmentShortage;
    }

    if (_computedColumnHeight >= _maxColumnHeight)
    {
      // We cannot stretch any further. We'll just have to live with the overflowing columns. This
      // typically happens if the max column height is less than the height of the tallest piece
      // of unbreakable content (e.g. lines).
      return _computedColumnHeight;
    }

    // If the initial guessed column height wasn't enough, stretch it now. Stretch by the lowest
    // amount of space shortage found during layout.

    krys_debug_assert(_minSpaceShortage > 0); // We should never _shrink_ the height!
    if (_minSpaceShortage == RenderFragmentedFlow::MaxLogicalHeight())
    {
      return _computedColumnHeight + sizeContainmentShortage; // So bail out rather than looping infinitely.
    }

    return _computedColumnHeight + std::max<LayoutUnit>(sizeContainmentShortage, _minSpaceShortage);
  }

  void RenderMultiColumnSet::ClearForcedBreaks() noexcept
  {
    _contentRuns.clear();
  }

  void RenderMultiColumnSet::AddForcedBreak(LayoutUnit offsetFromFirstPage) noexcept
  {
    if (!RequiresBalancing())
    {
      return;
    }
    if (!_contentRuns.empty() && offsetFromFirstPage <= _contentRuns.back().BreakOffset())
    {
      return;
    }
    // Append another item as long as we haven't exceeded used column count. What ends up in the
    // overflow area shouldn't affect column balancing.
    if (_contentRuns.size() < _computedColumnCount)
    {
      _contentRuns.emplace_back(offsetFromFirstPage);
    }
  }

  bool RenderMultiColumnSet::RecalculateColumnHeight(bool initial) noexcept
  {
    LayoutUnit oldColumnHeight = _computedColumnHeight;
    if (RequiresBalancing())
    {
      if (initial)
      {
        DistributeImplicitBreaks();
      }
      // After having calculated an initial column height, the multicol container typically needs at
      // least one more layout pass with a new column height, but if a height was specified, we only
      // need to do this if we think that we need less space than specified. Conversely, if we
      // determined that the columns need to be as tall as the specified height of the container, we
      // have already laid it out correctly, and there's no need for another pass.
      SetAndConstrainColumnHeight(CalculateBalancedHeight(initial));
    }
    else
    {
      // The position of the column set may have changed, in which case height available for
      // columns may have changed as well.
      SetAndConstrainColumnHeight(_computedColumnHeight);
    }
    if (_computedColumnHeight == oldColumnHeight)
    {
      return false; // No change. We're done.
    }

    _minSpaceShortage = RenderFragmentedFlow::MaxLogicalHeight();
    return true; // Need another pass.
  }

  void RenderMultiColumnSet::RecordSpaceShortage(LayoutUnit spaceShortage) noexcept
  {
    if (spaceShortage >= _minSpaceShortage)
    {
      return;
    }

    // The space shortage is what we use as our stretch amount. We need a positive number here in
    // order to get anywhere. Some lines actually have zero height. Ignore them.
    if (spaceShortage > 0)
    {
      _minSpaceShortage = spaceShortage;
    }
  }

  void RenderMultiColumnSet::UpdateLogicalWidth() noexcept
  {
    // FIXME: This will eventually vary if we are contained inside fragments.
    SetComputedColumnWidthAndCount(MultiColumnFlow()->ColumnWidth(), MultiColumnFlow()->ColumnCount());

    // FIXME: When we add fragments support, we'll start it off at the width of the multi-column
    // block in that particular fragment.
    SetLogicalWidth(MultiColumnBlockFlow()->ContentBoxLogicalWidth());
  }

  bool RenderMultiColumnSet::RequiresBalancing() const noexcept
  {
    if (auto *next = RenderMultiColumnFlow::NextColumnSetOrSpannerSiblingOf(this))
    {
      if (!next->IsRenderMultiColumnSet() && !next->IsLegend())
      {
        // If we're followed by a spanner, we need to balance.
        krys_debug_assert(MultiColumnFlow()->FindColumnSpannerPlaceholder(*next));
        return true;
      }
    }
    if (MultiColumnBlockFlow()->Style().GetColumnFill() == ColumnFill::Balance)
    {
      return true;
    }
    return !MultiColumnFlow()->ColumnHeightAvailable();
  }

  void RenderMultiColumnSet::PrepareForLayout(bool initial) noexcept
  {
    // Guess box logical top. This might eliminate the need for another layout pass.
    if (auto *previous = RenderMultiColumnFlow::PreviousColumnSetOrSpannerSiblingOf(this))
    {
      SetLogicalTop(previous->LogicalBottom() + previous->MarginAfter());
    }
    else
    {
      SetLogicalTop(MultiColumnBlockFlow()->BorderAndPaddingBefore());
    }

    if (initial)
    {
      _maxColumnHeight = CalculateMaxColumnHeight();
    }
    if (RequiresBalancing())
    {
      if (initial)
      {
        _computedColumnHeight = 0_lu;
        _availableColumnHeight = 0_lu;
        _columnHeightComputed = false;
      }
    }
    else
    {
      SetAndConstrainColumnHeight(HeightAdjustedForSetOffset(MultiColumnFlow()->ColumnHeightAvailable()));
    }

    // Set box width.
    UpdateLogicalWidth();

    // Any breaks will be re-inserted during layout, so get rid of what we already have.
    ClearForcedBreaks();

    // Nuke previously stored minimum column height. Contents may have changed for all we know.
    _minimumColumnHeight = 0_lu;

    _spaceShortageForSizeContainment = 0_lu;

    // Start with "infinite" flow thread portion height until height is known.
    SetLogicalBottomInFragmentedFlow(RenderFragmentedFlow::MaxLogicalHeight());

    SetNeedsLayout(MarkingBehavior::MarkOnlyThis);
  }

  void RenderMultiColumnSet::BeginFlow(RenderBlock *container) noexcept
  {
    // At this point layout is exactly at the beginning of this set. Store block offset from flow
    // thread start.
    SetLogicalTopInFragmentedFlow(MultiColumnFlow()->OffsetFromLogicalTopOfFirstFragment(container)
                                  + container->LogicalHeight());
  }

  void RenderMultiColumnSet::EndFlow(RenderBlock *container, LayoutUnit bottomInContainer) noexcept
  {
    // At this point layout is exactly at the end of this set. Store block offset from flow thread
    // start. Also note that a new column height may have affected the height used in the flow
    // thread (because of struts), which may affect the number of columns. So we also need to update
    // the flow thread portion height in order to be able to calculate actual column-count.
    SetLogicalBottomInFragmentedFlow(MultiColumnFlow()->OffsetFromLogicalTopOfFirstFragment(container)
                                     + bottomInContainer);
    container->SetLogicalHeight(bottomInContainer);
  }

  void RenderMultiColumnSet::Layout() noexcept
  {
    RenderBlockFlow::Layout();

    // At this point the logical top and bottom of the column set are known. Update maximum column
    // height (multicol height may be constrained).
    _maxColumnHeight = CalculateMaxColumnHeight();

    if (!NextSiblingMultiColumnSet())
    {
      // This is the last set, i.e. the last fragment. Seize the opportunity to validate them.
      MultiColumnFlow()->ValidateFragments();
    }
  }

  RenderBox::LogicalExtentComputedValues
    RenderMultiColumnSet::ComputeLogicalHeight(LayoutUnit, LayoutUnit logicalTop) const noexcept
  {
    return {_availableColumnHeight, logicalTop, ComputedMarginValues()};
  }

  LayoutUnit RenderMultiColumnSet::CalculateMaxColumnHeight() const noexcept
  {
    auto *multicolBlock = MultiColumnBlockFlow();
    const auto &multicolStyle = multicolBlock->Style();
    LayoutUnit availableHeight = MultiColumnFlow()->ColumnHeightAvailable();
    LayoutUnit maxColumnHeight = availableHeight ? availableHeight : RenderFragmentedFlow::MaxLogicalHeight();
    if (!multicolStyle.LogicalMaxHeight().IsNone())
    {
      maxColumnHeight = std::min(
        maxColumnHeight, multicolBlock->ComputeContentLogicalHeight(multicolStyle.LogicalMaxHeight(), null)
                           .value_or(maxColumnHeight));
    }
    return HeightAdjustedForSetOffset(maxColumnHeight);
  }

  LayoutUnit RenderMultiColumnSet::ColumnGap() const noexcept
  {
    // FIXME: Eventually we will cache the column gap when the widths of columns start varying, but for now
    // we just go to the parent block to get the gap.
    const auto &parentBlock = Downcast<RenderBlockFlow>(*Parent());
    const auto &parentBlockGap = parentBlock.Style().ColumnGap();
    if (parentBlockGap.IsNormal())
    {
      // "1em" is recommended as the normal gap setting. Matches <p> margins.
      return LayoutUnit(parentBlock.Style().GetFontDescription().ComputedSize());
    }
    return style::Evaluate<LayoutUnit>(parentBlockGap, parentBlock.ContentBoxLogicalWidth(),
                                       style::ZoomNeeded {});
  }

  uint32 RenderMultiColumnSet::ColumnCount() const noexcept
  {
    // We must always return a value of 1 or greater. Column count = 0 is a meaningless situation,
    // and will confuse and cause problems in other parts of the code.
    auto computedColumnHeight = ComputedColumnHeight();
    if (computedColumnHeight <= 0)
    {
      return 1;
    }

    // Our portion rect determines our column count. We have as many columns as needed to fit all the content.
    auto logicalHeightInColumns = FragmentedFlow()->IsHorizontalWritingMode()
                                    ? FragmentedFlowPortionRect().Height()
                                    : FragmentedFlowPortionRect().Width();
    if (logicalHeightInColumns <= 0)
    {
      return 1;
    }

    auto count = static_cast<uint32>((logicalHeightInColumns / computedColumnHeight).Floor());
    // logicalHeightInColumns may be saturated, so detect the remainder manually.
    if (count * computedColumnHeight < logicalHeightInColumns)
    {
      ++count;
    }
    krys_debug_assert(count >= 1);
    return count;
  }

  LayoutUnit RenderMultiColumnSet::ColumnLogicalLeft(uint32 index) const noexcept
  {
    LayoutUnit columnLogicalWidth = ComputedColumnWidth();
    LayoutUnit columnLogicalLeft = BorderAndPaddingLogicalLeft();
    LayoutUnit columnGap = ColumnGap();

    if (GetWritingMode().IsLogicalLeftInlineStart())
    {
      return columnLogicalLeft + index * (columnLogicalWidth + columnGap);
    }
    return columnLogicalLeft + ContentBoxLogicalWidth() - columnLogicalWidth
           - index * (columnLogicalWidth + columnGap);
  }

  LayoutRect RenderMultiColumnSet::ColumnRectAt(uint32 index) const noexcept
  {
    LayoutUnit columnLogicalWidth = ComputedColumnWidth();
    LayoutUnit columnLogicalHeight = ComputedColumnHeight();
    LayoutUnit columnLogicalTop = BorderAndPaddingBefore();

    if (IsHorizontalWritingMode())
    {
      return LayoutRect(ColumnLogicalLeft(index), columnLogicalTop, columnLogicalWidth, columnLogicalHeight);
    }
    return LayoutRect(columnLogicalTop, ColumnLogicalLeft(index), columnLogicalHeight, columnLogicalWidth);
  }

  uint32 RenderMultiColumnSet::ColumnIndexAtOffset(LayoutUnit offset,
                                                   ColumnIndexCalculationMode mode) const noexcept
  {
    LayoutRect portionRect(FragmentedFlowPortionRect());

    // Handle the offset being out of range.
    LayoutUnit fragmentedFlowLogicalTop = IsHorizontalWritingMode() ? portionRect.Y() : portionRect.X();
    if (offset < fragmentedFlowLogicalTop)
    {
      return 0;
    }
    // If we're laying out right now, we cannot constrain against some logical bottom, since it
    // isn't known yet. Otherwise, just return the last column if we're past the logical bottom.
    if (mode == ColumnIndexCalculationMode::ClampToExistingColumns)
    {
      LayoutUnit fragmentedFlowLogicalBottom =
        IsHorizontalWritingMode() ? portionRect.MaxY() : portionRect.MaxX();
      if (offset >= fragmentedFlowLogicalBottom)
      {
        return ColumnCount() - 1;
      }
    }

    // Sometimes ComputedColumnHeight() is 0 here: see https://bugs.webkit.org/show_bug.cgi?id=132884
    if (!ComputedColumnHeight())
    {
      return 0;
    }

    // Just divide by the column height to determine the correct column.
    return static_cast<uint32>(static_cast<float>(offset - fragmentedFlowLogicalTop)
                               / ComputedColumnHeight());
  }

  Pair<uint32, uint32>
    RenderMultiColumnSet::FirstAndLastColumnsFromOffsets(LayoutUnit topOffset,
                                                         LayoutUnit bottomOffset) const noexcept
  {
    auto portionRect = FragmentedFlowPortionRect();

    // Handle the offset being out of range.
    auto fragmentedFlowLogicalTop = IsHorizontalWritingMode() ? portionRect.Y() : portionRect.X();
    auto fragmentedFlowLogicalBottom = IsHorizontalWritingMode() ? portionRect.MaxY() : portionRect.MaxX();

    auto computeColumnIndex = [&](LayoutUnit offset, bool isBottom) -> uint32
    {
      if (offset < fragmentedFlowLogicalTop)
      {
        return 0;
      }

      if (offset >= fragmentedFlowLogicalBottom)
      {
        return ColumnCount() - 1;
      }

      // Sometimes ComputedColumnHeight() is 0 here: see https://bugs.webkit.org/show_bug.cgi?id=132884
      auto columnHeight = ComputedColumnHeight();
      if (!columnHeight)
      {
        return 0;
      }

      auto columnIndex = static_cast<float>(offset - fragmentedFlowLogicalTop) / columnHeight;
      if (isBottom && std::trunc(columnIndex) == columnIndex && bottomOffset > topOffset && columnIndex > 0)
      {
        columnIndex -= 1;
      }

      return static_cast<uint32>(columnIndex);
    };

    return {computeColumnIndex(topOffset, false), computeColumnIndex(bottomOffset, true)};
  }

  LayoutRect RenderMultiColumnSet::FragmentedFlowPortionRectAt(uint32 index) const noexcept
  {
    LayoutRect portionRect = FragmentedFlowPortionRect();
    if (IsHorizontalWritingMode())
    {
      return LayoutRect(portionRect.X(), portionRect.Y() + index * ComputedColumnHeight(),
                        portionRect.Width(), ComputedColumnHeight());
    }
    return LayoutRect(portionRect.X() + index * ComputedColumnHeight(), portionRect.Y(),
                      ComputedColumnHeight(), portionRect.Height());
  }

  LayoutRect RenderMultiColumnSet::FragmentedFlowPortionOverflowRect(const LayoutRect &portionRect,
                                                                     uint32 index,
                                                                     uint32 columnCount) const noexcept
  {
    // This function determines the portion of the flow thread that paints for the column. Along the inline
    // axis, columns are unclipped, so content overflowing a column can be seen in the gaps and next columns.
    //
    // In the block direction, we will not clip overflow out of the top of the first column, or out of the
    // bottom of the last column. This applies only to the true first column and last column across all column
    // sets.
    //
    // FIXME: Eventually we will know overflow on a per-column basis, but we can't do this until we have a
    // painting mode that understands not to paint contents from a previous column in the overflow area of a
    // following column. This problem applies to fragments and pages as well and is not unique to columns.
    bool isFirstColumn = !index;
    bool isLastColumn = index == columnCount - 1;

    // Calculate the overflow rectangle, based on the flow thread's, clipped at column logical
    // top/bottom unless it's the first/last column.
    return OverflowRectForFragmentedFlowPortion(portionRect, isFirstColumn && IsFirstFragment(),
                                                isLastColumn && IsLastFragment());
  }

  void RenderMultiColumnSet::PaintColumnRules(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    if (paintInfo.Context().PaintingDisabled())
    {
      return;
    }

    const auto &blockStyle = Parent()->Style();
    auto ruleColor = blockStyle.ColorResolvingCurrentColor(css::PropertyId::ColumnRuleColor);
    bool ruleTransparent = blockStyle.ColumnRuleIsTransparent();
    auto ruleStyle = CollapsedBorderStyle(blockStyle.ColumnRuleStyle());
    auto ruleThickness = style::Evaluate<LayoutUnit>(blockStyle.ColumnRuleWidth(), style::ZoomNeeded {});
    auto columnGap = ColumnGap();
    bool renderRule = ruleStyle > BorderStyle::Hidden && !ruleTransparent;
    if (!renderRule)
    {
      return;
    }

    uint32 columnCount = ColumnCount();
    if (columnCount <= 1)
    {
      return;
    }

    bool antialias = BorderPainter::ShouldAntialiasLines(paintInfo.Context());

    bool isLogicalLeftInlineStart = GetWritingMode().IsLogicalLeftInlineStart();
    LayoutUnit currentLogicalLeftOffset = isLogicalLeftInlineStart ? 0_lu : ContentBoxLogicalWidth();
    LayoutUnit ruleAdd = LogicalLeftOffsetForContent();
    LayoutUnit ruleLogicalLeft = isLogicalLeftInlineStart ? 0_lu : ContentBoxLogicalWidth();
    LayoutUnit inlineDirectionSize = ComputedColumnWidth();
    BoxSide boxSide = IsHorizontalWritingMode()  ? isLogicalLeftInlineStart ? BoxSide::Left : BoxSide::Right
                      : isLogicalLeftInlineStart ? BoxSide::Top
                                                 : BoxSide::Bottom;

    for (uint32 i = 0; i < columnCount; ++i)
    {
      // Move to the next position.
      if (isLogicalLeftInlineStart)
      {
        ruleLogicalLeft += inlineDirectionSize + columnGap / 2;
        currentLogicalLeftOffset += inlineDirectionSize + columnGap;
      }
      else
      {
        ruleLogicalLeft -= (inlineDirectionSize + columnGap / 2);
        currentLogicalLeftOffset -= (inlineDirectionSize + columnGap);
      }

      // Now paint the column rule.
      if (i < columnCount - 1)
      {
        LayoutUnit ruleLeft = IsHorizontalWritingMode()
                                ? paintOffset.X() + ruleLogicalLeft - ruleThickness / 2 + ruleAdd
                                : paintOffset.X() + BorderLeft() + PaddingLeft();
        LayoutUnit ruleRight =
          IsHorizontalWritingMode() ? ruleLeft + ruleThickness : ruleLeft + ContentBoxWidth();
        LayoutUnit ruleTop = IsHorizontalWritingMode()
                               ? paintOffset.Y() + BorderTop() + PaddingTop()
                               : paintOffset.Y() + ruleLogicalLeft - ruleThickness / 2 + ruleAdd;
        LayoutUnit ruleBottom =
          IsHorizontalWritingMode() ? ruleTop + ContentBoxHeight() : ruleTop + ruleThickness;
        IntRect pixelSnappedRuleRect =
          SnappedIntRect(ruleLeft, ruleTop, ruleRight - ruleLeft, ruleBottom - ruleTop);
        BorderPainter::DrawLineForBoxSide(paintInfo.Context(), GetDocument(), FloatRect(pixelSnappedRuleRect),
                                          boxSide, ruleColor, ruleStyle, 0, 0, antialias);
      }

      ruleLogicalLeft = currentLogicalLeftOffset;
    }
  }

  void RenderMultiColumnSet::RepaintFragmentedFlowContent(const LayoutRect &repaintRect) const noexcept
  {
    // Figure out the start and end columns and only check within that range so that we don't walk the
    // entire column set. Put the repaint rect into flow thread coordinates by flipping it first.
    LayoutRect fragmentedFlowRepaintRect(repaintRect);
    FragmentedFlow()->FlipForWritingMode(fragmentedFlowRepaintRect);

    // Now we can compare this rect with the flow thread portions owned by each column. First let's
    // just see if the repaint rect intersects our flow thread portion at all.
    LayoutRect clippedRect(fragmentedFlowRepaintRect);
    clippedRect.Intersect(RenderFragmentContainer::FragmentedFlowPortionOverflowRect());
    if (clippedRect.IsEmpty())
    {
      return;
    }

    // Now we know we intersect at least one column. Let's figure out the logical top and logical
    // bottom of the area we're repainting.
    LayoutUnit repaintLogicalTop =
      IsHorizontalWritingMode() ? fragmentedFlowRepaintRect.Y() : fragmentedFlowRepaintRect.X();
    LayoutUnit repaintLogicalBottom =
      (IsHorizontalWritingMode() ? fragmentedFlowRepaintRect.MaxY() : fragmentedFlowRepaintRect.MaxX()) - 1;

    // FIXME: this should use FirstAndLastColumnsFromOffsets.
    uint32 startColumn = ColumnIndexAtOffset(repaintLogicalTop);
    uint32 endColumn = ColumnIndexAtOffset(repaintLogicalBottom);

    uint32 columnCount = ColumnCount();
    for (uint32 i = startColumn; i <= endColumn; ++i)
    {
      LayoutRect columnRect = ColumnRectAt(i);

      // Get the portion of the flow thread that corresponds to this column.
      LayoutRect fragmentedFlowPortion = FragmentedFlowPortionRectAt(i);

      // Now get the overflow rect that corresponds to the column.
      LayoutRect fragmentedFlowOverflowPortion =
        FragmentedFlowPortionOverflowRect(fragmentedFlowPortion, i, columnCount);

      // Do a repaint for this specific column.
      FlipForWritingMode(columnRect);
      RepaintFragmentedFlowContentRectangle(repaintRect, fragmentedFlowPortion, columnRect.Location(),
                                            &fragmentedFlowOverflowPortion);
    }
  }

  List<LayoutRect>
    RenderMultiColumnSet::FragmentRectsForFlowContentRect(const LayoutRect &rect) const noexcept
  {
    auto fragmentedFlowRect = rect;
    FragmentedFlow()->FlipForWritingMode(fragmentedFlowRect);

    auto logicalTop = IsHorizontalWritingMode() ? fragmentedFlowRect.Y() : fragmentedFlowRect.X();
    auto logicalBottom = IsHorizontalWritingMode() ? fragmentedFlowRect.MaxY() : fragmentedFlowRect.MaxX();

    auto [startColumn, endColumn] = FirstAndLastColumnsFromOffsets(logicalTop, logicalBottom);

    List<LayoutRect> perColumnRects;

    uint32 columnCount = ColumnCount();
    for (uint32 i = startColumn; i <= endColumn; ++i)
    {
      auto columnRect = ColumnRectAt(i);
      FlipForWritingMode(columnRect);

      auto fragmentedFlowPortion = FragmentedFlowPortionRectAt(i);
      auto fragmentedFlowOverflowPortion =
        FragmentedFlowPortionOverflowRect(fragmentedFlowPortion, i, columnCount);

      auto rectInColumn = FragmentedFlowContentRectangle(rect, fragmentedFlowPortion, columnRect.Location(),
                                                         &fragmentedFlowOverflowPortion);
      FlipForWritingMode(rectInColumn);
      perColumnRects.push_back(rectInColumn);
    }

    return perColumnRects;
  }

  bool RenderMultiColumnSet::ContentRectSpansFragments(const LayoutRect &rect) const noexcept
  {
    auto fragmentedFlowRect = rect;
    FragmentedFlow()->FlipForWritingMode(fragmentedFlowRect);

    auto logicalTop = IsHorizontalWritingMode() ? fragmentedFlowRect.Y() : fragmentedFlowRect.X();
    auto logicalBottom = IsHorizontalWritingMode() ? fragmentedFlowRect.MaxY() : fragmentedFlowRect.MaxX();

    auto [startColumn, endColumn] = FirstAndLastColumnsFromOffsets(logicalTop, logicalBottom);
    return startColumn != endColumn;
  }

  void RenderMultiColumnSet::CollectLayerFragments(LayerFragments &fragments,
                                                   const LayoutRect &layerBoundingBox,
                                                   const LayoutRect &dirtyRect) const noexcept
  {
    static constexpr uint64 MaximumNumberOfFragments = 2'500'000;
    // Let's start by introducing the different coordinate systems involved here. They are different
    // in how they deal with writing modes and columns. RenderLayer rectangles tend to be more
    // physical than the rectangles used in RenderObject & co.
    //
    // The two rectangles passed to this method are physical, except that we pretend that there's
    // only one long column (that's the flow thread). They are relative to the top left corner of
    // the flow thread. All rectangles being compared to the dirty rect also need to be in this
    // coordinate system.
    //
    // Then there's the output from this method - the stuff we put into the list of fragments. The
    // translationOffset point is the actual physical translation required to get from a location in
    // the flow thread to a location in some column. The paginationClip rectangle is in the same
    // coordinate system as the two rectangles passed to this method (i.e. physical, in flow thread
    // coordinates, pretending that there's only one long column).
    //
    // All other rectangles in this method are slightly less physical, when it comes to how they are
    // used with different writing modes, but they aren't really logical either. They are just like
    // RenderBox::FrameRect(). More precisely, the sizes are physical, and the inline direction
    // coordinate is too, but the block direction coordinate is always "logical top". These
    // rectangles also pretend that there's only one long column, i.e. they are for the flow thread.
    //
    // To sum up: input and output from this method are "physical" RenderLayer-style rectangles and
    // points, while inside this method we mostly use the RenderObject-style rectangles (with the
    // block direction coordinate always being logical top).

    // Put the layer bounds into flow thread-local coordinates by flipping it first. Since we're in
    // a renderer, most rectangles are represented this way.
    LayoutRect layerBoundsInFragmentedFlow(layerBoundingBox);
    FragmentedFlow()->FlipForWritingMode(layerBoundsInFragmentedFlow);

    // Now we can compare with the flow thread portions owned by each column. First let's
    // see if the rect intersects our flow thread portion at all.
    LayoutRect clippedRect(layerBoundsInFragmentedFlow);
    clippedRect.Intersect(RenderFragmentContainer::FragmentedFlowPortionOverflowRect());
    if (clippedRect.IsEmpty())
    {
      return;
    }

    // Now we know we intersect at least one column. Let's figure out the logical top and logical
    // bottom of the area we're checking.
    LayoutUnit layerLogicalTop =
      IsHorizontalWritingMode() ? layerBoundsInFragmentedFlow.Y() : layerBoundsInFragmentedFlow.X();
    LayoutUnit layerLogicalBottom =
      (IsHorizontalWritingMode() ? layerBoundsInFragmentedFlow.MaxY() : layerBoundsInFragmentedFlow.MaxX())
      - 1;

    // Figure out the start and end columns and only check within that range so that we don't walk the
    // entire column set.
    // FIXME: this should use FirstAndLastColumnsFromOffsets.
    uint32 startColumn = ColumnIndexAtOffset(layerLogicalTop);
    uint32 endColumn = ColumnIndexAtOffset(layerLogicalBottom);

    LayoutUnit columnLogicalWidth = ComputedColumnWidth();
    LayoutUnit columnGap = ColumnGap();
    uint32 columnCount = ColumnCount();

    for (uint32 i = startColumn; i <= endColumn; ++i)
    {
      // Get the portion of the flow thread that corresponds to this column.
      LayoutRect fragmentedFlowPortion = FragmentedFlowPortionRectAt(i);

      // Now get the overflow rect that corresponds to the column.
      LayoutRect fragmentedFlowOverflowPortion =
        FragmentedFlowPortionOverflowRect(fragmentedFlowPortion, i, columnCount);

      // In order to create a fragment we must intersect the portion painted by this column.
      LayoutRect columnClippedRect(layerBoundsInFragmentedFlow);
      columnClippedRect.Intersect(fragmentedFlowOverflowPortion);
      if (columnClippedRect.IsEmpty())
      {
        continue;
      }

      // We also need to intersect the dirty rect. We have to apply a translation and shift based off
      // our column index.
      LayoutSize translationOffset;
      LayoutUnit inlineOffset = i * (columnLogicalWidth + columnGap);
      if (!GetWritingMode().IsLogicalLeftInlineStart())
      {
        inlineOffset = -inlineOffset;
      }
      translationOffset.SetWidth(inlineOffset);

      LayoutUnit blockOffset =
        LogicalTop() - FragmentedFlow()->LogicalTop()
        + (IsHorizontalWritingMode() ? -fragmentedFlowPortion.Y() : -fragmentedFlowPortion.X());
      if (Style().GetWritingMode().IsBlockFlipped())
      {
        blockOffset = -blockOffset;
      }
      translationOffset.SetHeight(blockOffset);
      if (!IsHorizontalWritingMode())
      {
        translationOffset = translationOffset.TransposedSize();
      }

      // Shift the dirty rect to be in flow thread coordinates with this translation applied.
      LayoutRect translatedDirtyRect(dirtyRect);
      translatedDirtyRect.Move(-translationOffset);

      // See if we intersect the dirty rect.
      columnClippedRect = layerBoundingBox;
      columnClippedRect.Intersect(translatedDirtyRect);
      if (columnClippedRect.IsEmpty())
      {
        continue;
      }

      // Something does need to paint in this column. Make a fragment now and supply the physical translation
      // offset and the clip rect for the column with that offset applied.
      LayerFragment fragment;
      fragment.PaginationOffset = translationOffset;

      LayoutRect flippedFragmentedFlowOverflowPortion(fragmentedFlowOverflowPortion);
      // Flip it into more a physical (RenderLayer-style) rectangle.
      FragmentedFlow()->FlipForWritingMode(flippedFragmentedFlowOverflowPortion);
      fragment.PaginationClip = flippedFragmentedFlowOverflowPortion;
      if (fragments.size() >= MaximumNumberOfFragments)
      {
        break;
      }
      fragments.push_back(krys::move(fragment));
    }
  }

  LayoutPoint RenderMultiColumnSet::ColumnTranslationForOffset(LayoutUnit offset) const noexcept
  {
    uint32 startColumn = ColumnIndexAtOffset(offset);

    LayoutRect fragmentedFlowPortion = FragmentedFlowPortionRectAt(startColumn);
    LayoutPoint translationOffset;

    translationOffset.SetX(ColumnLogicalLeft(startColumn));

    LayoutUnit blockOffset =
      IsHorizontalWritingMode() ? -fragmentedFlowPortion.Y() : -fragmentedFlowPortion.X();
    if (Style().GetWritingMode().IsBlockFlipped())
    {
      blockOffset = -blockOffset;
    }
    translationOffset.SetY(blockOffset);

    if (!IsHorizontalWritingMode())
    {
      translationOffset = translationOffset.TransposedPoint();
    }

    return translationOffset;
  }

  void RenderMultiColumnSet::AddOverflowFromInFlowChildren(EnumSet<ComputeOverflowOptions>) noexcept
  {
    // FIXME: Need to do much better here.
    uint32 columnCount = ColumnCount();
    if (!columnCount)
    {
      return;
    }

    LayoutRect lastRect = ColumnRectAt(columnCount - 1);
    AddLayoutOverflow(lastRect);
    if (!HasNonVisibleOverflow())
    {
      AddVisualOverflow(lastRect);
    }
  }

  dom::PositionWithAffinity RenderMultiColumnSet::PositionForPoint(const LayoutPoint &logicalPoint,
                                                                   HitTestSource source) noexcept
  {
    // FIXME: Pass the set, so a box split across columns answers for this one, once positions are
    // fragment-aware.
    return MultiColumnFlow()->PositionForPoint(
      TranslateFragmentPointToFragmentedFlow(logicalPoint,
                                             ColumnHitTestTranslationMode::ClampHitTestTranslationToColumns),
      source);
  }

  LayoutPoint RenderMultiColumnSet::TranslateFragmentPointToFragmentedFlow(
    const LayoutPoint &logicalPoint, ColumnHitTestTranslationMode clampMode) const noexcept
  {
    // Determine which columns we intersect.
    LayoutUnit columnGap = ColumnGap();
    LayoutUnit halfColumnGap = columnGap / 2;
    bool clamp = clampMode == ColumnHitTestTranslationMode::ClampHitTestTranslationToColumns;

    LayoutPoint point = logicalPoint;

    for (uint32 i = 0; i < ColumnCount(); ++i)
    {
      // Add in half the column gap to the left and right of the rect.
      LayoutRect columnRect = ColumnRectAt(i);
      if (IsHorizontalWritingMode())
      {
        LayoutRect gapAndColumnRect(columnRect.X() - halfColumnGap, columnRect.Y(),
                                    columnRect.Width() + columnGap, columnRect.Height());
        if (point.X() >= gapAndColumnRect.X() && point.X() < gapAndColumnRect.MaxX())
        {
          if (clamp)
          {
            // FIXME: The clamping that follows is not completely right for right-to-left content.
            // Clamp everything above the column to its top left.
            if (point.Y() < gapAndColumnRect.Y())
            {
              point = gapAndColumnRect.Location();
            }
            // Clamp everything below the column to the next column's top left. If there is
            // no next column, this still maps to just after this column.
            else if (point.Y() >= gapAndColumnRect.MaxY())
            {
              point = gapAndColumnRect.Location();
              point.Move(0_lu, gapAndColumnRect.Height());
            }
          }

          LayoutSize offsetInColumn = point - columnRect.Location();
          return FragmentedFlowPortionRectAt(i).Location() + offsetInColumn;
        }
      }
      else
      {
        LayoutRect gapAndColumnRect(columnRect.X(), columnRect.Y() - halfColumnGap, columnRect.Width(),
                                    columnRect.Height() + columnGap);
        if (point.Y() >= gapAndColumnRect.Y() && point.Y() < gapAndColumnRect.MaxY())
        {
          if (clamp)
          {
            // FIXME: The clamping that follows is not completely right for right-to-left content.
            // Clamp everything above the column to its top left.
            if (point.X() < gapAndColumnRect.X())
            {
              point = gapAndColumnRect.Location();
            }
            // Clamp everything below the column to the next column's top left. If there is
            // no next column, this still maps to just after this column.
            else if (point.X() >= gapAndColumnRect.MaxX())
            {
              point = gapAndColumnRect.Location();
              point.Move(gapAndColumnRect.Width(), 0_lu);
            }
          }

          LayoutSize offsetInColumn = point - columnRect.Location();
          return FragmentedFlowPortionRectAt(i).Location() + offsetInColumn;
        }
      }
    }

    return logicalPoint;
  }
}
