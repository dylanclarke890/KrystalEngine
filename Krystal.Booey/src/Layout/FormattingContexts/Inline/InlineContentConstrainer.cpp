#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentConstrainer.hpp"
#include "Krystal.Booey/Core/MathExtras.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineBuilder.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineTextItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>
#include <cmath>
#include <ranges>

namespace krys::boo::layout
{
  namespace
  {
    // Ideally, the act of balancing inline content will use the same number of lines as if the inline content
    // was laid out via `text-wrap: wrap`. However, adhering to this ideal is expensive (quadratic in the
    // number of break opportunities), and not caring about this ideal will allow us to use a more efficient
    // algorithm. Typically, if inline content spans many lines, the likelihood of someone caring about the
    // vertical space used decreases. So, we ignore this ideal number of lines requirement beyond this
    // threshold.
    constexpr size_t MaximumLinesToBalanceWithLineRequirement {12};

    // Define the penalty associated with show text wider/narrower than ideal bounds.
    // Separating stretchability and shrinkability allows us to weight under/over
    // filling the ideal bounds differently.
    constexpr InlineLayoutUnit TextWrapPrettyStretchability = 15;
    constexpr InlineLayoutUnit TextWrapPrettyShrinkability = 15;

    // Defines the maximum shrink/stretch factor allowed for text-wrap-pretty.
    constexpr float TextWrapPrettyMaxStretch = 3;
    constexpr float TextWrapPrettyMaxShrink = 3;

    // We would like 2 or more items on the last line for text-wrap-style:pretty to avoid orphans.
    constexpr size_t LastLinePreferredInlineItemCount = 2;

    KRYS_NODISCARD constexpr size_t LastLineBreakingPointOffset() noexcept
    {
      return 2 * LastLinePreferredInlineItemCount + 1;
    }

    // Use auto layout if ideal line width is too short relative to the largest inline item.
    // In these situations, text-wrap-pretty does very little of note other than take up time.
    KRYS_NODISCARD bool ValidIdealLineWidth(InlineLayoutUnit maxItemWidth, InlineLayoutUnit idealLineWidth,
                                            InlineLayoutUnit maxTextIndent) noexcept
    {
      return idealLineWidth >= maxItemWidth + maxTextIndent;
    }

    KRYS_NODISCARD bool ValidLineWidthPretty(InlineLayoutUnit candidateLineWidth,
                                             InlineLayoutUnit idealLineWidth) noexcept
    {
      auto difference = candidateLineWidth - idealLineWidth;
      if (difference > 0)
      {
        return difference <= TextWrapPrettyStretchability * TextWrapPrettyMaxStretch;
      }
      return std::abs(difference) <= TextWrapPrettyShrinkability * TextWrapPrettyMaxShrink;
    }

    // Full implementation of the raggedness function defined in:
    // http://www.eprg.org/G53DOC/pdfs/knuth-plass-breaking.pdf
    KRYS_NODISCARD float ComputeRaggedness(InlineLayoutUnit candidateLineWidth,
                                           InlineLayoutUnit idealLineWidth) noexcept
    {
      auto difference = candidateLineWidth - idealLineWidth;
      auto intermediate =
        difference / (difference > 0 ? TextWrapPrettyStretchability : TextWrapPrettyShrinkability);
      return 100 * std::abs(std::pow(intermediate, 3.f));
    }

    KRYS_NODISCARD float ComputeCostBalance(InlineLayoutUnit candidateLineWidth,
                                            InlineLayoutUnit idealLineWidth) noexcept
    {
      return ComputeRaggedness(candidateLineWidth, idealLineWidth);
    }

    KRYS_NODISCARD float ComputeCostPretty(InlineLayoutUnit candidateLineWidth,
                                           InlineLayoutUnit idealLineWidth, size_t breakIndex,
                                           size_t numberOfBreakOpportunities) noexcept
    {
      // FIXME: add support for river minimization.
      // Force max/minimum line width bounds if there are more then lastLinePreferredInlineItemCount items to
      // be laid out after the candidate line.
      if (numberOfBreakOpportunities > LastLineBreakingPointOffset()
          && breakIndex < numberOfBreakOpportunities - LastLineBreakingPointOffset())
      {
        if (!ValidLineWidthPretty(candidateLineWidth, idealLineWidth))
        {
          return std::numeric_limits<float>::infinity();
        }
      }
      return ComputeRaggedness(candidateLineWidth, idealLineWidth);
    }

    KRYS_NODISCARD LayoutUnit ComputeLineWidthFromSlidingWidth(InlineLayoutUnit indentWidth,
                                                               const SlidingWidth &slidingWidth) noexcept
    {
      return LayoutUnit::FromFloatCeil(indentWidth + slidingWidth.Width() + LayoutUnit::Epsilon());
    }

    KRYS_NODISCARD bool ContainsTrailingSoftHyphen(const InlineItem &inlineItem) noexcept
    {
      if (inlineItem.Style().GetHyphens() == Hyphens::None)
      {
        return false;
      }
      auto *textItem = DynamicDowncast<InlineTextItem>(inlineItem);
      if (!textItem)
      {
        return false;
      }
      return textItem->HasTrailingSoftHyphen();
    }

    KRYS_NODISCARD bool ContainsPreservedTab(const InlineItem &inlineItem) noexcept
    {
      auto *textItem = DynamicDowncast<InlineTextItem>(inlineItem);
      if (!textItem)
      {
        return false;
      }
      if (!textItem->IsWhitespace())
      {
        return false;
      }
      const auto &textBox = textItem->GetInlineTextBox();
      if (!TextUtil::ShouldPreserveSpacesAndTabs(textBox))
      {
        return false;
      }
      // A tab is a single byte, so the item's bytes can be compared directly.
      auto start = textItem->Start();
      auto length = textItem->Length();
      const auto &textContent = textBox.Content();
      for (size_t index = start; index < start + length; index++)
      {
        if (textContent[index] == u8'\t')
        {
          return true;
        }
      }
      return false;
    }

    KRYS_NODISCARD bool CannotConstrainInlineItem(const InlineItem &inlineItem) noexcept
    {
      // Opaque items are ignored by inline layout and do not affect constraint calculations.
      if (inlineItem.IsOpaque())
      {
        return false;
      }
      if (!inlineItem.LayoutBox().IsInlineLevelBox())
      {
        return true;
      }
      if (ContainsTrailingSoftHyphen(inlineItem))
      {
        return true;
      }
      if (ContainsPreservedTab(inlineItem))
      {
        return true;
      }
      if (inlineItem.Style().GetBoxDecorationBreak() == BoxDecorationBreak::Clone)
      {
        return true;
      }
      return false;
    }

