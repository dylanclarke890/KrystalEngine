#include "Krystal.Booey/Layout/FormattingContexts/Inline/TextOnlySimpleLineBuilder.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"

namespace krys::boo::layout
{
  struct TextOnlyLineBreakResult
  {
    InlineContentBreaker::IsEndOfLine EndOfLine {InlineContentBreaker::IsEndOfLine::Yes};
    size_t CommittedCount {0};
    size_t OverflowingContentLength {0};
    Maybe<InlineLayoutUnit> OverflowLogicalWidth {};
    bool IsRevert {false};
  };

  struct CandidateTextContent
  {
    void Append(InlineLayoutUnit contentWidth) noexcept
    {
      LogicalWidth += contentWidth;
      ++EndIndex;
    }

    size_t StartIndex {0};
    size_t EndIndex {0};
    InlineLayoutUnit LogicalWidth {0.f};
  };

  namespace
  {
    KRYS_NODISCARD inline InlineLayoutUnit
      MeasuredInlineTextItem(const InlineTextItem &inlineTextItem, const RenderStyle &style,
                             InlineLayoutUnit contentLogicalLeft) noexcept
    {
      krys_debug_assert(!inlineTextItem.Width());
      if (!inlineTextItem.IsWhitespace() || InlineTextItem::ShouldPreserveSpacesAndTabs(inlineTextItem))
      {
        return TextUtil::Width(inlineTextItem, style.GetFontCascade(), contentLogicalLeft);
      }
      return TextUtil::Width(inlineTextItem, style.GetFontCascade(), inlineTextItem.Start(),
                             inlineTextItem.Start() + 1, contentLogicalLeft);
    }

    KRYS_NODISCARD inline InlineItemPosition
      PlacedInlineItemEnd(size_t layoutRangeStartIndex, size_t placedInlineItemCount,
                          size_t overflowingContentLength, Span<const InlineItem> inlineItemList) noexcept
    {
      if (!overflowingContentLength)
      {
        return {layoutRangeStartIndex + placedInlineItemCount};
      }

      auto trailingInlineItemIndex = layoutRangeStartIndex + placedInlineItemCount - 1;
      size_t overflowingInlineTextItemLength =
        Downcast<InlineTextItem>(inlineItemList[trailingInlineItemIndex]).Length();
      return {trailingInlineItemIndex, overflowingInlineTextItemLength - overflowingContentLength};
    }

    KRYS_NODISCARD inline bool IsLastLineWithInlineContent(InlineItemPosition placedContentEnd,
                                                           size_t layoutRangeEndIndex) noexcept
    {
      return placedContentEnd.Index == layoutRangeEndIndex && !placedContentEnd.Offset;
    }

    inline bool ConsumeTrailingLineBreakIfApplicable(const TextOnlyLineBreakResult &result,
                                                     size_t trailingInlineItemIndex, size_t layoutRangeEnd,
                                                     Line &line,
                                                     Span<const InlineItem> inlineItemList) noexcept
    {
      // Trailing forced line break should be consumed after fully placed content.
      auto shouldConsumeTrailingLineBreak = [&]
      {
        if (result.OverflowingContentLength || result.IsRevert)
        {
          return false;
        }
        return trailingInlineItemIndex < layoutRangeEnd
               && inlineItemList[trailingInlineItemIndex].IsLineBreak();
      };
      if (!shouldConsumeTrailingLineBreak())
      {
        return false;
      }
      auto &trailingLineBreak = inlineItemList[trailingInlineItemIndex];
      krys_debug_assert(trailingLineBreak.IsLineBreak());
      line.AppendLineBreak(trailingLineBreak, trailingLineBreak.Style());
      return true;
    }
  }

  TextOnlySimpleLineBuilder::TextOnlySimpleLineBuilder(InlineFormattingContext &inlineFormattingContext,
                                                       const ElementBox &rootBox,
                                                       HorizontalConstraints rootHorizontalConstraints,
                                                       const InlineItemList &inlineItemList) noexcept
      : AbstractLineBuilder(inlineFormattingContext, rootBox, rootHorizontalConstraints, inlineItemList)
  {
  }

