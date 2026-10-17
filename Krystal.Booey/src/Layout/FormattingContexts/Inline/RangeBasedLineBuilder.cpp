#include "Krystal.Booey/Layout/FormattingContexts/Inline/RangeBasedLineBuilder.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo::layout
{
  namespace
  {
    KRYS_NODISCARD inline bool HasInlineBoxesOnly(size_t inlineBoxCount, size_t numberOfInlineItems) noexcept
    {
      return !(numberOfInlineItems % 2) && inlineBoxCount == numberOfInlineItems / 2;
    }
  }

  RangeBasedLineBuilder::RangeBasedLineBuilder(InlineFormattingContext &inlineFormattingContext,
                                               HorizontalConstraints rootHorizontalConstraints,
                                               const InlineContentCache::InlineItems &inlineItems) noexcept
      : AbstractLineBuilder(inlineFormattingContext, inlineFormattingContext.Root(),
                            rootHorizontalConstraints, inlineItems.Content()),
        _textOnlySimpleLineBuilder(inlineFormattingContext,
                                   Downcast<ElementBox>(inlineItems.Content()[0].LayoutBox()),
                                   rootHorizontalConstraints, inlineItems.Content()),
        _inlineBoxCount(inlineItems.InlineBoxCount())
  {
    krys_debug_assert(_inlineBoxCount);
  }

  LineLayoutResult RangeBasedLineBuilder::LayoutInlineContent(const LineInput &lineInput,
                                                              const Maybe<PreviousLine> &previousLine,
                                                              bool isFirstFormattedLineCandidate) noexcept
  {
    auto numberOfInlineItems = _inlineItemList.size();
    if (HasInlineBoxesOnly(_inlineBoxCount, numberOfInlineItems))
    {
      Line::RunList inlineBoxRuns;
      inlineBoxRuns.reserve(numberOfInlineItems);
      for (auto &inlineItem : _inlineItemList)
      {
        krys_debug_assert(inlineItem.IsInlineBoxStartOrEnd());
        inlineBoxRuns.push_back(Line::Run(inlineItem, inlineItem.FirstLineStyle(), {}));
      }

      auto lineRect = lineInput.InitialLogicalRect;
      auto contentLeft =
        InlineFormattingUtils::HorizontalAlignmentOffset(RootStyle(), {}, lineRect.Width(), {}, true);
      LineLayoutResult lineLayoutResult;
      lineLayoutResult.ItemRange = lineInput.NeedsLayoutRange;
      lineLayoutResult.Runs = krys::move(inlineBoxRuns);
      lineLayoutResult.Content = {
        contentLeft, {}, contentLeft, std::max(0.f, contentLeft - lineRect.Right())};
      lineLayoutResult.Geometry = {lineRect.TopLeft(), lineRect.Width(), lineRect.Left()};
      lineLayoutResult.FirstLast = {
        isFirstFormattedLineCandidate ? IsFirstFormattedLine::Yes : IsFirstFormattedLine::No, {}};
      lineLayoutResult.ContentEnding = LineLayoutResult::InlineContentEnding::Generic;
      lineLayoutResult.NonSpanningInlineLevelBoxCount = _inlineBoxCount;
      return lineLayoutResult;
    }

    // 1. Shrink the layout range that we can run text-only builder on (currently it's just the
    // opening/closing inline box)
    // 2. Run text-only line builder
    // 3. Insert/append the missing inline box run
    auto adjustedNeedsLayoutRange = [&]
    {
      auto needsLayoutRange = lineInput.NeedsLayoutRange;
      if (isFirstFormattedLineCandidate)
      {
        krys_debug_assert(_inlineItemList.front().IsInlineBoxStart());
        krys_debug_assert(!needsLayoutRange.Start.Offset);
        // Skip leading InlineItemStart (e.g. <span>)
        ++needsLayoutRange.Start.Index;
      }
      // SKip trailing InlineItemEnd (e.g. </span>)
      krys_debug_assert(_inlineItemList.back().IsInlineBoxEnd());
      krys_debug_assert(!needsLayoutRange.End.Offset);
      --needsLayoutRange.End.Index;
      return needsLayoutRange;
    };
    auto needsLayoutRange = adjustedNeedsLayoutRange();
    krys_debug_assert(!needsLayoutRange.IsEmpty());

    auto lineLayoutResult = _textOnlySimpleLineBuilder.LayoutInlineContent(
      {needsLayoutRange, lineInput.InitialLogicalRect}, previousLine, isFirstFormattedLineCandidate);

    auto insertLeadingInlineBoxRun = [&]
    {
      auto &leadingInlineItem = _inlineItemList.front();
      krys_debug_assert(leadingInlineItem.IsInlineBoxStart());

      if (isFirstFormattedLineCandidate)
      {
        krys_debug_assert(!previousLine);
        lineLayoutResult.Runs.insert(lineLayoutResult.Runs.begin(),
                                     Line::Run(leadingInlineItem, leadingInlineItem.FirstLineStyle(), {}));
        lineLayoutResult.ItemRange.Start = lineInput.NeedsLayoutRange.Start;
        return;
      }
      // Subsequent lines need leading spanning inline box run.
      lineLayoutResult.Runs.insert(lineLayoutResult.Runs.begin(), Line::Run(leadingInlineItem, 0.f, 0.f));
    };
    insertLeadingInlineBoxRun();

    auto appendTrailingInlineBoxRunIfNeeded = [&]
    {
      if (lineLayoutResult.ItemRange.End != needsLayoutRange.End)
      {
        return;
      }
      auto &trailingInlineItem = _inlineItemList.back();
      lineLayoutResult.Runs.push_back(Line::Run(
        trailingInlineItem,
        isFirstFormattedLineCandidate ? trailingInlineItem.FirstLineStyle() : trailingInlineItem.Style(),
        lineLayoutResult.Content.LogicalWidth));
      lineLayoutResult.ItemRange.End = lineInput.NeedsLayoutRange.End;
    };
    appendTrailingInlineBoxRunIfNeeded();

    return lineLayoutResult;
  }

  bool RangeBasedLineBuilder::IsEligibleForRangeInlineLayout(
    const InlineFormattingContext &inlineFormattingContext, InlineItemRange needsLayoutRange,
    const InlineContentCache::InlineItems &inlineItems, const PlacedFloats &placedFloats) noexcept
  {
    if (inlineItems.IsEmpty())
    {
      return false;
    }

    if (needsLayoutRange.Start || needsLayoutRange.End.Index != inlineItems.Size())
    {
      return false;
    }

    // Range based line builder only supports the following content <inline box>eligible for text only
    // layout</inline box>
    auto &inlineItemList = inlineItems.Content();
    auto numberOfInlineItems = inlineItemList.size();
    auto isEmptyContent = HasInlineBoxesOnly(inlineItems.InlineBoxCount(), numberOfInlineItems);
    auto isFullyNestedContent = inlineItems.InlineBoxCount() == 1 && inlineItemList.front().IsInlineBoxStart()
                                && inlineItemList.back().IsInlineBoxEnd() && numberOfInlineItems > 2;
    if (!isEmptyContent && !isFullyNestedContent)
    {
      return false;
    }

    auto hasDecorationOrBreak = [&]
    {
      for (auto &inlineItem : inlineItemList)
      {
        if (!inlineItem.IsInlineBoxStart())
        {
          return false;
        }
        auto &inlineBox = inlineItem.LayoutBox();
        if (inlineFormattingContext.GeometryForBox(inlineBox).HorizontalMarginBorderAndPadding())
        {
          return true;
        }
        if (inlineBox.Style().GetBoxDecorationBreak() != style::ComputedStyle::InitialBoxDecorationBreak())
        {
          return true;
        }
      }
      krys_debug_assert(false);
      return true;
    };

    if (hasDecorationOrBreak())
    {
      // FIXME: Add start decoration support is just a matter of shrinking the available space for the first
      // line (or on subsequent lines when decoration break is present)
      return false;
    }

    if (inlineFormattingContext.GetLayoutState().ParentBlockLayoutState().GetLineClamp())
    {
      return false;
    }

    if (!isEmptyContent)
    {
      // Check the nested text content.
      if (!inlineItems.HasTextAndLineBreakOnlyContent() || inlineItems.RequiresVisualReordering()
          || !placedFloats.IsEmpty() || inlineItems.HasTextAutospace())
      {
        return false;
      }

      auto &rootStyle = inlineFormattingContext.Root().Style();
      auto &inlineBoxStyle = inlineItemList.front().LayoutBox().Style();
      if (inlineBoxStyle.GetTextAlign() != rootStyle.GetTextAlign())
      {
        return false;
      }
      if (!TextOnlySimpleLineBuilder::IsEligibleForSimplifiedInlineLayoutByStyle(
            inlineFormattingContext.Root())
          || !TextOnlySimpleLineBuilder::IsEligibleForSimplifiedInlineLayoutByStyle(
            inlineItemList.front().LayoutBox()))
      {
        return false;
      }
    }

    return true;
  }
}