    KRYS_NODISCARD PreviousLine BuildPreviousLine(size_t lineIndex,
                                                  LineLayoutResult &lineLayoutResult) noexcept
    {
      return PreviousLine {lineIndex, lineLayoutResult.Content.TrailingOverflowingContentWidth,
                           lineLayoutResult.EndsWithLineBreak(),
                           lineLayoutResult.BidiDirectionality.InlineBaseDirection,
                           krys::move(lineLayoutResult.Floats.SuspendedFloats)};
    }

    KRYS_NODISCARD bool HasIntrusiveOrNewlyPlacedFloat(const LineLayoutResult &lineLayoutResult) noexcept
    {
      return !lineLayoutResult.Floats.HasIntrusiveFloat.IsEmpty()
             || !lineLayoutResult.Floats.PlacedFloats.empty();
    }

    KRYS_NODISCARD Maybe<size_t> IndexOfBreakOpportunity(const List<size_t> &breakOpportunities,
                                                         size_t inlineItemIndex) noexcept
    {
      auto it = std::ranges::find(breakOpportunities, inlineItemIndex);
      if (it == breakOpportunities.end())
      {
        return {};
      }
      return static_cast<size_t>(std::distance(breakOpportunities.begin(), it));
    }
  }

  InlineContentConstrainer::InlineContentConstrainer(InlineFormattingContext &inlineFormattingContext,
                                                     const InlineItemList &inlineItemList,
                                                     HorizontalConstraints horizontalConstraints) noexcept
      : _inlineFormattingContext(inlineFormattingContext), _inlineItemList(inlineItemList),
        _horizontalConstraints(horizontalConstraints)
  {
    Initialize();
  }

  void InlineContentConstrainer::UpdateCachedWidths() noexcept
  {
    // We should only initialize the inline item width cache once.
    krys_debug_assert(!_hasValidInlineItemWidthCache);
    _inlineItemWidths = List<InlineLayoutUnit>(_numberOfInlineItems);
    _firstLineStyleInlineItemWidths = List<InlineLayoutUnit>(_numberOfInlineItems);
    // Cache inline item widths to speed up later computations.
    auto &formattingUtils = _inlineFormattingContext.FormattingUtils();
    for (size_t i = 0; i < _numberOfInlineItems; i++)
    {
      const auto &item = _inlineItemList[i];
      auto isWordSeparator = false;
      if (auto *textItem = DynamicDowncast<InlineTextItem>(item))
      {
        isWordSeparator = textItem->IsWordSeparator();
      }
      // Opaque items are ignored by inline layout. Skip over these items.
      if (!item.IsOpaque())
      {
        _inlineItemWidths[i] = formattingUtils.InlineItemWidth(item, 0, false)
                               + (isWordSeparator ? item.Style().UsedWordSpacing() : 0.0f);
        _inlineItemWidthsMax = std::max(_inlineItemWidthsMax, _inlineItemWidths[i]);
        _firstLineStyleInlineItemWidths[i] =
          formattingUtils.InlineItemWidth(item, 0, true)
          + (isWordSeparator ? item.FirstLineStyle().UsedWordSpacing() : 0.0f);
        _inlineItemWidthsMax = std::max(_inlineItemWidthsMax, _firstLineStyleInlineItemWidths[i]);
      }
    }
    _hasValidInlineItemWidthCache = true;
  }

  void InlineContentConstrainer::CheckCanConstrainInlineItems() noexcept
  {
    for (auto &inlineItem : _inlineItemList)
    {
      if (CannotConstrainInlineItem(inlineItem))
      {
        _cannotConstrainContent = true;
        return;
      }
    }
  }

  InlineContentConstrainer::EntryPretty InlineContentConstrainer::LayoutSingleLineForPretty(
    InlineItemRange layoutRange, InlineLayoutUnit idealLineWidth, const EntryPretty &lastValidEntry,
    size_t previousBreakIndex) noexcept
  {
    auto lineBuilder = LineBuilder {_inlineFormattingContext, _horizontalConstraints, _inlineItemList};
    // Hyphenation occurs when we require more space than is available. In this case, we should apply max
    // stretch to idealLineWidth.
    auto lineInitialRect =
      InlineRect {0.f, _horizontalConstraints.LogicalLeft.ToFloat(),
                  idealLineWidth + TextWrapPrettyStretchability * TextWrapPrettyMaxStretch, 0.f};
    auto lineLayoutResult = lineBuilder.LayoutInlineContent(
      {layoutRange, lineInitialRect}, lastValidEntry.PreviousLineInfo, !lastValidEntry.PreviousLineInfo);
    auto lineEnd = InlineFormattingUtils::LeadingInlineItemPositionForNextLine(
      lineLayoutResult.ItemRange.End, lastValidEntry.LineEnd,
      HasIntrusiveOrNewlyPlacedFloat(lineLayoutResult), layoutRange.End);
    bool didLayoutAllItems = lineEnd.Index == layoutRange.EndIndex();
    bool hasEnoughItemsForNextLine =
      layoutRange.EndIndex() > LastLineBreakingPointOffset()
      && lineEnd.Index < layoutRange.EndIndex() - LastLineBreakingPointOffset();
    if (didLayoutAllItems || hasEnoughItemsForNextLine)
    {
      auto lastLineWidth = lineLayoutResult.Content.LogicalWidth;
      // This function is only called when there are no more viable break points for PrettifyRange.
      // Use the last valid entry's accumulated cost as we must use this breakpoint no matter what.
      return {lastValidEntry.AccumulatedCost,
              previousBreakIndex,
              lastValidEntry.LineIndex + 1,
              lastLineWidth,
              lineEnd,
              BuildPreviousLine(lastValidEntry.LineIndex + 1, lineLayoutResult)};
    }
    // Handle the case where there would be too few items to be laid out in the next line.
    // Redo the layout to leave lastLinePreferredInlineItemCount items at the end to avoid orphans.
    auto shortenedLayoutRange = layoutRange;
    shortenedLayoutRange.End.Index = std::max(shortenedLayoutRange.StartIndex() + 1,
                                              shortenedLayoutRange.End.Index - LastLineBreakingPointOffset());
    auto shortenedLineLayoutResult =
      lineBuilder.LayoutInlineContent({shortenedLayoutRange, lineInitialRect},
                                      lastValidEntry.PreviousLineInfo, !lastValidEntry.PreviousLineInfo);
    auto shortenedLineEnd = InlineFormattingUtils::LeadingInlineItemPositionForNextLine(
      shortenedLineLayoutResult.ItemRange.End, lastValidEntry.LineEnd,
      HasIntrusiveOrNewlyPlacedFloat(shortenedLineLayoutResult), shortenedLayoutRange.End);
    auto lastLineWidth = shortenedLineLayoutResult.Content.LogicalWidth;
    // This function is only called when there are no more viable break points for PrettifyRange.
    // Use the last valid entry's accumulated cost as we must use this breakpoint no matter what.
    return {lastValidEntry.AccumulatedCost,
            previousBreakIndex,
            lastValidEntry.LineIndex + 1,
            lastLineWidth,
            shortenedLineEnd,
            BuildPreviousLine(lastValidEntry.LineIndex + 1, shortenedLineLayoutResult)};
  }