  LineLayoutResult TextOnlySimpleLineBuilder::LayoutInlineContent(const LineInput &lineInput,
                                                                  const Maybe<PreviousLine> &previousLine,
                                                                  bool isFirstFormattedLineCandidate) noexcept
  {
    if (auto lineLayoutResult =
          PlaceSingleCharacterContentIfApplicable(lineInput, isFirstFormattedLineCandidate))
    {
      krys_debug_assert(!previousLine);
      return krys::move(*lineLayoutResult);
    }

    Initialize(lineInput.NeedsLayoutRange, lineInput.InitialLogicalRect, previousLine,
               isFirstFormattedLineCandidate);
    auto &rootStyle = RootStyle();
    auto placedContentEnd = TextUtil::IsWrappingAllowed(rootStyle)
                              ? PlaceInlineTextContent(rootStyle, lineInput.NeedsLayoutRange)
                              : PlaceNonWrappingInlineTextContent(rootStyle, lineInput.NeedsLayoutRange);
    auto result = _line.Close();

    auto isLastInlineContent =
      IsLastLineWithInlineContent(placedContentEnd, lineInput.NeedsLayoutRange.EndIndex());
    auto lineEndsWithForcedLineBreak = Line::HasTrailingForcedLineBreak(result.Runs);
    auto isLastLineOrLineEndsWithForcedLineBreak = isLastInlineContent || lineEndsWithForcedLineBreak;
    auto inlineContentEnding =
      result.IsContentful ? InlineFormattingUtils::InlineContentEnding(result) : null;
    auto contentLogicalLeft = InlineFormattingUtils::HorizontalAlignmentOffset(
      rootStyle, result.ContentLogicalRight, _lineLogicalRect.Width(), result.HangingTrailingContentWidth,
      isLastLineOrLineEndsWithForcedLineBreak);

    LineLayoutResult lineLayoutResult;
    lineLayoutResult.ItemRange = {lineInput.NeedsLayoutRange.Start, placedContentEnd};
    lineLayoutResult.Runs = krys::move(result.Runs);
    lineLayoutResult.Content = {contentLogicalLeft, result.ContentLogicalWidth,
                                contentLogicalLeft + result.ContentLogicalRight,
                                _overflowContentLogicalWidth};
    lineLayoutResult.Geometry = {_lineLogicalRect.TopLeft(), _lineLogicalRect.Width(),
                                 _lineLogicalRect.Left()};
    lineLayoutResult.Hanging = {!result.IsHangingTrailingContentWhitespace,
                                result.HangingTrailingContentWidth};
    lineLayoutResult.FirstLast = {isFirstFormattedLineCandidate && inlineContentEnding.has_value()
                                    ? IsFirstFormattedLine::Yes
                                    : IsFirstFormattedLine::No,
                                  isLastInlineContent};
    lineLayoutResult.ContentEnding = inlineContentEnding;
    lineLayoutResult.TrimmedTrailingWhitespaceWidth = _trimmedTrailingWhitespaceWidth;
    return lineLayoutResult;
  }

  void TextOnlySimpleLineBuilder::Initialize(const InlineItemRange &layoutRange,
                                             const InlineRect &initialLogicalRect,
                                             const Maybe<PreviousLine> &previousLine,
                                             bool isFirstFormattedLineCandidate) noexcept
  {
    Reset();

    krys_debug_assert(!layoutRange.IsEmpty() || (previousLine && !previousLine->SuspendedFloats.empty()));
    auto partialLeadingTextItem = [&]() -> Maybe<InlineTextItem>
    {
      if (!previousLine || !layoutRange.Start.Offset)
      {
        return {};
      }

      auto &overflowingInlineTextItem = Downcast<InlineTextItem>(_inlineItemList[layoutRange.Start.Index]);
      krys_debug_assert(layoutRange.Start.Offset < overflowingInlineTextItem.Length());
      auto overflowingLength = overflowingInlineTextItem.Length() - layoutRange.Start.Offset;
      if (overflowingLength)
      {
        // Turn previous line's overflow content into the next line's leading content.
        // "sp[<-line break->]lit_content" -> break position: 2 -> leading partial content length: 11.
        return overflowingInlineTextItem.Right(static_cast<uint32>(overflowingLength),
                                               previousLine->TrailingOverflowingContentWidth);
      }
      return {};
    };
    _isFirstFormattedLineCandidate = isFirstFormattedLineCandidate;
    _partialLeadingTextItem = partialLeadingTextItem();
    _line.Initialize({}, isFirstFormattedLineCandidate);
    _previousLine = previousLine;
    _lineLogicalRect = initialLogicalRect;
    _trimmedTrailingWhitespaceWidth = {};
    _overflowContentLogicalWidth = {};
  }