  void InlineContentConstrainer::Initialize() noexcept
  {
    auto lineClamp = _inlineFormattingContext.GetLayoutState().ParentBlockLayoutState().GetLineClamp();
    auto numberOfVisibleLinesAllowed = lineClamp ? Maybe<size_t> {lineClamp->MaximumLines} : null;

    if (!_inlineFormattingContext.GetLayoutState().GetPlacedFloats().IsEmpty())
    {
      _cannotConstrainContent = true;
      return;
    }

    // Do not adjust single line content.
    if (numberOfVisibleLinesAllowed == 1u)
    {
      _hasSingleLineVisibleContent = true;
      return;
    }

    _numberOfInlineItems = _inlineItemList.size();
    _maximumLineWidthConstraint = _horizontalConstraints.LogicalWidth;

    CheckCanConstrainInlineItems();
    if (_cannotConstrainContent)
    {
      return;
    }

    // Perform a line layout with `text-wrap: wrap` to compute useful metrics such as:
    //  - the number of lines used
    //  - the original widths of each line
    //  - forced break locations
    auto layoutRange = InlineItemRange {0, _inlineItemList.size()};
    auto lineBuilder = LineBuilder {_inlineFormattingContext, _horizontalConstraints, _inlineItemList};
    auto previousLineEnd = Maybe<InlineItemPosition> {};
    auto previousLine = Maybe<PreviousLine> {};
    auto isFirstFormattedLineCandidate = true;
    size_t lineIndex = 0;
    while (!layoutRange.IsEmpty())
    {
      auto lineInitialRect = InlineRect {0.f, _horizontalConstraints.LogicalLeft.ToFloat(),
                                         _horizontalConstraints.LogicalWidth.ToFloat(), 0.f};
      auto lineLayoutResult = lineBuilder.LayoutInlineContent({layoutRange, lineInitialRect}, previousLine,
                                                              isFirstFormattedLineCandidate);

      // Record relevant geometry measurements from one line layout
      _originalLineInlineItemRanges.push_back(lineLayoutResult.ItemRange);
      _originalLineEndsWithForcedBreak.push_back(lineLayoutResult.EndsWithLineBreak());
      bool useFirstLineStyle = !lineIndex;
      bool isFirstLineInChunk = !lineIndex || _originalLineEndsWithForcedBreak[lineIndex - 1];
      SlidingWidth lineSlidingWidth {*this,
                                     _inlineItemList,
                                     lineLayoutResult.ItemRange.StartIndex(),
                                     lineLayoutResult.ItemRange.EndIndex(),
                                     useFirstLineStyle,
                                     isFirstLineInChunk};
      auto previousLineEndsWithLineBreak =
        lineIndex ? Maybe<InlineFormattingUtils::
                            LineEndsWithLineBreak> {_originalLineEndsWithForcedBreak[lineIndex - 1]
                                                      ? InlineFormattingUtils::LineEndsWithLineBreak::Yes
                                                      : InlineFormattingUtils::LineEndsWithLineBreak::No}
                  : null;
      auto textIndent = _inlineFormattingContext.FormattingUtils().ComputedTextIndent(
        InlineFormattingUtils::IsIntrinsicWidthMode::No,
        isFirstFormattedLineCandidate ? IsFirstFormattedLine::Yes : IsFirstFormattedLine::No,
        previousLineEndsWithLineBreak, _maximumLineWidthConstraint.ToFloat());
      _originalLineConstraints.push_back(ComputeLineWidthFromSlidingWidth(textIndent, lineSlidingWidth));

      // If next line count would match (or exceed) the number of visible lines due to line-clamp, we can bail
      // out early.
      if (numberOfVisibleLinesAllowed && (lineIndex + 1 >= *numberOfVisibleLinesAllowed))
      {
        break;
      }

      layoutRange.Start = InlineFormattingUtils::LeadingInlineItemPositionForNextLine(
        lineLayoutResult.ItemRange.End, previousLineEnd, HasIntrusiveOrNewlyPlacedFloat(lineLayoutResult),
        layoutRange.End);
      previousLineEnd = layoutRange.Start;
      isFirstFormattedLineCandidate &= !lineLayoutResult.HasContentfulInFlowContent();
      previousLine = BuildPreviousLine(lineIndex, lineLayoutResult);
      lineIndex++;
    }

    // Cache inline item widths after laying out all inline content with LineBuilder.
    UpdateCachedWidths();
    _numberOfLinesInOriginalLayout = lineIndex;

    // Do not adjust single line content.
    if (_numberOfLinesInOriginalLayout == 1)
    {
      _hasSingleLineVisibleContent = true;
    }
  }

  Maybe<List<LayoutUnit>>
    InlineContentConstrainer::ComputeParagraphLevelConstraints(TextWrapStyle wrapStyle) noexcept
  {
    krys_debug_assert(wrapStyle == TextWrapStyle::Balance || wrapStyle == TextWrapStyle::Pretty);

    if (_cannotConstrainContent || _hasSingleLineVisibleContent)
    {
      return {};
    }

    // If forced line breaks exist, then we can constrain each forced-break-delimited
    // chunk of text separately. This helps simplify first line/indentation logic.
    List<size_t> chunkSizes; // Number of lines per chunk of text
    size_t currentChunkSize = 0;
    for (size_t i = 0; i < _originalLineInlineItemRanges.size(); i++)
    {
      currentChunkSize++;
      if (_originalLineEndsWithForcedBreak[i])
      {
        chunkSizes.push_back(currentChunkSize);
        currentChunkSize = 0;
      }
    }
    if (currentChunkSize > 0)
    {
      chunkSizes.push_back(currentChunkSize);
    }

    // Constrain each chunk
    auto constrainChunk = [&](size_t chunkStart, size_t chunkSize) -> Maybe<List<LayoutUnit>>
    {
      const bool isFirstChunk = !chunkStart;
      auto rangeToConstrain =
        InlineItemRange {_originalLineInlineItemRanges[chunkStart].StartIndex(),
                         _originalLineInlineItemRanges[chunkStart + chunkSize - 1].EndIndex()};
      if (rangeToConstrain.StartIndex() >= rangeToConstrain.EndIndex())
      {
        return {};
      }
      if (wrapStyle == TextWrapStyle::Balance)
      {
        InlineLayoutUnit totalWidth = 0.f;
        for (size_t line = 0; line < chunkSize; line++)
        {
          totalWidth += _originalLineConstraints[chunkStart + line].ToFloat();
        }

        const InlineLayoutUnit idealLineWidth = totalWidth / static_cast<InlineLayoutUnit>(chunkSize);
        if (_numberOfLinesInOriginalLayout <= MaximumLinesToBalanceWithLineRequirement)
        {
          return BalanceRangeWithLineRequirement(rangeToConstrain, idealLineWidth, chunkSize, isFirstChunk);
        }
        return BalanceRangeWithNoLineRequirement(rangeToConstrain, idealLineWidth, isFirstChunk);
      }

      if (wrapStyle == TextWrapStyle::Pretty)
      {
        const InlineLayoutUnit idealLineWidth =
          _maximumLineWidthConstraint.ToFloat() - TextWrapPrettyStretchability * TextWrapPrettyMaxStretch;
        return PrettifyRange(rangeToConstrain, idealLineWidth, isFirstChunk);
      }

      krys_debug_assert(false);
      return {};
    };

    size_t chunkStart = 0;
    List<LayoutUnit> constrainedLineWidths;
    for (auto chunkSize : chunkSizes)
    {
      auto constrainedLineWidthsForChunk = constrainChunk(chunkStart, chunkSize);
      if (!constrainedLineWidthsForChunk)
      {
        for (size_t i = 0; i < chunkSize; i++)
        {
          constrainedLineWidths.push_back(_maximumLineWidthConstraint);
        }
      }
      else
      {
        for (auto constrainedLineWidth : *constrainedLineWidthsForChunk)
        {
          constrainedLineWidths.push_back(constrainedLineWidth);
        }
      }
      chunkStart += chunkSize;
    }

    return constrainedLineWidths;
  }

  Maybe<List<LayoutUnit>> InlineContentConstrainer::BalanceRangeWithLineRequirement(
    InlineItemRange range, InlineLayoutUnit idealLineWidth, size_t numberOfLines, bool isFirstChunk) noexcept
  {
    krys_debug_assert(range.StartIndex() < range.EndIndex());

    // breakOpportunities holds the indices i such that a line break can occur before m_inlineItemList[i].
    auto breakOpportunities = ComputeBreakOpportunities(range);

    // We need a dummy break opportunity at the beginning for algorithmic base case purposes
    breakOpportunities.insert(breakOpportunities.begin(), range.StartIndex());
    auto numberOfBreakOpportunities = breakOpportunities.size();

    // Indentation offsets
    auto previousLineEndsWithLineBreak = isFirstChunk ? Maybe<InlineFormattingUtils::LineEndsWithLineBreak> {}
                                                      : InlineFormattingUtils::LineEndsWithLineBreak::Yes;
    auto firstLineTextIndent = ComputedTextIndent(
      isFirstChunk ? IsFirstFormattedLine::Yes : IsFirstFormattedLine::No, previousLineEndsWithLineBreak);
    auto textIndent =
      ComputedTextIndent(IsFirstFormattedLine::No, InlineFormattingUtils::LineEndsWithLineBreak::No);
    // state[i][j] holds the optimal set of line breaks where the jth line break (1-indexed) is
    // right before m_inlineItemList[breakOpportunities[i]]. "Optimal" in this context means the
    // lowest possible accumulated cost.
    List<List<EntryBalance>> state(numberOfBreakOpportunities, List<EntryBalance>(numberOfLines + 1));
    state[0][0].AccumulatedCost = 0.f;

    // Special case the first line because of ::first-line styling, indentation, etc.
    SlidingWidth firstLineSlidingWidth {
      *this, _inlineItemList, range.StartIndex(), range.StartIndex(), isFirstChunk, true};
    for (size_t breakIndex = 1; breakIndex < numberOfBreakOpportunities; breakIndex++)
    {
      auto end = breakOpportunities[breakIndex];
      firstLineSlidingWidth.AdvanceEndTo(end);

      auto firstLineCandidateWidth =
        ComputeLineWidthFromSlidingWidth(firstLineTextIndent, firstLineSlidingWidth);
      if (firstLineCandidateWidth > _maximumLineWidthConstraint)
      {
        break;
      }

      auto cost = ComputeCostBalance(firstLineCandidateWidth.ToFloat(), idealLineWidth);
      state[breakIndex][1].AccumulatedCost = cost;
    }

    // breakOpportunities[firstStartIndex] is the first possible starting position for a candidate line that
    // is NOT the first line
    size_t firstStartIndex = 1;
    SlidingWidth slidingWidth {
      *this, _inlineItemList, breakOpportunities[firstStartIndex], breakOpportunities[firstStartIndex], false,
      false};
    for (size_t breakIndex = 1; breakIndex < numberOfBreakOpportunities; breakIndex++)
    {
      size_t end = breakOpportunities[breakIndex];
      slidingWidth.AdvanceEndTo(end);

      // We prune our search space by limiting the possible starting positions for our candidate line.
      while (ComputeLineWidthFromSlidingWidth(textIndent, slidingWidth) > _maximumLineWidthConstraint)
      {
        firstStartIndex++;
        if (firstStartIndex > breakIndex)
        {
          break;
        }
        slidingWidth.AdvanceStartTo(breakOpportunities[firstStartIndex]);
      }

      // Evaluate all possible lines that break before m_inlineItemList[end]
      auto innerSlidingWidth = slidingWidth;
      for (size_t startIndex = firstStartIndex; startIndex < breakIndex; startIndex++)
      {
        size_t start = breakOpportunities[startIndex];
        krys_debug_assert(start != range.StartIndex());
        innerSlidingWidth.AdvanceStartTo(start);
        auto candidateLineWidth = ComputeLineWidthFromSlidingWidth(textIndent, innerSlidingWidth);
        auto candidateLineCost = ComputeCostBalance(candidateLineWidth.ToFloat(), idealLineWidth);
        krys_debug_assert(candidateLineWidth <= _maximumLineWidthConstraint);

        // Compute the cost of this line based on the line index
        for (size_t lineIndex = 1; lineIndex <= numberOfLines; lineIndex++)
        {
          auto accumulatedCost = candidateLineCost + state[startIndex][lineIndex - 1].AccumulatedCost;
          auto currentAccumulatedCost = state[breakIndex][lineIndex].AccumulatedCost;
          if (accumulatedCost < currentAccumulatedCost
              || AreEssentiallyEqual(accumulatedCost, currentAccumulatedCost))
          {
            state[breakIndex][lineIndex].AccumulatedCost = accumulatedCost;
            state[breakIndex][lineIndex].PreviousBreakIndex = startIndex;
          }
        }
      }
    }

    // Check if we found no solution
    if (std::isinf(state[numberOfBreakOpportunities - 1][numberOfLines].AccumulatedCost))
    {
      return {};
    }

    // breaks[i] equals the index into m_inlineItemList before which the ith line will break
    List<size_t> breaks(numberOfLines);
    size_t breakIndex = numberOfBreakOpportunities - 1;
    for (size_t line = numberOfLines; line > 0; line--)
    {
      breaks[line - 1] = breakOpportunities[breakIndex];
      breakIndex = state[breakIndex][line].PreviousBreakIndex;
    }

    return ComputeLineWidthsFromBreaks(range, breaks, isFirstChunk);
  }