  Maybe<LineLayoutResult> TextOnlySimpleLineBuilder::PlaceSingleCharacterContentIfApplicable(
    const LineInput &lineInput, bool isFirstFormattedLineCandidate) noexcept
  {
    if (_inlineItemList.size() != 1)
    {
      return {};
    }
    auto *inlineTextItem = DynamicDowncast<InlineTextItem>(_inlineItemList[0]);
    if (!inlineTextItem || inlineTextItem->IsWhitespace())
    {
      return {};
    }
    // A single character is a single code point here.
    auto content = inlineTextItem->Content();
    if (content.size() > 1
        && text::DecodeUTF8(Span<const char8>(content.data(), content.size())).Length != content.size())
    {
      return {};
    }
    krys_debug_assert(lineInput.NeedsLayoutRange.Start.Index + 1 == lineInput.NeedsLayoutRange.End.Index
                      && !lineInput.NeedsLayoutRange.Start.Offset && !lineInput.NeedsLayoutRange.End.Offset);

    auto lineRect = lineInput.InitialLogicalRect;
    auto contentWidth = inlineTextItem->Width().value_or(0.f);
    Line::RunList singleRun;
    singleRun.push_back(Line::Run(*inlineTextItem, inlineTextItem->Style(), {}, contentWidth));

    auto contentLeft =
      InlineFormattingUtils::HorizontalAlignmentOffset(RootStyle(), contentWidth, lineRect.Width(), {}, true);
    auto contentRight = contentLeft + contentWidth;

    LineLayoutResult lineLayoutResult;
    lineLayoutResult.ItemRange = lineInput.NeedsLayoutRange;
    lineLayoutResult.Runs = krys::move(singleRun);
    lineLayoutResult.Content = {contentLeft, contentWidth, contentRight,
                                std::max(0.f, contentRight - lineRect.Right())};
    lineLayoutResult.Geometry = {lineRect.TopLeft(), lineRect.Width(), lineRect.Left()};
    lineLayoutResult.FirstLast = {
      isFirstFormattedLineCandidate ? IsFirstFormattedLine::Yes : IsFirstFormattedLine::No, true};
    lineLayoutResult.ContentEnding = LineLayoutResult::InlineContentEnding::Generic;
    return lineLayoutResult;
  }