  Maybe<List<LayoutUnit>> InlineContentConstrainer::BalanceRangeWithNoLineRequirement(
    InlineItemRange range, InlineLayoutUnit idealLineWidth, bool isFirstChunk) noexcept
  {
    krys_debug_assert(range.StartIndex() < range.EndIndex());

    // breakOpportunities holds the indices i such that a line break can occur before m_inlineItemList[i].
    auto breakOpportunities = ComputeBreakOpportunities(range);
    if (breakOpportunities.size() == 1)
    {
      return {};
    }

    // We need a dummy break opportunity at the beginning for algorithmic base case purposes
    breakOpportunities.insert(breakOpportunities.begin(), range.StartIndex());
    auto numberOfBreakOpportunities = breakOpportunities.size();

    // Indentation offsets
    auto previousLineEndsWithLineBreak = isFirstChunk ? Maybe<InlineFormattingUtils::LineEndsWithLineBreak> {}
                                                      : InlineFormattingUtils::LineEndsWithLineBreak::Yes;
    auto firstLineTextIndent = ComputedTextIndent(
      isFirstChunk ? IsFirstFormattedLine::Yes : IsFirstFormattedLine::No, previousLineEndsWithLineBreak);
    auto textIndent =
      ComputedTextIndent(IsFirstFormattedLine::No, InlineFormattingUtils::LineEndsWithLineBreak::No);

    // state[i] holds the optimal set of line breaks where the last line break is right
    // before m_inlineItemList[breakOpportunities[i]]. "Optimal" in this context means the
    // lowest possible accumulated cost.
    List<EntryBalance> state(numberOfBreakOpportunities);
    state[0].AccumulatedCost = 0.f;

    // Special case the first line because of ::first-line styling, indentation, etc.
    SlidingWidth firstLineSlidingWidth {
      *this, _inlineItemList, range.StartIndex(), range.StartIndex(), isFirstChunk, true};
    for (size_t breakIndex = 1; breakIndex < numberOfBreakOpportunities; breakIndex++)
    {
      auto end = breakOpportunities[breakIndex];
      firstLineSlidingWidth.AdvanceEndTo(end);

      auto firstLineCandidateWidth =
        ComputeLineWidthFromSlidingWidth(firstLineTextIndent, firstLineSlidingWidth);
      if (firstLineCandidateWidth > _maximumLineWidthConstraint)
      {
        break;
      }

      auto cost = ComputeCostBalance(firstLineCandidateWidth.ToFloat(), idealLineWidth);
      state[breakIndex].AccumulatedCost = cost;
    }

    // breakOpportunities[firstStartIndex] is the first possible starting position for a candidate line that
    // is NOT the first line
    size_t firstStartIndex = 1;
    SlidingWidth slidingWidth {
      *this, _inlineItemList, breakOpportunities[firstStartIndex], breakOpportunities[firstStartIndex], false,
      false};
    for (size_t breakIndex = 1; breakIndex < numberOfBreakOpportunities; breakIndex++)
    {
      size_t end = breakOpportunities[breakIndex];
      slidingWidth.AdvanceEndTo(end);

      // We prune our search space by limiting the possible starting positions for our candidate line.
      while (ComputeLineWidthFromSlidingWidth(textIndent, slidingWidth) > _maximumLineWidthConstraint)
      {
        firstStartIndex++;
        if (firstStartIndex > breakIndex)
        {
          break;
        }
        slidingWidth.AdvanceStartTo(breakOpportunities[firstStartIndex]);
      }

      // Evaluate all possible lines that break before m_inlineItemList[end]
      auto innerSlidingWidth = slidingWidth;
      for (size_t startIndex = firstStartIndex; startIndex < breakIndex; startIndex++)
      {
        size_t start = breakOpportunities[startIndex];
        krys_debug_assert(start != range.StartIndex());
        innerSlidingWidth.AdvanceStartTo(start);
        auto candidateLineWidth = ComputeLineWidthFromSlidingWidth(textIndent, innerSlidingWidth);
        auto candidateLineCost = ComputeCostBalance(candidateLineWidth.ToFloat(), idealLineWidth);
        krys_debug_assert(candidateLineWidth <= _maximumLineWidthConstraint);

        auto accumulatedCost = candidateLineCost + state[startIndex].AccumulatedCost;
        if (accumulatedCost < state[breakIndex].AccumulatedCost)
        {
          state[breakIndex].AccumulatedCost = accumulatedCost;
          state[breakIndex].PreviousBreakIndex = startIndex;
        }
      }
    }

    // Check if we found no solution
    if (std::isinf(state[numberOfBreakOpportunities - 1].AccumulatedCost))
    {
      return {};
    }

    // breaks[i] equals the index into m_inlineItemList before which the ith line will break
    List<size_t> breaks;
    size_t breakIndex = numberOfBreakOpportunities - 1;
    do
    {
      breaks.push_back(breakOpportunities[breakIndex]);
      breakIndex = state[breakIndex].PreviousBreakIndex;
    } while (breakIndex);
    std::ranges::reverse(breaks);

    return ComputeLineWidthsFromBreaks(range, breaks, isFirstChunk);
  }