  InlineItemPosition
    TextOnlySimpleLineBuilder::PlaceInlineTextContent(const RenderStyle &rootStyle,
                                                      const InlineItemRange &layoutRange) noexcept
  {
    auto hasWrapOpportunityBeforeWhitespace =
      rootStyle.GetWhiteSpaceCollapse() != WhiteSpaceCollapse::BreakSpaces
      && rootStyle.GetLineBreak() != LineBreak::AfterWhiteSpace;
    size_t placedInlineItemCount = 0;

    auto result = TextOnlyLineBreakResult {};
    auto candidateContent = CandidateTextContent {layoutRange.StartIndex(), layoutRange.StartIndex(), {}};

    auto nextItemIndex = layoutRange.StartIndex();
    auto isAtSoftWrapOpportunityOrContentEnd = [&](const InlineTextItem &inlineTextItem)
    {
      if (inlineTextItem.IsWhitespace())
      {
        return true;
      }
      if (nextItemIndex >= layoutRange.EndIndex() || _inlineItemList[nextItemIndex].IsLineBreak())
      {
        return true;
      }
      auto &nextInlineTextItem = Downcast<InlineTextItem>(_inlineItemList[nextItemIndex]);
      if (nextInlineTextItem.IsWhitespace())
      {
        return hasWrapOpportunityBeforeWhitespace;
      }
      return &inlineTextItem.GetInlineTextBox() == &nextInlineTextItem.GetInlineTextBox()
             || TextUtil::MayBreakInBetween(inlineTextItem, nextInlineTextItem);
    };

    auto processCandidateContent = [&]
    {
      result = CommitCandidateContent(rootStyle, candidateContent, layoutRange);
      placedInlineItemCount =
        !result.IsRevert ? placedInlineItemCount + result.CommittedCount : result.CommittedCount;
      candidateContent = {candidateContent.EndIndex, candidateContent.EndIndex, {}};
      return result.EndOfLine == InlineContentBreaker::IsEndOfLine::Yes;
    };

    // Handle leading partial content first (overflowing text from the previous line).
    auto isEndOfLine = false;
    if (_partialLeadingTextItem)
    {
      candidateContent.Append({});
      ++nextItemIndex;
      if (isAtSoftWrapOpportunityOrContentEnd(*_partialLeadingTextItem))
      {
        isEndOfLine = processCandidateContent();
      }
    }

    while (!isEndOfLine && nextItemIndex < layoutRange.EndIndex())
    {
      auto &inlineItem = _inlineItemList[nextItemIndex++];
      krys_debug_assert(inlineItem.IsText() || inlineItem.IsLineBreak());

      if (auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItem))
      {
        auto contentWidth = [&]
        {
          if (auto logicalWidth = inlineTextItem->Width())
          {
            return *logicalWidth;
          }
          return MeasuredInlineTextItem(*inlineTextItem, rootStyle,
                                        _line.ContentLogicalRight() + candidateContent.LogicalWidth);
        };
        candidateContent.Append(contentWidth());
        if (isAtSoftWrapOpportunityOrContentEnd(*inlineTextItem))
        {
          isEndOfLine = processCandidateContent();
        }
        continue;
      }
      if (inlineItem.IsLineBreak())
      {
        isEndOfLine = true;
        result = {};
        continue;
      }
    }
    if (ConsumeTrailingLineBreakIfApplicable(result, layoutRange.StartIndex() + placedInlineItemCount,
                                             layoutRange.EndIndex(), _line, _inlineItemList))
    {
      ++placedInlineItemCount;
    }
    krys_debug_assert(placedInlineItemCount);
    auto placedContentEnd = PlacedInlineItemEnd(layoutRange.StartIndex(), placedInlineItemCount,
                                                result.OverflowingContentLength, _inlineItemList);
    HandleLineEnding(rootStyle, placedContentEnd, layoutRange.EndIndex());
    _overflowContentLogicalWidth = result.OverflowLogicalWidth;
    return placedContentEnd;
  }

  InlineItemPosition
    TextOnlySimpleLineBuilder::PlaceNonWrappingInlineTextContent(const RenderStyle &rootStyle,
                                                                 const InlineItemRange &layoutRange) noexcept
  {
    krys_debug_assert(!TextUtil::IsWrappingAllowed(rootStyle));
    krys_debug_assert(!_partialLeadingTextItem);

    auto candidateContent = CandidateTextContent {layoutRange.StartIndex(), layoutRange.StartIndex(), {}};
    auto isEndOfLine = false;
    auto trailingLineBreakIndex = Maybe<size_t> {};
    auto nextItemIndex = layoutRange.StartIndex();

    while (!isEndOfLine)
    {
      auto &inlineItem = _inlineItemList[nextItemIndex];
      if (auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItem))
      {
        auto contentWidth = [&]
        {
          if (auto logicalWidth = inlineTextItem->Width())
          {
            return *logicalWidth;
          }
          return MeasuredInlineTextItem(*inlineTextItem, rootStyle, candidateContent.LogicalWidth);
        };
        candidateContent.Append(contentWidth());
      }
      else if (inlineItem.IsLineBreak())
      {
        trailingLineBreakIndex = nextItemIndex;
      }
      else
      {
        krys_debug_assert(false);
        return layoutRange.End;
      }
      ++nextItemIndex;
      isEndOfLine = nextItemIndex >= layoutRange.EndIndex() || trailingLineBreakIndex.has_value();
    }

    if (trailingLineBreakIndex && candidateContent.StartIndex == candidateContent.EndIndex)
    {
      _line.AppendLineBreak(_inlineItemList[*trailingLineBreakIndex],
                            _inlineItemList[*trailingLineBreakIndex].Style());
      return {*trailingLineBreakIndex + 1, {}};
    }

    auto result = CommitCandidateContent(rootStyle, candidateContent, layoutRange);
    nextItemIndex = layoutRange.StartIndex() + result.CommittedCount;
    if (ConsumeTrailingLineBreakIfApplicable(result, nextItemIndex, layoutRange.EndIndex(), _line,
                                             _inlineItemList))
    {
      ++nextItemIndex;
    }

    auto placedInlineItemCount = nextItemIndex - layoutRange.StartIndex();
    auto placedContentEnd = PlacedInlineItemEnd(layoutRange.StartIndex(), placedInlineItemCount,
                                                result.OverflowingContentLength, _inlineItemList);
    HandleLineEnding(rootStyle, placedContentEnd, layoutRange.EndIndex());
    return placedContentEnd;
  }

  TextOnlyLineBreakResult
    TextOnlySimpleLineBuilder::CommitCandidateContent(const RenderStyle &rootStyle,
                                                      const CandidateTextContent &candidateContent,
                                                      const InlineItemRange &layoutRange) noexcept
  {
    auto hasLeadingPartiaContent =
      _partialLeadingTextItem && candidateContent.StartIndex == layoutRange.StartIndex();
    auto contentWidth = [&](const InlineTextItem &inlineTextItem, InlineLayoutUnit contentOffset)
    {
      if (auto logicalWidth = inlineTextItem.Width())
      {
        return *logicalWidth;
      }
      return MeasuredInlineTextItem(inlineTextItem, rootStyle, _line.ContentLogicalRight() + contentOffset);
    };

    if (candidateContent.LogicalWidth <= AvailableWidth() && !hasLeadingPartiaContent)
    {
      for (auto index = candidateContent.StartIndex; index < candidateContent.EndIndex; ++index)
      {
        auto &inlineTextItem = Downcast<InlineTextItem>(_inlineItemList[index]);
        _line.AppendTextFast(inlineTextItem, rootStyle, contentWidth(inlineTextItem, {}));
      }

      if (_line.HasContentOrListMarker())
      {
        _wrapOpportunityList.push_back(&_inlineItemList[candidateContent.EndIndex - 1]);
      }
      return {InlineContentBreaker::IsEndOfLine::No, candidateContent.EndIndex - candidateContent.StartIndex};
    }

    auto candidateContentForLineBreaking = InlineContentBreaker::ContinuousContent {};
    auto startIndex = candidateContent.StartIndex;

    if (hasLeadingPartiaContent)
    {
      candidateContentForLineBreaking.AppendTextContent(*_partialLeadingTextItem, rootStyle,
                                                        contentWidth(*_partialLeadingTextItem, {}));
      ++startIndex;
    }
    for (auto index = startIndex; index < candidateContent.EndIndex; ++index)
    {
      auto &inlineTextItem = Downcast<InlineTextItem>(_inlineItemList[index]);
      candidateContentForLineBreaking.AppendTextContent(
        inlineTextItem, rootStyle,
        contentWidth(inlineTextItem, candidateContentForLineBreaking.LogicalWidth()));
    }
    return HandleOverflowingTextContent(rootStyle, candidateContentForLineBreaking, layoutRange);
  }

  TextOnlyLineBreakResult TextOnlySimpleLineBuilder::HandleOverflowingTextContent(
    const RenderStyle &rootStyle, const InlineContentBreaker::ContinuousContent &candidateContent,
    const InlineItemRange &layoutRange) noexcept
  {
    using Action = InlineContentBreaker::Result::Action;
    using IsEndOfLine = InlineContentBreaker::IsEndOfLine;
    krys_debug_assert(!candidateContent.Runs().empty());

    auto availableWidth = AvailableWidth();
    auto lineBreakingResult = InlineContentBreaker::Result {Action::Keep, IsEndOfLine::No, {}, {}};
    if (candidateContent.LogicalWidth() > availableWidth)
    {
      auto lineStatus = InlineContentBreaker::LineStatus {
        _line.ContentLogicalRight(),         availableWidth,
        _line.TrimmableTrailingWidth(),      _line.TrailingSoftHyphenWidth(),
        _line.IsTrailingRunFullyTrimmable(), _line.HasContentOrListMarker(),
        !_wrapOpportunityList.empty()};
      lineBreakingResult = GetInlineContentBreaker().ProcessInlineContent(candidateContent, lineStatus);
    }

    if (lineBreakingResult.ResultAction == Action::Keep)
    {
      auto &committedRuns = candidateContent.Runs();
      for (auto &run : committedRuns)
      {
        _line.AppendTextFast(Downcast<InlineTextItem>(run.Item), run.Style, run.ContentWidth());
      }
      if (_line.HasContentOrListMarker())
      {
        _wrapOpportunityList.push_back(&committedRuns.back().Item);
      }
      return {lineBreakingResult.EndOfLine, committedRuns.size()};
    }

    krys_debug_assert(lineBreakingResult.EndOfLine == IsEndOfLine::Yes);

    if (lineBreakingResult.ResultAction == Action::Wrap)
    {
      return {IsEndOfLine::Yes,
              {},
              {},
              OverflowWidthAsLeadingForNextLine(candidateContent.Runs(), lineBreakingResult)};
    }

    if (lineBreakingResult.ResultAction == Action::WrapWithHyphen)
    {
      krys_debug_assert(_line.TrailingSoftHyphenWidth());
      _line.AddTrailingHyphen(*_line.TrailingSoftHyphenWidth());
      return {IsEndOfLine::Yes};
    }

    if (lineBreakingResult.ResultAction == Action::Break)
    {
      auto processPartialContent = [&]() -> TextOnlyLineBreakResult
      {
        if (!lineBreakingResult.PartialTrailing)
        {
          krys_debug_assert(false);
          return {IsEndOfLine::Yes};
        }
        auto trailingRunIndex = lineBreakingResult.PartialTrailing->TrailingRunIndex;
        auto &runs = candidateContent.Runs();
        for (size_t index = 0; index < trailingRunIndex; ++index)
        {
          auto &run = runs[index];
          _line.AppendTextFast(Downcast<InlineTextItem>(run.Item), run.Style, run.ContentWidth());
        }

        auto committedInlineItemCount = trailingRunIndex + 1;
        auto &trailingRun = runs[trailingRunIndex];
        if (!lineBreakingResult.PartialTrailing->Partial)
        {
          _line.AppendTextFast(Downcast<InlineTextItem>(trailingRun.Item), trailingRun.Style,
                               trailingRun.ContentWidth());
          if (auto hyphenWidth = lineBreakingResult.PartialTrailing->HyphenWidth)
          {
            _line.AddTrailingHyphen(*hyphenWidth);
          }
          return {IsEndOfLine::Yes, committedInlineItemCount};
        }

        auto partialRun = *lineBreakingResult.PartialTrailing->Partial;
        auto &trailingInlineTextItem = Downcast<InlineTextItem>(runs[trailingRunIndex].Item);
        _line.AppendTextFast(trailingInlineTextItem.Left(static_cast<uint32>(partialRun.Length)),
                             trailingRun.Style, partialRun.LogicalWidth);
        if (auto hyphenWidth = partialRun.HyphenWidth)
        {
          _line.AddTrailingHyphen(*hyphenWidth);
        }
        auto overflowingContentLength = trailingInlineTextItem.Length() - partialRun.Length;
        return {IsEndOfLine::Yes, committedInlineItemCount, overflowingContentLength,
                OverflowWidthAsLeadingForNextLine(candidateContent.Runs(), lineBreakingResult)};
      };
      return processPartialContent();
    }

    // Revert to a previous position cases.
    if (_wrapOpportunityList.empty())
    {
      krys_debug_assert(false);
      return {IsEndOfLine::Yes};
    }

    if (lineBreakingResult.ResultAction == Action::RevertToLastWrapOpportunity)
    {
      return {
        IsEndOfLine::Yes,
        RevertToTrailingItem(rootStyle, layoutRange, Downcast<InlineTextItem>(*_wrapOpportunityList.back())),
        {},
        {},
        true};
    }

    if (lineBreakingResult.ResultAction == Action::RevertToLastNonOverflowingWrapOpportunity)
    {
      return {IsEndOfLine::Yes, RevertToLastNonOverflowingItem(rootStyle, layoutRange), {}, {}, true};
    }

    krys_debug_assert(false);
    return {IsEndOfLine::Yes};
  }

  void TextOnlySimpleLineBuilder::HandleLineEnding(const RenderStyle &rootStyle,
                                                   InlineItemPosition placedContentEnd,
                                                   size_t layoutRangeEndIndex) noexcept
  {
    auto horizontalAvailableSpace = _lineLogicalRect.Width();
    auto isLastInlineContent = IsLastLineWithInlineContent(placedContentEnd, layoutRangeEndIndex);
    auto shouldPreserveTrailingWhitespace = [&]
    {
      return rootStyle.GetLineBreak() == LineBreak::AfterWhiteSpace
             && GetIntrinsicWidthMode() != IntrinsicWidthMode::Minimum
             && (!isLastInlineContent || horizontalAvailableSpace < _line.ContentLogicalWidth());
    };
    _trimmedTrailingWhitespaceWidth = _line.HandleTrailingTrimmableContent(
      shouldPreserveTrailingWhitespace() ? Line::TrailingContentAction::Preserve
                                         : Line::TrailingContentAction::Remove);
    if (GetFormattingContext().Quirks().TrailingNonBreakingSpaceNeedsAdjustment(
          IsInIntrinsicWidthMode(), horizontalAvailableSpace < _line.ContentLogicalWidth()))
    {
      _line.HandleOverflowingNonBreakingSpace(shouldPreserveTrailingWhitespace()
                                                ? Line::TrailingContentAction::Preserve
                                                : Line::TrailingContentAction::Remove,
                                              _line.ContentLogicalWidth() - horizontalAvailableSpace);
    }

    _line.HandleTrailingHangingContent(GetIntrinsicWidthMode(), horizontalAvailableSpace,
                                       isLastInlineContent);
  }

  size_t TextOnlySimpleLineBuilder::RevertToTrailingItem(const RenderStyle &rootStyle,
                                                         const InlineItemRange &layoutRange,
                                                         const InlineTextItem &trailingInlineItem) noexcept
  {
    _line.Initialize({}, IsFirstFormattedLineCandidate());
    size_t numberOfInlineItemsOnLine = 0;

    auto appendTextInlineItem = [&](const InlineTextItem &inlineTextItem)
    {
      auto logicalWidth = inlineTextItem.Width()
                            ? *inlineTextItem.Width()
                            : MeasuredInlineTextItem(inlineTextItem, rootStyle, _line.ContentLogicalRight());
      _line.AppendTextFast(inlineTextItem, rootStyle, logicalWidth);
      ++numberOfInlineItemsOnLine;
      return &inlineTextItem == &trailingInlineItem;
    };

    if (_partialLeadingTextItem && appendTextInlineItem(*_partialLeadingTextItem))
    {
      return numberOfInlineItemsOnLine;
    }
    for (size_t index = layoutRange.StartIndex() + numberOfInlineItemsOnLine; index < layoutRange.EndIndex();
         ++index)
    {
      if (appendTextInlineItem(Downcast<InlineTextItem>(_inlineItemList[index])))
      {
        return numberOfInlineItemsOnLine;
      }
    }
    krys_debug_assert(false);
    return {};
  }

  size_t
    TextOnlySimpleLineBuilder::RevertToLastNonOverflowingItem(const RenderStyle &rootStyle,
                                                              const InlineItemRange &layoutRange) noexcept
  {
    // Revert all the way back to a wrap opportunity when either a soft hyphen fits or no hyphen is required.
    for (auto i = _wrapOpportunityList.size(); i--;)
    {
      auto committedCount =
        RevertToTrailingItem(rootStyle, layoutRange, Downcast<InlineTextItem>(*_wrapOpportunityList[i]));
      auto trailingSoftHyphenWidth = _line.TrailingSoftHyphenWidth();

      auto hasRevertedEnough = [&]
      {
        if (!i || !trailingSoftHyphenWidth)
        {
          return true;
        }
        return *trailingSoftHyphenWidth <= AvailableWidth();
      };
      if (hasRevertedEnough())
      {
        if (trailingSoftHyphenWidth)
        {
          _line.AddTrailingHyphen(*trailingSoftHyphenWidth);
        }
        return committedCount;
      }
    }
    krys_debug_assert(false);
    return 0;
  }

  InlineLayoutUnit TextOnlySimpleLineBuilder::AvailableWidth() const noexcept
  {
    auto epsilon = GetIntrinsicWidthMode() == IntrinsicWidthMode::Minimum ? 0.f : LayoutUnit::Epsilon();
    auto contentLogicalRight = _line.ContentLogicalRight();
    return (_lineLogicalRect.Width() + epsilon)
           - (!std::isnan(contentLogicalRight) ? contentLogicalRight : 0.f);
  }

  bool TextOnlySimpleLineBuilder::IsEligibleForSimplifiedTextOnlyInlineLayoutByContent(
    const InlineContentCache::InlineItems &inlineItems, const PlacedFloats &placedFloats) noexcept
  {
    if (inlineItems.IsEmpty())
    {
      return false;
    }
    if (!inlineItems.HasTextAndLineBreakOnlyContent() || inlineItems.HasInlineBoxes()
        || inlineItems.RequiresVisualReordering() || inlineItems.HasTextAutospace())
    {
      return false;
    }
    if (!placedFloats.IsEmpty())
    {
      return false;
    }

    return true;
  }

  bool TextOnlySimpleLineBuilder::IsEligibleForSimplifiedInlineLayoutByStyle(const Box &box) noexcept
  {
    auto isEligibleByStyle = [](const RenderStyle &style)
    {
      if (style.GetFontCascade().WordSpacing())
      {
        return false;
      }
      if (style.GetWritingMode().IsBidiRTL())
      {
        return false;
      }
      if (style.GetWordBreak() == WordBreak::AutoPhrase)
      {
        return false;
      }
      if (style.GetTextIndent() != style::ComputedStyle::InitialTextIndent())
      {
        return false;
      }
      if (style.GetTextAlignLast() == style::TextAlignLast::Justify
          || style.GetTextAlign() == style::TextAlign::Justify
          || style.Display() == DisplayType::RubyAnnotation)
      {
        return false;
      }
      if (style.GetBoxDecorationBreak() == BoxDecorationBreak::Clone)
      {
        return false;
      }
      if (!style.GetHangingPunctuation().IsNone())
      {
        return false;
      }
      if (!style.GetHyphenateLimitLines().IsNoLimit())
      {
        return false;
      }
      if (style.GetTextWrapMode() == TextWrapMode::Wrap
          && (style.GetTextWrapStyle() == TextWrapStyle::Balance
              || style.GetTextWrapStyle() == TextWrapStyle::Pretty))
      {
        return false;
      }
      return true;
    };

    auto &style = box.Style();
    auto &firstLineStyle = box.FirstLineStyle();
    return isEligibleByStyle(style) && (&style == &firstLineStyle || isEligibleByStyle(firstLineStyle));
  }
}