  Maybe<List<LayoutUnit>> InlineContentConstrainer::PrettifyRange(InlineItemRange range,
                                                                  InlineLayoutUnit idealLineWidth,
                                                                  bool isFirstChunk) noexcept
  {
    krys_debug_assert(range.StartIndex() < range.EndIndex());
    // Fall back to auto layout if the ideal line width is too narrow relative to the width of the largest
    // inline item.
    if (!ValidIdealLineWidth(_inlineItemWidthsMax, idealLineWidth, ComputeMaxTextIndent()))
    {
      return {};
    }

    // breakOpportunities holds the indices i such that a line break can occur before m_inlineItemList[i].
    auto breakOpportunities = ComputeBreakOpportunities(range);
    if (breakOpportunities.size() == 1)
    {
      return {};
    }

    // We need a dummy break opportunity at the beginning for algorithmic base case purposes
    breakOpportunities.insert(breakOpportunities.begin(), range.StartIndex());
    auto numberOfBreakOpportunities = breakOpportunities.size();

    // Indentation offsets
    auto previousLineEndsWithLineBreak = isFirstChunk ? Maybe<InlineFormattingUtils::LineEndsWithLineBreak> {}
                                                      : InlineFormattingUtils::LineEndsWithLineBreak::Yes;
    auto firstLineTextIndent = ComputedTextIndent(
      isFirstChunk ? IsFirstFormattedLine::Yes : IsFirstFormattedLine::No, previousLineEndsWithLineBreak);
    auto textIndent =
      ComputedTextIndent(IsFirstFormattedLine::No, InlineFormattingUtils::LineEndsWithLineBreak::No);

    // state[i] holds the optimal set of line breaks where the last line break is right
    // before m_inlineItemList[breakOpportunities[i]]. "Optimal" in this context means the
    // lowest possible accumulated cost.
    List<EntryPretty> state(numberOfBreakOpportunities);
    state[0].AccumulatedCost = 0.f;
    Maybe<size_t> lastValidStateIndex;

    // Special case the first line because of ::first-line styling, indentation, etc.
    SlidingWidth firstLineSlidingWidth {
      *this, _inlineItemList, range.StartIndex(), range.StartIndex(), isFirstChunk, true};
    for (size_t breakIndex = 1; breakIndex < numberOfBreakOpportunities; breakIndex++)
    {
      auto end = breakOpportunities[breakIndex];
      firstLineSlidingWidth.AdvanceEndTo(end);

      auto firstLineCandidateWidth =
        ComputeLineWidthFromSlidingWidth(firstLineTextIndent, firstLineSlidingWidth);
      if (firstLineCandidateWidth > _maximumLineWidthConstraint)
      {
        break;
      }

      auto cost = ComputeCostPretty(firstLineCandidateWidth.ToFloat(), idealLineWidth, breakIndex,
                                    numberOfBreakOpportunities);
      if (cost < state[breakIndex].AccumulatedCost)
      {
        lastValidStateIndex = breakIndex;
        auto &entry = state[breakIndex];
        entry.AccumulatedCost = cost;
        entry.PreviousBreakIndex = 0;
        entry.LineIndex = state[0].LineIndex + 1;
        entry.LastLineWidth = firstLineCandidateWidth.ToFloat();
        entry.LineEnd = {end, 0};
        auto lineInitialRect = InlineRect {0.f, _horizontalConstraints.LogicalLeft.ToFloat(),
                                           firstLineCandidateWidth.ToFloat(), 0.f};
        auto lineBuilder = LineBuilder {_inlineFormattingContext, _horizontalConstraints, _inlineItemList};
        auto lineLayoutResult =
          lineBuilder.LayoutInlineContent({{range.StartIndex(), end}, lineInitialRect},
                                          state[0].PreviousLineInfo, !state[0].PreviousLineInfo);
        entry.PreviousLineInfo = BuildPreviousLine(entry.LineIndex, lineLayoutResult);
      }
    }

    // If we are unable to build a valid line from without hyphenation,
    // try to build a valid line using hyphenation from the beginning.
    if (!lastValidStateIndex)
    {
      auto newEntry =
        LayoutSingleLineForPretty({breakOpportunities[0], range.EndIndex()}, idealLineWidth, state[0], 0);
      auto newEntryIndex = IndexOfBreakOpportunity(breakOpportunities, newEntry.LineEnd.Index);
      if (!newEntryIndex)
      {
        return {};
      }
      lastValidStateIndex = newEntryIndex;
      state[*lastValidStateIndex] = krys::move(newEntry);
    }
    krys_debug_assert(lastValidStateIndex.has_value());

    // breakOpportunities[firstStartIndex] is the first possible starting position for a candidate line that
    // is NOT the first line
    size_t firstStartIndex = 1;
    SlidingWidth slidingWidth {
      *this, _inlineItemList, breakOpportunities[firstStartIndex], breakOpportunities[firstStartIndex], false,
      false};
    // breakIndex should always be one or more break opportunities ahead of firstStartIndex.
    for (size_t breakIndex = firstStartIndex + 1; breakIndex < numberOfBreakOpportunities; breakIndex++)
    {
      size_t end = breakOpportunities[breakIndex];
      slidingWidth.AdvanceEndTo(end);

      // We prune our search space by limiting the possible starting positions for our candidate line.
      while (ComputeLineWidthFromSlidingWidth(textIndent, slidingWidth) > _maximumLineWidthConstraint)
      {
        firstStartIndex++;
        if (firstStartIndex >= breakIndex)
        {
          break;
        }
        slidingWidth.AdvanceStartTo(breakOpportunities[firstStartIndex]);
      }
      krys_debug_assert(firstStartIndex <= breakIndex);

      // If the start of our slidingWidth is past the last valid breaking point, we will not be able to find a
      // valid solution. Try to find a solution using hyphenation.
      if (firstStartIndex > *lastValidStateIndex)
      {
        // Perform a single line layout from lastValidStateIndex.

        // Sanity check indices before proceeding.
        if (*lastValidStateIndex >= breakOpportunities.size() || *lastValidStateIndex >= state.size())
        {
          krys_debug_assert(false);
          return {};
        }

        auto newEntry =
          LayoutSingleLineForPretty({state[*lastValidStateIndex].LineEnd.Index, range.EndIndex()},
                                    idealLineWidth, state[*lastValidStateIndex], *lastValidStateIndex);
        auto newEntryIndex = IndexOfBreakOpportunity(breakOpportunities, newEntry.LineEnd.Index);
        // If hyphenation does not create a valid solution, we should return early.
        if (!newEntryIndex)
        {
          return {};
        }
        lastValidStateIndex = newEntryIndex;
        state[*lastValidStateIndex] = krys::move(newEntry);
        // If hyphenation does not create a valid solution, we should return early.
        if (*lastValidStateIndex == state[*lastValidStateIndex].PreviousBreakIndex)
        {
          return {};
        }
      }

      // Evaluate all possible lines that break before m_inlineItemList[end]
      auto innerSlidingWidth = slidingWidth;
      for (size_t startIndex = firstStartIndex; startIndex < breakIndex; startIndex++)
      {
        if (state[startIndex].AccumulatedCost == std::numeric_limits<float>::infinity())
        {
          continue;
        }
        size_t start = breakOpportunities[startIndex];
        krys_debug_assert(start != range.StartIndex());
        innerSlidingWidth.AdvanceStartTo(start);
        auto candidateLineWidth = ComputeLineWidthFromSlidingWidth(textIndent, innerSlidingWidth);
        // FIXME: Update candidateLineWidth here using state[startIndex].lineEnd.offset.
        auto candidateLineCost = ComputeCostPretty(candidateLineWidth.ToFloat(), idealLineWidth, breakIndex,
                                                   numberOfBreakOpportunities);
        auto accumulatedCost = candidateLineCost + state[startIndex].AccumulatedCost;

        if (accumulatedCost < state[breakIndex].AccumulatedCost)
        {
          lastValidStateIndex = breakIndex;
          auto &entry = state[breakIndex];
          entry.AccumulatedCost = accumulatedCost;
          krys_debug_assert(breakIndex > startIndex);
          entry.PreviousBreakIndex = startIndex;
          entry.LastLineWidth = candidateLineWidth.ToFloat();
          entry.LineEnd = {end, 0};
          entry.LineIndex = state[startIndex].LineIndex + 1;
          auto lineInitialRect =
            InlineRect {0.f, _horizontalConstraints.LogicalLeft.ToFloat(), candidateLineWidth.ToFloat(), 0.f};
          auto lineBuilder = LineBuilder {_inlineFormattingContext, _horizontalConstraints, _inlineItemList};
          auto lineLayoutResult = lineBuilder.LayoutInlineContent({{start, end}, lineInitialRect},
                                                                  state[startIndex].PreviousLineInfo,
                                                                  !state[startIndex].PreviousLineInfo);
          entry.PreviousLineInfo = BuildPreviousLine(entry.LineIndex, lineLayoutResult);
        }
      }
    }

    // Check if we found no solution
    if (std::isinf(state[numberOfBreakOpportunities - 1].AccumulatedCost))
    {
      return {};
    }
    // breaks[i] equals the index into m_inlineItemList before which the ith line will break
    List<LayoutUnit> widths;
    size_t breakIndex = numberOfBreakOpportunities - 1;
    do
    {
      // state[breakIndex].previousBreakIndex should always be less than breakIndex.
      // If this invariant fails, we will find ourselves in an infinite loop.
      // In the case we should fall back to auto layout.
      if (breakIndex <= state[breakIndex].PreviousBreakIndex)
      {
        krys_debug_assert(false);
        return {};
      }
      widths.push_back(LayoutUnit {state[breakIndex].LastLineWidth});
      breakIndex = state[breakIndex].PreviousBreakIndex;
    } while (breakIndex);
    std::ranges::reverse(widths);
    return widths;
  }

  InlineLayoutUnit InlineContentConstrainer::InlineItemWidth(size_t inlineItemIndex,
                                                             bool useFirstLineStyle) const noexcept
  {
    // Opaque items are ignored by inline layout. Skip over this item by setting its width to 0.
    if (_inlineItemList[inlineItemIndex].IsOpaque())
    {
      return {};
    }
    if (_hasValidInlineItemWidthCache)
    {
      return useFirstLineStyle ? _firstLineStyleInlineItemWidths[inlineItemIndex]
                               : _inlineItemWidths[inlineItemIndex];
    }
    // If inline items width cache has not yet been initialized, we should explicitly calculate the item's
    // width.
    return _inlineFormattingContext.FormattingUtils().InlineItemWidth(_inlineItemList[inlineItemIndex], 0,
                                                                      useFirstLineStyle);
  }

  bool InlineContentConstrainer::ShouldTrimLeading(size_t inlineItemIndex, bool useFirstLineStyle,
                                                   bool isFirstLineInChunk) const noexcept
  {
    auto &inlineItem = _inlineItemList[inlineItemIndex];
    auto &style = useFirstLineStyle ? inlineItem.FirstLineStyle() : inlineItem.Style();

    // Handle line break first so we can focus on other types of white space
    if (inlineItem.IsLineBreak())
    {
      return true;
    }

    if (auto *textItem = DynamicDowncast<InlineTextItem>(inlineItem))
    {
      if (textItem->IsWhitespace())
      {
        bool isFirstLineLeadingPreservedWhiteSpace =
          style.GetWhiteSpaceCollapse() == WhiteSpaceCollapse::Preserve && isFirstLineInChunk;
        return !isFirstLineLeadingPreservedWhiteSpace
               && style.GetWhiteSpaceCollapse() != WhiteSpaceCollapse::BreakSpaces;
      }
      return false;
    }

    if (InlineItemWidth(inlineItemIndex, useFirstLineStyle) <= 0)
    {
      return true;
    }

    return false;
  }

  bool InlineContentConstrainer::ShouldTrimTrailing(size_t inlineItemIndex,
                                                    bool useFirstLineStyle) const noexcept
  {
    auto &inlineItem = _inlineItemList[inlineItemIndex];
    auto &style = useFirstLineStyle ? inlineItem.FirstLineStyle() : inlineItem.Style();

    // Handle line break first so we can focus on other types of white space
    if (inlineItem.IsLineBreak())
    {
      return true;
    }

    if (auto *textItem = DynamicDowncast<InlineTextItem>(inlineItem))
    {
      if (textItem->IsWhitespace())
      {
        return style.GetWhiteSpaceCollapse() != WhiteSpaceCollapse::BreakSpaces;
      }
      return false;
    }

    if (InlineItemWidth(inlineItemIndex, useFirstLineStyle) <= 0)
    {
      return true;
    }

    return false;
  }

  SlidingWidth::SlidingWidth(const InlineContentConstrainer &inlineContentConstrainer,
                             KRYS_MAYBE_UNUSED const InlineItemList &inlineItemList, size_t start, size_t end,
                             bool useFirstLineStyle, bool isFirstLineInChunk) noexcept
      : _inlineContentConstrainer(inlineContentConstrainer),
#if KRYS_DEBUG
        _inlineItemList(inlineItemList),
#endif
        _start(start), _end(start), _useFirstLineStyle(useFirstLineStyle),
        _isFirstLineInChunk(isFirstLineInChunk)
  {
    krys_debug_assert(start <= end);
    AdvanceEndTo(end);
  }

  InlineLayoutUnit SlidingWidth::Width() const noexcept
  {
    return _totalWidth - _leadingTrimmableWidth - _trailingTrimmableWidth;
  }

  void SlidingWidth::AdvanceStart() noexcept
  {
    krys_debug_assert(_start < _end);
    auto startItemIndex = _start;
    auto startItemWidth = _inlineContentConstrainer.InlineItemWidth(startItemIndex, _useFirstLineStyle);
    _totalWidth -= startItemWidth;
    _start++;

    if (_inlineContentConstrainer.ShouldTrimLeading(startItemIndex, _useFirstLineStyle, _isFirstLineInChunk))
    {
      _leadingTrimmableWidth -= startItemWidth;
      return;
    }

    _firstLeadingNonTrimmedItem = null;
    _leadingTrimmableWidth = 0;
    for (auto current = _start; current < _end; current++)
    {
      if (!_inlineContentConstrainer.ShouldTrimLeading(current, _useFirstLineStyle, _isFirstLineInChunk))
      {
        _firstLeadingNonTrimmedItem = current;
        break;
      }
      _leadingTrimmableWidth += _inlineContentConstrainer.InlineItemWidth(current, _useFirstLineStyle);
    }

    // Update trailing logic if necessary:
    //   1: Check if the removed start item was the first trailing item
    //   2: Check if the first non trimmed leading item surpassed the first trailing item
    // In both cases, we should have m_leadingTrimmableWidth + m_trailingTrimmableWidth = m_totalWidth
    if (_leadingTrimmableWidth + _trailingTrimmableWidth > _totalWidth)
    {
      _trailingTrimmableWidth = _totalWidth - _leadingTrimmableWidth;
    }
  }

  void SlidingWidth::AdvanceStartTo(size_t newStart) noexcept
  {
    krys_debug_assert(_start <= newStart);
    while (_start < newStart)
    {
      AdvanceStart();
    }
  }

  void SlidingWidth::AdvanceEnd() noexcept
  {
#if KRYS_DEBUG
    krys_debug_assert(_end < _inlineItemList.size());
#endif
    auto endItemIndex = _end;
    auto endItemWidth = _inlineContentConstrainer.InlineItemWidth(endItemIndex, _useFirstLineStyle);
    _totalWidth += endItemWidth;
    _end++;

    if (!_firstLeadingNonTrimmedItem)
    {
      if (_inlineContentConstrainer.ShouldTrimLeading(endItemIndex, _useFirstLineStyle, _isFirstLineInChunk))
      {
        _leadingTrimmableWidth += endItemWidth;
        return;
      }
      _firstLeadingNonTrimmedItem = endItemIndex;
      return;
    }

    if (_inlineContentConstrainer.ShouldTrimTrailing(_end - 1, _useFirstLineStyle))
    {
      _trailingTrimmableWidth += endItemWidth;
      return;
    }

    _trailingTrimmableWidth = 0;
  }

  void SlidingWidth::AdvanceEndTo(size_t newEnd) noexcept
  {
    krys_debug_assert(_end <= newEnd);
    while (_end < newEnd)
    {
      AdvanceEnd();
    }
  }

  List<size_t> InlineContentConstrainer::ComputeBreakOpportunities(InlineItemRange range) const noexcept
  {
    List<size_t> breakOpportunities;
    size_t currentIndex = range.StartIndex();
    while (currentIndex < range.EndIndex())
    {
      currentIndex = _inlineFormattingContext.FormattingUtils().NextWrapOpportunity(
        currentIndex, range, Span<const InlineItem> {_inlineItemList});
      // FIXME: we should not consider the range end as breaking opportunity.
      breakOpportunities.push_back(currentIndex);
    }
    return breakOpportunities;
  }

  List<LayoutUnit> InlineContentConstrainer::ComputeLineWidthsFromBreaks(InlineItemRange inlineItems,
                                                                         const List<size_t> &breaks,
                                                                         bool isFirstChunk) const noexcept
  {
    List<LayoutUnit> lineWidths(breaks.size());
    auto previousLineEndsWithLineBreak = isFirstChunk ? Maybe<InlineFormattingUtils::LineEndsWithLineBreak> {}
                                                      : InlineFormattingUtils::LineEndsWithLineBreak::Yes;
    auto firstLineTextIndent = ComputedTextIndent(
      isFirstChunk ? IsFirstFormattedLine::Yes : IsFirstFormattedLine::No, previousLineEndsWithLineBreak);
    auto textIndent =
      ComputedTextIndent(IsFirstFormattedLine::No, InlineFormattingUtils::LineEndsWithLineBreak::No);
    for (size_t i = 0; i < breaks.size(); i++)
    {
      auto start = !i ? inlineItems.StartIndex() : breaks[i - 1];
      auto end = breaks[i];
      auto indentWidth = !i ? firstLineTextIndent : textIndent;
      SlidingWidth slidingWidth {*this, _inlineItemList, start, end, !i && isFirstChunk, !i};
      lineWidths[i] = ComputeLineWidthFromSlidingWidth(indentWidth, slidingWidth);
    }
    return lineWidths;
  }

  InlineLayoutUnit InlineContentConstrainer::ComputeMaxTextIndent() const noexcept
  {
    auto noPreviousLineTextIndent = ComputedTextIndent(IsFirstFormattedLine::Yes, {});
    auto firstLineTextIndent =
      ComputedTextIndent(IsFirstFormattedLine::Yes, InlineFormattingUtils::LineEndsWithLineBreak::Yes);
    auto textIndent =
      ComputedTextIndent(IsFirstFormattedLine::No, InlineFormattingUtils::LineEndsWithLineBreak::No);

    // Return the maximum indent value
    return std::max({noPreviousLineTextIndent, firstLineTextIndent, textIndent});
  }

  InlineLayoutUnit InlineContentConstrainer::ComputedTextIndent(
    IsFirstFormattedLine isFirstFormattedLine,
    Maybe<InlineFormattingUtils::LineEndsWithLineBreak> previousLineEndsWithLineBreak) const noexcept
  {
    return _inlineFormattingContext.FormattingUtils().ComputedTextIndent(
      InlineFormattingUtils::IsIntrinsicWidthMode::No, isFirstFormattedLine, previousLineEndsWithLineBreak,
      _maximumLineWidthConstraint.ToFloat());
  }
}
