#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineBuilder.hpp"
#include "Krystal.Booey/Core/UnicodeBidi.hpp"
#include "Krystal.Booey/Graphics/ComplexTextController.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingUtils.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineQuirks.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <ranges>
#include <unicode/ubidi.h>

namespace krys::boo::layout
{
  struct LineContent
  {
    InlineItemRange Range;
    size_t PartialTrailingContentLength {0};
    Maybe<InlineLayoutUnit> OverflowLogicalWidth {};
    HashMap<const Box *, InlineLayoutUnit> RubyBaseAlignmentOffsetList {};
    InlineLayoutUnit RubyAnnotationOffset {0.f};
    enum class LineBreakReason : uint8
    {
      ForcedLineBreakByBlockContent,
      Other
    };
    LineBreakReason BreakReason {LineBreakReason::Other};
  };

  struct LineCandidate
  {
    void Reset() noexcept;

    struct InlineContent
    {
      KRYS_NODISCARD const InlineContentBreaker::ContinuousContent &GetContinuousContent() const noexcept
      {
        return _continuousContent;
      }
      KRYS_NODISCARD InlineContentBreaker::ContinuousContent &GetContinuousContent() noexcept
      {
        return _continuousContent;
      }
      KRYS_NODISCARD const InlineItem *TrailingLineBreak() const noexcept
      {
        return _trailingLineBreak;
      }
      KRYS_NODISCARD const InlineItem *TrailingWordBreakOpportunity() const noexcept
      {
        return _trailingWordBreakOpportunity;
      }

      void AppendInlineItem(const InlineItem &inlineItem, const RenderStyle &style,
                            InlineLayoutUnit logicalWidth,
                            InlineLayoutUnit textSpacingAdjustment = 0) noexcept;
      void Reset() noexcept;
      KRYS_NODISCARD bool IsEmpty() const noexcept
      {
        return _continuousContent.Runs().empty() && !TrailingWordBreakOpportunity() && !TrailingLineBreak();
      }

      void SetHasTrailingSoftWrapOpportunity(bool hasTrailingSoftWrapOpportunity) noexcept
      {
        _hasTrailingSoftWrapOpportunity = hasTrailingSoftWrapOpportunity;
      }
      KRYS_NODISCARD bool HasTrailingSoftWrapOpportunity() const noexcept
      {
        return _hasTrailingSoftWrapOpportunity;
      }

      void SetTrailingSoftHyphenWidth(InlineLayoutUnit hyphenWidth) noexcept
      {
        _continuousContent.SetTrailingSoftHyphenWidth(hyphenWidth);
      }

      void SetHangingContentWidth(InlineLayoutUnit logicalWidth) noexcept
      {
        _continuousContent.SetHangingContentWidth(logicalWidth);
      }

      void SetHasTrailingClonedDecoration(bool hasClonedDecoration) noexcept
      {
        _hasTrailingClonedDecoration = hasClonedDecoration;
      }
      KRYS_NODISCARD bool HasTrailingClonedDecoration() const noexcept
      {
        return _hasTrailingClonedDecoration;
      }

      void SetMinimumRequiredWidth(InlineLayoutUnit minimumRequiredWidth) noexcept
      {
        _continuousContent.SetMinimumRequiredWidth(minimumRequiredWidth);
      }

      KRYS_NODISCARD Maybe<size_t> FirstTextRunIndex() const noexcept
      {
        return _firstTextRunIndex;
      }
      KRYS_NODISCARD Maybe<size_t> LastTextRunIndex() const noexcept
      {
        return _lastTextRunIndex;
      }

      KRYS_NODISCARD bool IsShapingCandidateByContent() const noexcept
      {
        return _hasTextContentSpanningBoxes;
      }

    private:
      InlineContentBreaker::ContinuousContent _continuousContent;
      const InlineItem *_trailingLineBreak {nullptr};
      const InlineItem *_trailingWordBreakOpportunity {nullptr};
      bool _hasTrailingClonedDecoration {false};
      bool _hasTrailingSoftWrapOpportunity {false};
      Maybe<size_t> _firstTextRunIndex {};
      Maybe<size_t> _lastTextRunIndex {};
      Maybe<size_t> _lastInlineBoxIndex {};
      bool _hasTextContentSpanningBoxes {false};
    };

    // Candidate content is a collection of inline content or a float box.
    InlineContent Content;
    const InlineItem *FloatItem {nullptr};
    const InlineItem *BlockItem {nullptr};
  };

  namespace
  {
    KRYS_NODISCARD bool IsContentfulOrHasDecoration(const InlineItem &inlineItem,
                                                    const InlineFormattingContext &formattingContext) noexcept
    {
      if (inlineItem.IsFloat() || inlineItem.IsOpaque())
      {
        return false;
      }
      if (auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItem))
      {
        auto wouldProduceEmptyRun =
          inlineTextItem->IsFullyTrimmable() || inlineTextItem->IsEmpty() || inlineTextItem->IsWordSeparator()
          || inlineTextItem->IsZeroWidthSpaceSeparator() || inlineTextItem->IsQuirkNonBreakingSpace();
        return !wouldProduceEmptyRun;
      }

      if (inlineItem.IsInlineBoxStart())
      {
        return formattingContext.GeometryForBox(inlineItem.LayoutBox()).MarginBorderAndPaddingStart() != 0_lu;
      }
      if (inlineItem.IsInlineBoxEnd())
      {
        return formattingContext.GeometryForBox(inlineItem.LayoutBox()).MarginBorderAndPaddingEnd() != 0_lu;
      }
      return inlineItem.IsAtomicInlineBox() || inlineItem.IsLineBreak();
    }

    KRYS_NODISCARD inline StringBuilder ToString(const Line::RunList &runs) noexcept
    {
      // FIXME: We could try to reuse the content builder in InlineItemsBuilder if this turns out to be a perf
      // bottleneck.
      StringBuilder lineContentBuilder;
      for (auto &run : runs)
      {
        if (!run.IsText())
        {
          continue;
        }
        auto &textContent = run.TextContent();
        lineContentBuilder.AppendSubstring(Downcast<InlineTextBox>(run.LayoutBox()).Content(),
                                           textContent->Start, textContent->Length);
      }
      return lineContentBuilder;
    }

    void ComputedVisualOrder(const Line::RunList &lineRuns, List<int32> &visualOrderList) noexcept
    {
      List<UBiDiLevel> runLevels;
      runLevels.reserve(lineRuns.size());

      List<size_t> runIndexOffsetMap;
      runIndexOffsetMap.reserve(lineRuns.size());
      size_t numberOfOpaqueRuns = 0;
      for (size_t i = 0, accumulatedOffset = 0; i < lineRuns.size(); ++i)
      {
        if (lineRuns[i].BidiLevel() == InlineItem::OpaqueBidiLevel)
        {
          ++accumulatedOffset;
          ++numberOfOpaqueRuns;
          continue;
        }

        // bidiLevels are required to be less than the MAX + 1, otherwise
        // ubidi_reorderVisual will silently fail.
        if (lineRuns[i].BidiLevel() > UBIDI_MAX_EXPLICIT_LEVEL + 1)
        {
          krys_debug_assert(lineRuns[i].BidiLevel() == UBIDI_DEFAULT_LTR);
          continue;
        }

        runLevels.push_back(lineRuns[i].BidiLevel());
        runIndexOffsetMap.push_back(accumulatedOffset);
      }

      auto forceBiDiOnOpaqueLine = [&]
      {
        if (lineRuns.empty() || numberOfOpaqueRuns != lineRuns.size())
        {
          return;
        }
        // When an RTL line has only opaque items (e.g. [spanning inline box start][inline box end] on
        // <span><div></div></span>) we need to set the bidi level on the spanning inline box as if it was
        // contentful to initiate bidi processing (mainly just RTL direction align).
        if (!lineRuns.front().IsLineSpanningInlineBoxStart())
        {
          return;
        }
        runLevels.push_back(static_cast<UBiDiLevel>(
          lineRuns.front().LayoutBox().Parent().GetWritingMode().IsBidiLTR() ? UBIDI_LTR : UBIDI_RTL));
        runIndexOffsetMap.push_back(0);
      };
      forceBiDiOnOpaqueLine();

      visualOrderList.resize(runLevels.size());
      ubidi_reorderVisual(runLevels.data(), static_cast<int32_t>(runLevels.size()), visualOrderList.data());
      if (numberOfOpaqueRuns)
      {
        krys_debug_assert(visualOrderList.size() == runIndexOffsetMap.size());
        for (size_t i = 0; i < runIndexOffsetMap.size(); ++i)
        {
          visualOrderList[i] +=
            static_cast<int32>(runIndexOffsetMap[static_cast<size_t>(visualOrderList[i])]);
        }
      }
    }

    KRYS_NODISCARD bool HasTrailingSoftWrapOpportunity(size_t softWrapOpportunityIndex, size_t layoutRangeEnd,
                                                       Span<const InlineItem> inlineItemList) noexcept
    {
      if (!softWrapOpportunityIndex || softWrapOpportunityIndex == layoutRangeEnd)
      {
        // This candidate inline content ends because the entire content ends and not because there's a soft
        // wrap opportunity.
        return false;
      }
      // See https://www.w3.org/TR/css-text-3/#line-break-details
      auto &trailingInlineItem = inlineItemList[softWrapOpportunityIndex - 1];
      if (trailingInlineItem.IsFloat())
      {
        // While we stop at floats, they are not considered real soft wrap opportunities.
        return false;
      }
      if (trailingInlineItem.IsAtomicInlineBox() || trailingInlineItem.IsLineBreak()
          || trailingInlineItem.IsWordBreakOpportunity() || trailingInlineItem.IsInlineBoxEnd())
      {
        // For Web-compatibility there is a soft wrap opportunity before and after each replaced element or
        // other atomic inline.
        return true;
      }
      if (auto *inlineTextItem = DynamicDowncast<InlineTextItem>(trailingInlineItem))
      {
        if (inlineTextItem->IsWhitespace())
        {
          return true;
        }
        // Now in case of non-whitespace trailing content, we need to check if the actual soft wrap
        // opportunity belongs to the next set. e.g. "this_is_the_trailing_run<span>
        // <-but_this_space_here_is_the_soft_wrap_opportunity" When there's an inline box
        // start(<span>)/end(</span>) between the trailing and the (next)leading run, while we break before
        // the inline box start (<span>) the actual soft wrap position is after the inline box start (<span>)
        // but in terms of line breaking continuity the inline box start (<span>) and the whitespace run
        // belong together.
        krys_assert(layoutRangeEnd <= inlineItemList.size());
        for (auto index = softWrapOpportunityIndex; index < layoutRangeEnd; ++index)
        {
          if (inlineItemList[index].IsInlineBoxStart() || inlineItemList[index].IsInlineBoxEnd()
              || inlineItemList[index].IsOpaque())
          {
            continue;
          }
          // FIXME: Check if [non-whitespace][inline-box][no-whitespace] content has rules about it.
          // For now let's say the soft wrap position belongs to the next set of runs when
          // [non-whitespace][inline-box][whitespace], [non-whitespace][inline-box][box] etc.
          auto *inlineItemListTextItem = DynamicDowncast<InlineTextItem>(inlineItemList[index]);
          return inlineItemListTextItem && !inlineItemListTextItem->IsWhitespace();
        }
        return true;
      }
      if (trailingInlineItem.IsInlineBoxStart())
      {
        // This is a special case when the inline box's first child is a float box.
        return false;
      }
      if (trailingInlineItem.IsOpaque())
      {
        for (auto index = softWrapOpportunityIndex; index--;)
        {
          if (!inlineItemList[index].IsOpaque())
          {
            return HasTrailingSoftWrapOpportunity(index + 1, layoutRangeEnd, inlineItemList);
          }
        }
        krys_debug_assert(inlineItemList[softWrapOpportunityIndex].IsFloat());
        return false;
      }
      krys_debug_assert(false);
      return true;
    }

    KRYS_NODISCARD TextDirection
      InlineBaseDirectionForLineContent(const Line::RunList &runs, const RenderStyle &rootStyle,
                                        const Maybe<PreviousLine> &previousLine) noexcept
    {
      krys_debug_assert(!runs.empty());
      auto shouldUseBlockDirection = rootStyle.GetUnicodeBidi() != UnicodeBidi::Plaintext;
      if (shouldUseBlockDirection)
      {
        return rootStyle.GetWritingMode().BidiDirection();
      }
      // A previous line ending with a line break (<br> or preserved \n) introduces a new unicode paragraph
      // with its own direction.
      if (previousLine && !previousLine->EndsWithLineBreak)
      {
        return previousLine->InlineBaseDirection;
      }
      return TextUtil::DirectionForTextContent(ToString(runs).View());
    }
  }

  void LineCandidate::InlineContent::AppendInlineItem(const InlineItem &inlineItem, const RenderStyle &style,
                                                      InlineLayoutUnit logicalWidth,
                                                      InlineLayoutUnit textSpacingAdjustment) noexcept
  {
    if (inlineItem.IsAtomicInlineBox() || inlineItem.IsOpaque())
    {
      return _continuousContent.Append(inlineItem, style, logicalWidth, textSpacingAdjustment);
    }

    if (inlineItem.IsInlineBoxStartOrEnd())
    {
      auto numberOfRuns = _continuousContent.Runs().size();
      _hasTextContentSpanningBoxes =
        _hasTextContentSpanningBoxes || (_lastTextRunIndex && *_lastTextRunIndex == numberOfRuns - 1);
      _lastInlineBoxIndex = numberOfRuns;
      _continuousContent.Append(inlineItem, style, logicalWidth, textSpacingAdjustment);
      return;
    }

    if (auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItem))
    {
      auto numberOfRuns = _continuousContent.Runs().size();
      _firstTextRunIndex = _firstTextRunIndex.value_or(numberOfRuns);
      _lastTextRunIndex = numberOfRuns;
      _hasTextContentSpanningBoxes =
        _hasTextContentSpanningBoxes || (_lastInlineBoxIndex && *_lastInlineBoxIndex == numberOfRuns - 1);
      return _continuousContent.AppendTextContent(*inlineTextItem, style, logicalWidth);
    }

    if (inlineItem.IsLineBreak())
    {
      _trailingLineBreak = &inlineItem;
      return;
    }

    if (inlineItem.IsWordBreakOpportunity())
    {
      _trailingWordBreakOpportunity = &inlineItem;
      return;
    }

    krys_debug_assert(false);
  }

  void LineCandidate::InlineContent::Reset() noexcept
  {
    _continuousContent.Reset();
    _trailingLineBreak = {};
    _trailingWordBreakOpportunity = {};
    _hasTrailingClonedDecoration = {};
    _hasTrailingSoftWrapOpportunity = {};
    _firstTextRunIndex = {};
    _lastTextRunIndex = {};
    _lastInlineBoxIndex = {};
    _hasTextContentSpanningBoxes = {};
  }

  void LineCandidate::Reset() noexcept
  {
    FloatItem = {};
    BlockItem = {};
    Content.Reset();
  }

  LineBuilder::LineBuilder(InlineFormattingContext &inlineFormattingContext,
                           HorizontalConstraints rootHorizontalConstraints,
                           const InlineItemList &inlineItemList,
                           TextSpacingContext textSpacingContext) noexcept
      : AbstractLineBuilder(inlineFormattingContext, inlineFormattingContext.Root(),
                            rootHorizontalConstraints, inlineItemList),
        _floatingContext(inlineFormattingContext.GetFloatingContext()),
        _textSpacingContext(krys::move(textSpacingContext))
  {
  }

  LineBuilder::~LineBuilder() noexcept = default;

  LineLayoutResult LineBuilder::LayoutInlineContent(const LineInput &lineInput,
                                                    const Maybe<PreviousLine> &previousLine,
                                                    bool isFirstFormattedLineCandidate) noexcept
  {
    Initialize(lineInput.InitialLogicalRect, lineInput.NeedsLayoutRange, previousLine,
               isFirstFormattedLineCandidate);
    auto lineContent = PlaceInlineAndFloatContent(lineInput.NeedsLayoutRange);
    auto result = _line.Close();
    auto inlineContentEnding =
      result.IsContentful ? InlineFormattingUtils::InlineContentEnding(result) : null;

    LineLayoutResult lineLayoutResult;
    lineLayoutResult.ItemRange = lineContent->Range;
    if (IsInIntrinsicWidthMode())
    {
      lineLayoutResult.Runs = krys::move(result.Runs);
      lineLayoutResult.Floats = {krys::move(_placedFloats), krys::move(_suspendedFloats), {}};
      lineLayoutResult.Content = {{}, result.ContentLogicalWidth, {}, lineContent->OverflowLogicalWidth};
      lineLayoutResult.Geometry = {_lineLogicalRect.TopLeft()};
      lineLayoutResult.FirstLast = {isFirstFormattedLineCandidate && inlineContentEnding.has_value()
                                      ? IsFirstFormattedLine::Yes
                                      : IsFirstFormattedLine::No,
                                    {}};
      lineLayoutResult.ContentEnding = inlineContentEnding;
      return lineLayoutResult;
    }

    auto isLastInlineContent =
      IsLastLineWithInlineContent(*lineContent, lineInput.NeedsLayoutRange.EndIndex(), result.Runs);
    // Lines with nothing but content trailing out-of-flow boxes should also be considered last line for
    // alignment e.g. <div style="text-align-last: center">last line<br><div style="display: inline; position:
    // absolute"></div></div> Both the inline content ('last line') and the trailing out-of-flow box are
    // supposed to be center aligned.
    auto shouldTreatAsLastLine =
      isLastInlineContent || lineContent->Range.EndIndex() == lineInput.NeedsLayoutRange.EndIndex();
    auto inlineBaseDirection = !result.Runs.empty()
                                 ? InlineBaseDirectionForLineContent(result.Runs, RootStyle(), _previousLine)
                                 : TextDirection::LTR;
    auto lineEndsWithForcedLineBreak =
      lineContent->BreakReason == LineContent::LineBreakReason::ForcedLineBreakByBlockContent
      || Line::HasTrailingForcedLineBreak(result.Runs);
    auto isLastLineOrLineEndsWithForcedLineBreak = shouldTreatAsLastLine || lineEndsWithForcedLineBreak;
    auto contentLogicalLeft =
      !result.Runs.empty()
        ? InlineFormattingUtils::HorizontalAlignmentOffset(
            RootStyle(), result.ContentLogicalRight, _lineLogicalRect.Width(),
            result.HangingTrailingContentWidth, isLastLineOrLineEndsWithForcedLineBreak, inlineBaseDirection)
        : 0.f;
    List<int32> visualOrderList;
    if (result.ContentNeedsBidiReordering)
    {
      ComputedVisualOrder(result.Runs, visualOrderList);
    }

    lineLayoutResult.Runs = krys::move(result.Runs);
    lineLayoutResult.Floats = {krys::move(_placedFloats), krys::move(_suspendedFloats),
                               _lineIsConstrainedByFloat};
    lineLayoutResult.Content = {contentLogicalLeft, result.ContentLogicalWidth,
                                contentLogicalLeft + result.ContentLogicalRight,
                                lineContent->OverflowLogicalWidth};
    lineLayoutResult.Geometry = {_lineLogicalRect.TopLeft(), _lineLogicalRect.Width(),
                                 _lineInitialLogicalRect.Left(), _initialIntrusiveFloatsWidth,
                                 _initialLetterClearGap};
    lineLayoutResult.Hanging = {!result.IsHangingTrailingContentWhitespace,
                                result.HangingTrailingContentWidth, result.HangablePunctuationStartWidth};
    lineLayoutResult.BidiDirectionality = {krys::move(visualOrderList), inlineBaseDirection};
    lineLayoutResult.FirstLast = {isFirstFormattedLineCandidate && inlineContentEnding.has_value()
                                    ? IsFirstFormattedLine::Yes
                                    : IsFirstFormattedLine::No,
                                  isLastInlineContent};
    lineLayoutResult.RubyAlignment = {krys::move(lineContent->RubyBaseAlignmentOffsetList),
                                      lineContent->RubyAnnotationOffset};
    lineLayoutResult.ContentEnding = inlineContentEnding;
    lineLayoutResult.NonSpanningInlineLevelBoxCount = result.NonSpanningInlineLevelBoxCount;
    if (lineContent->Range.IsEmpty())
    {
      lineLayoutResult.HintForNextLineTopToAvoidIntrusiveFloat =
        _lineLogicalRect.Top() + _candidateContentMaximumHeight;
    }
    return lineLayoutResult;
  }

  void LineBuilder::Initialize(const InlineRect &initialLineLogicalRect,
                               const InlineItemRange &needsLayoutRange,
                               const Maybe<PreviousLine> &previousLine,
                               bool isFirstFormattedLineCandidate) noexcept
  {
    krys_debug_assert(!needsLayoutRange.IsEmpty()
                      || (previousLine && !previousLine->SuspendedFloats.empty()));
    Reset();

    _previousLine = previousLine;
    _isFirstFormattedLineCandidate = isFirstFormattedLineCandidate;
    _placedFloats.clear();
    _suspendedFloats.clear();
    _lineSpanningInlineBoxes.clear();
    _overflowingLogicalWidth = {};
    _partialLeadingTextItem = {};
    _initialLetterClearGap = {};
    _candidateContentMaximumHeight = {};
    GetInlineContentBreaker().SetHyphenationDisabled(GetLayoutState().IsHyphenationDisabled());

    auto createLineSpanningInlineBoxes = [&]
    {
      auto isRootLayoutBox = [&](const ElementBox &elementBox)
      {
        return &elementBox == &Root();
      };
      if (needsLayoutRange.IsEmpty())
      {
        return;
      }
      // An inline box may not necessarily start on the current line:
      // <span>first line<br>second line<span>with some more embedding<br> forth line</span></span>
      // We need to make sure that there's an [InlineBoxStart] for every inline box that's present on the
      // current line. We only have to do it on the first run as any subsequent inline content is either at
      // the same/higher nesting level.
      auto &firstInlineItem = _inlineItemList[needsLayoutRange.StartIndex()];
      // If the parent is the formatting root, we can stop here. This is root inline box content, there's no
      // nesting inline box from the previous line(s) unless the inline box closing is forced over to the
      // current line. e.g. <span>normally the inline box closing forms a continuous content</span>
      // <span>unless it's forced to the next line<br></span>
      auto &firstLayoutBox = firstInlineItem.LayoutBox();
      auto hasLeadingInlineBoxEnd = firstInlineItem.IsInlineBoxEnd();

      if (!hasLeadingInlineBoxEnd)
      {
        if (isRootLayoutBox(firstLayoutBox.Parent()))
        {
          return;
        }

        if (isRootLayoutBox(firstLayoutBox.Parent().Parent()))
        {
          // In many cases the entire content is wrapped inside a single inline box.
          // e.g. <div><span>wall of text with<br>single, line spanning inline box...</span></div>
          krys_debug_assert(firstLayoutBox.Parent().IsInlineBox());
          _lineSpanningInlineBoxes.push_back(InlineItem {
            firstLayoutBox.Parent(), InlineItem::Type::InlineBoxStart, InlineItem::OpaqueBidiLevel});
          return;
        }
      }

      SmallList<const Box *, 2> spanningLayoutBoxList;
      if (hasLeadingInlineBoxEnd)
      {
        spanningLayoutBoxList.push_back(&firstLayoutBox);
      }

      auto *ancestor = &firstInlineItem.LayoutBox().Parent();
      while (!isRootLayoutBox(*ancestor))
      {
        spanningLayoutBoxList.push_back(ancestor);
        ancestor = &ancestor->Parent();
      }
      // Let's treat these spanning inline items as opaque bidi content. They should not change the bidi
      // levels on adjacent content.
      for (auto *spanningInlineBox : spanningLayoutBoxList | std::views::reverse)
      {
        _lineSpanningInlineBoxes.push_back(
          InlineItem {*spanningInlineBox, InlineItem::Type::InlineBoxStart, InlineItem::OpaqueBidiLevel});
      }
    };
    createLineSpanningInlineBoxes();
    _line.Initialize(_lineSpanningInlineBoxes, isFirstFormattedLineCandidate);

    _lineInitialLogicalRect = initialLineLogicalRect;
    auto previousLineEndsWithLineBreak =
      previousLine ? Maybe<InlineFormattingUtils::
                             LineEndsWithLineBreak> {previousLine->EndsWithLineBreak
                                                       ? InlineFormattingUtils::LineEndsWithLineBreak::Yes
                                                       : InlineFormattingUtils::LineEndsWithLineBreak::No}
                   : null;
    _lineMarginStart = GetFormattingContext().FormattingUtils().ComputedTextIndent(
      IsInIntrinsicWidthMode() ? InlineFormattingUtils::IsIntrinsicWidthMode::Yes
                               : InlineFormattingUtils::IsIntrinsicWidthMode::No,
      isFirstFormattedLineCandidate ? IsFirstFormattedLine::Yes : IsFirstFormattedLine::No,
      previousLineEndsWithLineBreak, initialLineLogicalRect.Width());

    auto constraints = FloatAvoidingRect(initialLineLogicalRect, {});
    _lineLogicalRect = constraints.LogicalRect;
    _lineIsConstrainedByFloat = constraints.ConstrainedSideSet;
    // This is by how much intrusive floats (coming from parent/sibling FCs) initially offset the line.
    _initialIntrusiveFloatsWidth = _lineLogicalRect.Left() - initialLineLogicalRect.Left();
    _lineLogicalRect.MoveHorizontally(_lineMarginStart);
    // While negative margins normally don't expand the available space, preferred width computation gets
    // confused by negative text-indent (shrink the space needed for the content) which we have to balance it
    // here.
    _lineLogicalRect.ExpandHorizontally(-_lineMarginStart);

    auto initializeLeadingContentFromOverflow = [&]
    {
      if (!previousLine || !needsLayoutRange.Start.Offset)
      {
        return;
      }
      auto overflowingInlineItemPosition = needsLayoutRange.Start;
      if (auto *overflowingInlineTextItem =
            DynamicDowncast<InlineTextItem>(_inlineItemList[overflowingInlineItemPosition.Index]))
      {
        krys_debug_assert(overflowingInlineItemPosition.Offset < overflowingInlineTextItem->Length());
        auto overflowingLength = overflowingInlineTextItem->Length() - overflowingInlineItemPosition.Offset;
        if (overflowingLength)
        {
          // Turn previous line's overflow content into the next line's leading content.
          // "sp[<-line break->]lit_content" -> break position: 2 -> leading partial content length: 11.
          _partialLeadingTextItem = overflowingInlineTextItem->Right(
            static_cast<uint32>(overflowingLength), previousLine->TrailingOverflowingContentWidth);
          return;
        }
      }
      _overflowingLogicalWidth = previousLine->TrailingOverflowingContentWidth;
    };
    initializeLeadingContentFromOverflow();
  }

  UniquePtr<LineContent>
    LineBuilder::PlaceInlineAndFloatContent(const InlineItemRange &needsLayoutRange) noexcept
  {
    size_t resumedFloatCount = 0;
    auto layoutPreviouslySuspendedFloats = [&]
    {
      if (!_previousLine)
      {
        return true;
      }
      // FIXME: Note that placedInlineItemCount is not incremented here as these floats are already accounted
      // for (at previous line) as LineContent only takes one range -meaning that inline layout may continue
      // while float layout is being suspended and the placed InlineItem range ends at the last inline item
      // placed on the current line.
      for (size_t index = 0; index < _previousLine->SuspendedFloats.size(); ++index)
      {
        auto &suspendedFloat = *_previousLine->SuspendedFloats[index];
        auto isPlaced = TryPlacingFloatBox(
          suspendedFloat, !index ? MayOverConstrainLine::OnlyWhenFirstFloatOnLine : MayOverConstrainLine::No);
        if (!isPlaced)
        {
          // Can't place more floats here. We'll try to place these floats on subsequent lines.
          for (; index < _previousLine->SuspendedFloats.size(); ++index)
          {
            _suspendedFloats.push_back(_previousLine->SuspendedFloats[index]);
          }
          return false;
        }
        ++resumedFloatCount;
      }
      _previousLine->SuspendedFloats.clear();
      return true;
    };

    auto lineContent = CreateUnique<LineContent>();

    if (!layoutPreviouslySuspendedFloats())
    {
      // Couldn't even manage to place all suspended floats from previous line(s). -which also means we can't
      // fit any inline content at this vertical position.
      lineContent->Range = {needsLayoutRange.Start, needsLayoutRange.Start};
      _candidateContentMaximumHeight = _lineLogicalRect.Height();
      return lineContent;
    }

    size_t placedInlineItemCount = 0;

    auto layoutInlineAndFloatContent = [&]
    {
      auto lineCandidate = CreateUnique<LineCandidate>();

      auto currentItemIndex = needsLayoutRange.StartIndex();
      while (currentItemIndex < needsLayoutRange.EndIndex())
      {
        // 1. Collect the set of runs that we can commit to the line as one entity e.g.
        // <span>text_and_span_start_span_end</span>.
        // 2. Apply floats and shrink the available horizontal space e.g. <span>intru_<div style="float:
        // left"></div>sive_float</span>.
        // 3. Check if the content fits the line and commit the content accordingly (full, partial or not
        // commit at all).
        // 4. Return if we are at the end of the line either by not being able to fit more content or because
        // of an explicit line break.
        auto candidateStartEndIndex = std::pair<size_t, size_t> {
          currentItemIndex, GetFormattingContext().FormattingUtils().NextWrapOpportunity(
                              currentItemIndex, needsLayoutRange, _inlineItemList)};
        CandidateContentForLine(*lineCandidate, candidateStartEndIndex, needsLayoutRange,
                                _line.ContentLogicalRight());
        // Now check if we can put this content on the current line.
        if (auto *floatItem = lineCandidate->FloatItem)
        {
          krys_debug_assert(lineCandidate->Content.IsEmpty());
          if (!TryPlacingFloatBox(floatItem->LayoutBox(), _line.Runs().empty() ? MayOverConstrainLine::Yes
                                                                               : MayOverConstrainLine::No))
          {
            // This float overconstrains the line (it simply means shrinking the line box by the float would
            // cause inline content overflow.) At this point we suspend float layout but continue with inline
            // layout. Such suspended float will be placed at the next available vertical positon when this
            // line "closes".
            _suspendedFloats.push_back(&floatItem->LayoutBox());
          }
          ++placedInlineItemCount;
        }
        else if (auto *blockItem = lineCandidate->BlockItem)
        {
          // We need to break whenever we come across a block level block to ensure it's the only item on the
          // line. This is unlike hard line break as in case of 'text<br>', hard line break stays on the
          // current line.
          if (placedInlineItemCount)
          {
            lineContent->BreakReason = LineContent::LineBreakReason::ForcedLineBreakByBlockContent;
            return;
          }

          krys_debug_assert(lineCandidate->Content.IsEmpty());
          HandleBlockContent(*blockItem);
          ++placedInlineItemCount;
          // It's always end of line before/after a block level box.
          return;
        }
        else
        {
          auto result = HandleInlineContent(needsLayoutRange, *lineCandidate);
          auto isEndOfLine = result.EndOfLine == InlineContentBreaker::IsEndOfLine::Yes;
          if (!result.CommittedCount.IsRevert)
          {
            placedInlineItemCount += result.CommittedCount.Value;
            auto &inlineContent = lineCandidate->Content;
            auto inlineContentIsFullyPlaced =
              inlineContent.GetContinuousContent().Runs().size() == result.CommittedCount.Value
              && !result.PartialTrailingContentLength;
            if (inlineContentIsFullyPlaced)
            {
              if (auto *wordBreakOpportunity = inlineContent.TrailingWordBreakOpportunity())
              {
                // <wbr> needs to be on the line as an empty run so that we can construct an inline box and
                // compute basic geometry.
                ++placedInlineItemCount;
                _line.AppendWordBreakOpportunity(*wordBreakOpportunity, wordBreakOpportunity->Style());
              }
              if (inlineContent.TrailingLineBreak())
              {
                // Fully placed (or empty) content followed by a line break means "end of line".
                // FIXME: This will put the line break box at the end of the line while in case of some inline
                // boxes, the line break could very well be at an earlier position. This has no visual
                // implications at this point though (only geometry correctness on the line break box). e.g.
                // <span style="border-right: 10px solid green">text<br></span> where the <br>'s horizontal
                // position is before the right border and not after.
                auto &trailingLineBreak = *inlineContent.TrailingLineBreak();
                _line.AppendLineBreak(trailingLineBreak, trailingLineBreak.Style());
                if (trailingLineBreak.BidiLevel() != UBIDI_DEFAULT_LTR)
                {
                  _line.SetContentNeedsBidiReordering();
                }
                ++placedInlineItemCount;
                isEndOfLine = true;
              }
            }
          }
          else
          {
            placedInlineItemCount = result.CommittedCount.Value;
          }

          if (isEndOfLine)
          {
            lineContent->PartialTrailingContentLength = result.PartialTrailingContentLength;
            lineContent->OverflowLogicalWidth = result.OverflowLogicalWidth;
            return;
          }
        }
        currentItemIndex = needsLayoutRange.StartIndex() + placedInlineItemCount;
      }
      // Looks like we've run out of content.
      krys_debug_assert(placedInlineItemCount || resumedFloatCount);
    };
    layoutInlineAndFloatContent();

    auto computePlacedInlineItemRange = [&]
    {
      lineContent->Range = {needsLayoutRange.Start, needsLayoutRange.Start};

      if (!placedInlineItemCount)
      {
        return;
      }

      // Layout range already includes "suspended" floats from previous line(s). See
      // layoutPreviouslySuspendedFloats above for details.
      krys_debug_assert(_placedFloats.size() >= resumedFloatCount);
      auto onlyFloatContentPlaced = placedInlineItemCount == _placedFloats.size() - resumedFloatCount;
      if (onlyFloatContentPlaced || !lineContent->PartialTrailingContentLength)
      {
        lineContent->Range.End = {needsLayoutRange.StartIndex() + placedInlineItemCount, {}};
        return;
      }

      auto trailingInlineItemIndex = needsLayoutRange.StartIndex() + placedInlineItemCount - 1;
      size_t overflowingInlineTextItemLength =
        Downcast<InlineTextItem>(_inlineItemList[trailingInlineItemIndex]).Length();
      krys_debug_assert(lineContent->PartialTrailingContentLength
                        && lineContent->PartialTrailingContentLength < overflowingInlineTextItemLength);
      lineContent->Range.End = {trailingInlineItemIndex,
                                overflowingInlineTextItemLength - lineContent->PartialTrailingContentLength};
    };
    computePlacedInlineItemRange();

    krys_debug_assert(lineContent->Range.EndIndex() <= needsLayoutRange.EndIndex());

    auto handleLineEnding = [&]
    {
      auto isLastInlineContent =
        IsLastLineWithInlineContent(*lineContent, needsLayoutRange.EndIndex(), _line.Runs());
      auto horizontalAvailableSpace = _lineLogicalRect.Width();
      auto &rootStyle = RootStyle();

      auto handleTrailingContent = [&]
      {
        auto &quirks = GetFormattingContext().Quirks();
        auto lineHasOverflow = [&]
        {
          return horizontalAvailableSpace < _line.ContentLogicalWidth() && _line.HasContentOrListMarker();
        };
        auto isLineBreakAfterWhitespace = [&]
        {
          return rootStyle.GetLineBreak() == LineBreak::AfterWhiteSpace
                 && GetIntrinsicWidthMode() != IntrinsicWidthMode::Minimum
                 && (!isLastInlineContent || lineHasOverflow());
        };
        _line.HandleTrailingTrimmableContent(isLineBreakAfterWhitespace()
                                               ? Line::TrailingContentAction::Preserve
                                               : Line::TrailingContentAction::Remove);
        if (quirks.TrailingNonBreakingSpaceNeedsAdjustment(IsInIntrinsicWidthMode(), lineHasOverflow()))
        {
          _line.HandleOverflowingNonBreakingSpace(isLineBreakAfterWhitespace()
                                                    ? Line::TrailingContentAction::Preserve
                                                    : Line::TrailingContentAction::Remove,
                                                  _line.ContentLogicalWidth() - horizontalAvailableSpace);
        }

        _line.HandleTrailingHangingContent(GetIntrinsicWidthMode(), horizontalAvailableSpace,
                                           isLastInlineContent);

        auto mayNeedOutOfFlowOverflowTrimming = !IsInIntrinsicWidthMode() && lineHasOverflow()
                                                && !lineContent->PartialTrailingContentLength
                                                && TextUtil::IsWrappingAllowed(rootStyle);
        if (mayNeedOutOfFlowOverflowTrimming)
        {
          // Overflowing out-of-flow boxes should wrap the to subsequent lines just like any other in-flow
          // content. However since we take a shortcut by not considering out-of-flow content as inflow but
          // instead treating it as an opaque box with zero width and no soft wrap opportunity, any
          // overflowing out-of-flow content would pile up as trailing content. Alternatively we could
          // initiate a two pass layout first with out-of-flow content treated as true inflow and a second
          // without them.
          krys_debug_assert(!lineContent->Range.End.Offset);
          if (auto *lastRemovedTrailingBox = _line.RemoveOverflowingOutOfFlowContent())
          {
            auto lineEndIndex = [&]
            {
              for (auto index = lineContent->Range.Start.Index; index < lineContent->Range.End.Index; ++index)
              {
                if (&_inlineItemList[index].LayoutBox() == lastRemovedTrailingBox)
                {
                  return index;
                }
              }
              krys_debug_assert(false);
              return lineContent->Range.End.Index;
            };
            lineContent->Range.End.Index = lineEndIndex();
          }
        }
      };
      handleTrailingContent();

      // On each line, reset the embedding level of any sequence of whitespace characters at the end of the
      // line to the paragraph embedding level
      _line.ResetBidiLevelForTrailingWhitespace(
        static_cast<UBiDiLevel>(rootStyle.GetWritingMode().IsBidiLTR() ? UBIDI_LTR : UBIDI_RTL));

      if (_line.HasContent())
      {
        auto applyRunBasedAlignmentIfApplicable = [&]
        {
          if (IsInIntrinsicWidthMode())
          {
            return;
          }

          // FIXME: Ruby alignment (RubyFormattingContext) and text-align: justify (InlineContentAligner) come
          // with ruby and the content aligner.
        };
        applyRunBasedAlignmentIfApplicable();
      }
    };
    handleLineEnding();

    return lineContent;
  }

  InlineLayoutUnit
    LineBuilder::LeadingPunctuationWidthForLineCandiate(const LineCandidate &lineCandidate) const noexcept
  {
    auto &inlineContent = lineCandidate.Content;
    auto firstTextRunIndex = inlineContent.FirstTextRunIndex();
    if (!firstTextRunIndex)
    {
      return {};
    }

    auto isFirstLineFirstContent = IsFirstFormattedLineCandidate() && !_line.HasContent();
    if (!isFirstLineFirstContent)
    {
      return {};
    }

    auto &runs = inlineContent.GetContinuousContent().Runs();
    auto *inlineTextItem = DynamicDowncast<InlineTextItem>(runs[*firstTextRunIndex].Item);
    if (!inlineTextItem)
    {
      krys_debug_assert(false);
      return {};
    }
    auto &style =
      IsFirstFormattedLineCandidate() ? inlineTextItem->FirstLineStyle() : inlineTextItem->Style();
    if (!TextUtil::HasHangablePunctuationStart(*inlineTextItem, style))
    {
      return {};
    }

    if (*firstTextRunIndex)
    {
      // The text content is not the first in the candidate list. However it may be the first contentful one.
      for (size_t index = *firstTextRunIndex; index--;)
      {
        if (IsContentfulOrHasDecoration(runs[index].Item, GetFormattingContext()))
        {
          return {};
        }
      }
    }
    // This candidate leading content may have hanging punctuation start.
    return TextUtil::HangablePunctuationStartWidth(*inlineTextItem, style);
  }

  InlineLayoutUnit
    LineBuilder::TrailingPunctuationOrStopOrCommaWidthForLineCandiate(const LineCandidate &lineCandidate,
                                                                      size_t startIndexAfterCandidateContent,
                                                                      size_t layoutRangeEnd) const noexcept
  {
    auto &inlineContent = lineCandidate.Content;
    auto lastTextRunIndex = inlineContent.LastTextRunIndex();
    if (!lastTextRunIndex)
    {
      return {};
    }

    auto &runs = inlineContent.GetContinuousContent().Runs();
    auto *inlineTextItem = DynamicDowncast<InlineTextItem>(runs[*lastTextRunIndex].Item);
    if (!inlineTextItem)
    {
      krys_debug_assert(false);
      return {};
    }

    auto &style =
      IsFirstFormattedLineCandidate() ? inlineTextItem->FirstLineStyle() : inlineTextItem->Style();

    if (TextUtil::HasHangableStopOrCommaEnd(*inlineTextItem, style))
    {
      // Stop or comma does apply to all lines not just the last formatted one.
      return TextUtil::HangableStopOrCommaEndWidth(*inlineTextItem, style);
    }

    if (TextUtil::HasHangablePunctuationEnd(*inlineTextItem, style))
    {
      // FIXME: If this turns out to be problematic (finding out if this is the last formatted line that is),
      // we may have to fallback to a post-process setup, where after finishing laying out the content, we go
      // back and re-layout the last (2?) line(s) when there's trailing hanging punctuation. For now let's
      // probe the content all the way to layoutRangeEnd.
      for (auto index = startIndexAfterCandidateContent; index < layoutRangeEnd; ++index)
      {
        if (IsContentfulOrHasDecoration(_inlineItemList[index], GetFormattingContext()))
        {
          return {};
        }
      }
      return TextUtil::HangablePunctuationEndWidth(*inlineTextItem, style);
    }

    return {};
  }

  List<std::pair<size_t, size_t>>
    LineBuilder::CollectShapeRanges(const LineCandidate &lineCandidate) const noexcept
  {
    // Normally candidate content is inline items between 2 soft wraping opportunities e.g.
    // <div>some text<span>more text</span></div>
    // where candidate contents are as follows: [some] [ ] [text<span>more] [ ] [text</span>]
    // However when white space is preserved and/or no wrapping is allowed the entire content is
    // one candidate content with all sorts of inline level content.

    // Let's find shaping ranges by filtering out content that are not relevant to shaping,
    // followed by processing this compressed list of [content , break, joint ] where
    // 'content' means shapable content (text)
    // 'break' means shape breaking gap (e.g. whitespace between 2 words)
    // 'keep' means box that keeps adjacent inline items in the same shaping context ("text<span>more" <-
    // inline box start)
    auto &runs = lineCandidate.Content.GetContinuousContent().Runs();

    auto isFirstFormattedLineCandidate = IsFirstFormattedLineCandidate();
    enum class ShapingType : uint8
    {
      Content,
      Break,
      Keep
    };
    struct Content
    {
      ShapingType Type {ShapingType::Break};
      size_t Index {0};
    };
    List<Content> contentList;
    for (size_t index = 0; index < runs.size(); ++index)
    {
      auto &inlineItem = runs[index].Item;

      auto type = Maybe<ShapingType> {};
      switch (inlineItem.GetType())
      {
        case InlineItem::Type::Text:
          type =
            Downcast<InlineTextItem>(inlineItem).IsWhitespace() ? ShapingType::Break : ShapingType::Content;
          break;
        case InlineItem::Type::AtomicInlineBox: type = ShapingType::Break; break;
        case InlineItem::Type::InlineBoxStart:
        case InlineItem::Type::InlineBoxEnd:
        {
          auto &boxGeometry = GetFormattingContext().GeometryForBox(inlineItem.LayoutBox());
          auto &style = isFirstFormattedLineCandidate ? inlineItem.FirstLineStyle() : inlineItem.Style();
          auto hasDecoration = [&]
          {
            // Note that this depends on the content being RTL (inline-box-end vs. start decoration matching
            // visual order -visual matching).
            auto shouldCheckLogicalStart = style.GetWritingMode().BidiDirection() == TextDirection::LTR
                                             ? inlineItem.GetType() == InlineItem::Type::InlineBoxEnd
                                             : inlineItem.GetType() == InlineItem::Type::InlineBoxStart;
            return shouldCheckLogicalStart
                     ? (boxGeometry.MarginStart() || boxGeometry.BorderStart() || boxGeometry.PaddingStart())
                     : (boxGeometry.MarginEnd() || boxGeometry.BorderEnd() || boxGeometry.PaddingEnd());
          };
          auto hasBidiIsolation = IsIsolated(style.GetUnicodeBidi());
          type = hasDecoration() || hasBidiIsolation ? ShapingType::Break : ShapingType::Keep;
          break;
        }
        case InlineItem::Type::HardLineBreak:
        case InlineItem::Type::SoftLineBreak:
        case InlineItem::Type::WordBreakOpportunity:
        case InlineItem::Type::Float:
        case InlineItem::Type::Opaque:
        case InlineItem::Type::Block:                break;
        default:                                     krys_debug_assert(false);
      }

      auto shouldIgnore = [&]
      {
        if (!type)
        {
          return true;
        }
        if (*type == ShapingType::Content)
        {
          return false;
        }
        return contentList.empty() || *type == contentList.back().Type;
      };
      if (!shouldIgnore())
      {
        contentList.push_back(Content {*type, index});
      }
    }

    // Trailing non-content entries should just be ignored.
    while (!contentList.empty())
    {
      if (contentList.back().Type == ShapingType::Content)
      {
        break;
      }
      contentList.pop_back();
    }

    if (contentList.empty())
    {
      return {};
    }

    krys_debug_assert(contentList.front().Type == ShapingType::Content
                      && contentList.back().Type == ShapingType::Content);
    List<std::pair<size_t, size_t>> ranges;

    const FontCascade *lastFontCascade = &RootStyle().GetFontCascade();
    auto leadingContentRunIndex = Maybe<size_t> {};
    auto trailingContentRunIndex = Maybe<size_t> {};
    auto hasBoundaryBetween = false;

    auto resetCandidateRange = [&]
    {
      leadingContentRunIndex = {};
      trailingContentRunIndex = {};
      hasBoundaryBetween = false;
    };
    auto commitIfHasContentAndReset = [&]
    {
      if (leadingContentRunIndex && trailingContentRunIndex && hasBoundaryBetween)
      {
        ranges.push_back({*leadingContentRunIndex, *trailingContentRunIndex});
      }
      resetCandidateRange();
    };

    for (auto entry : contentList)
    {
      switch (entry.Type)
      {
        case ShapingType::Break: commitIfHasContentAndReset(); break;
        case ShapingType::Keep:
          if (hasBoundaryBetween)
          {
            // Nested inline boxes e.g. <span>content<span>more<span>and some more
            krys_debug_assert(leadingContentRunIndex);
            break;
          }
          if (leadingContentRunIndex)
          {
            hasBoundaryBetween = true;
          }
          break;
        case ShapingType::Content:
        {
          auto &inlineTextItem = Downcast<InlineTextItem>(runs[entry.Index].Item);
          auto &styleToUse =
            isFirstFormattedLineCandidate ? inlineTextItem.FirstLineStyle() : inlineTextItem.Style();
          auto &inlineTextBox = inlineTextItem.GetInlineTextBox();
          auto isEligibleText = !inlineTextBox.CanUseSimpleFontCodePath() && !inlineTextBox.IsCombined()
                                && inlineTextItem.Direction() == TextDirection::RTL;

          if (!leadingContentRunIndex)
          {
            if (isEligibleText)
            {
              leadingContentRunIndex = entry.Index;
            }
            lastFontCascade = &styleToUse.GetFontCascade();
          }
          else if (hasBoundaryBetween)
          {
            auto hasMatchingFontCascade = *lastFontCascade == styleToUse.GetFontCascade();
            if (isEligibleText && hasMatchingFontCascade)
            {
              trailingContentRunIndex = entry.Index;
            }
            else
            {
              resetCandidateRange();
            }
          }
          else if (!isEligibleText)
          {
            resetCandidateRange();
          }
          break;
        }
        default: krys_debug_assert(false);
      }
    }
    commitIfHasContentAndReset();
    return ranges;
  }

  void LineBuilder::ApplyShapingOnRunRange(LineCandidate &lineCandidate,
                                           std::pair<size_t, size_t> range) const noexcept
  {
    using ShapingBoundary = InlineContentBreaker::ContinuousContent::Run::ShapingBoundary;
    auto &inlineContent = lineCandidate.Content;
    auto &runs = inlineContent.GetContinuousContent().Runs();
    if (range.first >= range.second || range.first >= runs.size() || range.second >= runs.size())
    {
      krys_debug_assert(false);
      return;
    }
    runs[range.first].Shaping = ShapingBoundary::Start;
    runs[range.second].Shaping = ShapingBoundary::End;

    StringBuilder textContent;
    for (auto index = range.first; index <= range.second; ++index)
    {
      if (auto *inlineTextItem = DynamicDowncast<InlineTextItem>(runs[index].Item))
      {
        textContent.Append(inlineTextItem->Content());
      }
    }

    krys_debug_assert(!textContent.IsEmpty());
    auto characterScanForCodePath = true;
    auto &style = IsFirstFormattedLineCandidate() ? runs[range.first].Item.FirstLineStyle()
                                                  : runs[range.first].Item.Style();
    auto textRun = TextRun {textContent.View(),
                            _lineLogicalRect.Left(),
                            {},
                            ExpansionBehavior::DefaultBehavior(),
                            TextDirection::RTL,
                            style.RtlOrdering() == Order::Visual,
                            characterScanForCodePath};
    // The advance of each byte: a glyph's advance goes to the first byte of its character.
    auto glyphAdvances = ComplexTextController::GlyphAdvancesForTextRun(style.GetFontCascade(), textRun);

    if (glyphAdvances.size() != textRun.Length())
    {
      krys_debug_assert(false);
      return;
    }

    size_t glyphIndex = 0;
    auto shapedContentWidth = InlineLayoutUnit {};
    for (auto index = range.first; index <= range.second; ++index)
    {
      auto &run = runs[index];
      auto *inlineTextItem = DynamicDowncast<InlineTextItem>(run.Item);
      if (!inlineTextItem)
      {
        krys_debug_assert(run.Item.IsInlineBoxStartOrEnd());
        continue;
      }
      auto runWidth = InlineLayoutUnit {};
      for (size_t i = 0; i < inlineTextItem->Length(); ++i)
      {
        runWidth += std::max(0.f, glyphAdvances[glyphIndex]);
        ++glyphIndex;
      }
      run.AdjustContentWidth(runWidth);
      shapedContentWidth += runWidth;
    }
    inlineContent.GetContinuousContent().AdjustLogicalWidth(shapedContentWidth);
    inlineContent.GetContinuousContent().SetHasShapedContent();
  }

  void LineBuilder::ApplyShapingIfNeeded(LineCandidate &lineCandidate) noexcept
  {
    if (!GetLayoutState().ShouldShapeTextAcrossInlineBoxes())
    {
      return;
    }

    if (!lineCandidate.Content.IsShapingCandidateByContent())
    {
      return;
    }

    for (auto range : CollectShapeRanges(lineCandidate))
    {
      ApplyShapingOnRunRange(lineCandidate, range);
    }
  }

  void LineBuilder::ShapePartialLineCandidate(LineCandidate &lineCandidate,
                                              size_t trailingRunIndex) const noexcept
  {
    using ShapingBoundary = InlineContentBreaker::ContinuousContent::Run::ShapingBoundary;
    auto &inlineContent = lineCandidate.Content;
    auto &runs = inlineContent.GetContinuousContent().Runs();

    if (trailingRunIndex >= runs.size())
    {
      krys_debug_assert(false);
      return;
    }

    // Find the shaping boundary end to see if we need to reshape the candidate text.
    for (auto index = trailingRunIndex + 1; index < runs.size(); ++index)
    {
      auto shapingBoundary = runs[index].Shaping;
      if (!shapingBoundary)
      {
        continue;
      }
      if (*shapingBoundary == ShapingBoundary::Start)
      {
        // Trailing content is a new shaping boundary, no need to reshape leading content.
        return;
      }
      krys_debug_assert(*shapingBoundary == ShapingBoundary::End);
      auto endPosition = Maybe<size_t> {};
      for (auto i = trailingRunIndex + 1; i--;)
      {
        auto &run = runs[i];
        if (!endPosition && run.Item.IsText())
        {
          endPosition = i;
        }

        auto runShapingBoundary = run.Shaping;
        if (runShapingBoundary && *runShapingBoundary == ShapingBoundary::Start)
        {
          if (!endPosition)
          {
            krys_debug_assert(false);
            return;
          }
          if (*endPosition == i)
          {
            // No shaping is needed when content does not cross multiple boxes.
            run.Shaping = {};
            if (i < trailingRunIndex)
            {
              run.AdjustContentWidth(GetFormattingContext().FormattingUtils().InlineItemWidth(
                run.Item, {}, IsFirstFormattedLineCandidate()));
            }
            return;
          }

          ApplyShapingOnRunRange(lineCandidate, {i, *endPosition});
          return;
        }
      }
      // We should always find a start when there's an end.
      krys_debug_assert(false);
    }
  }

  void LineBuilder::CandidateContentForLine(LineCandidate &lineCandidate,
                                            std::pair<size_t, size_t> startEndIndex,
                                            const InlineItemRange &layoutRange,
                                            InlineLayoutUnit currentLogicalRight,
                                            SkipFloats skipFloats) noexcept
  {
    krys_debug_assert(startEndIndex.first < layoutRange.EndIndex());
    krys_debug_assert(startEndIndex.second <= layoutRange.EndIndex());

    auto isFirstFormattedLineCandidate = IsFirstFormattedLineCandidate();
    lineCandidate.Reset();

    auto isLeadingPartiaContent = startEndIndex.first == layoutRange.StartIndex() && _partialLeadingTextItem;
    if (isLeadingPartiaContent)
    {
      krys_debug_assert(!_overflowingLogicalWidth);
      // Handle leading partial content first (overflowing text from the previous line).
      auto itemWidth = GetFormattingContext().FormattingUtils().InlineItemWidth(
        *_partialLeadingTextItem, currentLogicalRight, isFirstFormattedLineCandidate);
      lineCandidate.Content.AppendInlineItem(*_partialLeadingTextItem, _partialLeadingTextItem->Style(),
                                             itemWidth);
      currentLogicalRight += itemWidth;
      ++startEndIndex.first;
    }

    auto trailingSoftHyphenInlineTextItemIndex = Maybe<size_t> {};
    auto textSpacingAdjustment = InlineLayoutUnit {};
    auto contentHasInlineItemsWithDecorationClone = !_line.GetInlineBoxListWithClonedDecorationEnd().empty();

    for (auto index = startEndIndex.first; index < startEndIndex.second; ++index)
    {
      auto &inlineItem = _inlineItemList[index];
      auto &style = isFirstFormattedLineCandidate ? inlineItem.FirstLineStyle() : inlineItem.Style();
      if (inlineItem.IsInlineBoxStart())
      {
        auto &boundarySpacings = _textSpacingContext.InlineBoxBoundarySpacings;
        if (auto inlineBoxBoundaryTextSpacing = boundarySpacings.find(index);
            inlineBoxBoundaryTextSpacing != boundarySpacings.end())
        {
          textSpacingAdjustment = inlineBoxBoundaryTextSpacing->second;
        }
      }

      auto needsLayout = inlineItem.IsFloat() || inlineItem.IsAtomicInlineBox()
                         || (inlineItem.IsOpaque() && inlineItem.LayoutBox().IsRubyAnnotationBox());
      if (needsLayout)
      {
        // FIXME: Intrinsic width mode should call into the intrinsic width codepath. Currently we only get
        // here when box has fixed width (meaning no need to run intrinsic width on the box).
        if (!IsInIntrinsicWidthMode())
        {
          GetFormattingContext().GetIntegrationUtils().LayoutWithFormattingContextForBox(
            Downcast<ElementBox>(inlineItem.LayoutBox()));
        }
      }

      if (inlineItem.IsFloat())
      {
        if (skipFloats == SkipFloats::Yes)
        {
          continue;
        }
        lineCandidate.FloatItem = &inlineItem;
        // This is a soft wrap opportunity, must be the only item in the list.
        krys_debug_assert(startEndIndex.first + 1 == startEndIndex.second);
        continue;
      }
      if (auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItem))
      {
        auto logicalWidth = _overflowingLogicalWidth
                              ? *std::exchange(_overflowingLogicalWidth, null)
                              : GetFormattingContext().FormattingUtils().InlineItemWidth(
                                  *inlineTextItem, currentLogicalRight, isFirstFormattedLineCandidate);
        if (!currentLogicalRight)
        {
          auto &trimmableSpacings = _textSpacingContext.TrimmableSpacings;
          if (auto trimmableSpacing = trimmableSpacings.find(index);
              trimmableSpacing != trimmableSpacings.end())
          {
            logicalWidth -= trimmableSpacing->second;
          }
        }
        lineCandidate.Content.AppendInlineItem(*inlineTextItem, style, logicalWidth);
        // Word spacing does not make the run longer, but it produces an offset instead. See
        // Line::appendTextContent as well.
        currentLogicalRight +=
          logicalWidth + (inlineTextItem->IsWordSeparator() ? style.GetFontCascade().WordSpacing() : 0.f);
        trailingSoftHyphenInlineTextItemIndex =
          inlineTextItem->HasTrailingSoftHyphen() ? Maybe<size_t> {index} : null;
        continue;
      }
      if (inlineItem.IsInlineBoxStartOrEnd())
      {
        auto logicalWidth = GetFormattingContext().FormattingUtils().InlineItemWidth(
          inlineItem, currentLogicalRight, isFirstFormattedLineCandidate);
        // FIXME: Ruby bases reserve room for their annotations (RubyFormattingContext), once there's ruby
        // layout.

        contentHasInlineItemsWithDecorationClone |=
          inlineItem.IsInlineBoxStart() && style.GetBoxDecorationBreak() == BoxDecorationBreak::Clone;
        lineCandidate.Content.AppendInlineItem(inlineItem, style, logicalWidth, textSpacingAdjustment);
        currentLogicalRight += logicalWidth;
        continue;
      }
      if (inlineItem.IsAtomicInlineBox())
      {
        auto logicalWidth = GetFormattingContext().FormattingUtils().InlineItemWidth(
          inlineItem, currentLogicalRight, isFirstFormattedLineCandidate);
        // FIXME: While the line breaking related properties for atomic level boxes do not depend on the line
        // index (first line style) it'd be great to figure out the correct style to pass in.
        lineCandidate.Content.AppendInlineItem(inlineItem, inlineItem.LayoutBox().Parent().Style(),
                                               logicalWidth);
        currentLogicalRight += logicalWidth;
        continue;
      }
      if (inlineItem.IsLineBreak() || inlineItem.IsWordBreakOpportunity())
      {
#if KRYS_DEBUG
        // Since both <br> and <wbr> are explicit word break opportunities they have to be trailing items in
        // this candidate run list unless they are embedded in inline boxes. e.g. <span><wbr></span>
        for (auto i = index + 1; i < startEndIndex.second; ++i)
        {
          krys_debug_assert(_inlineItemList[i].IsInlineBoxEnd() || _inlineItemList[i].IsOpaque());
        }
#endif
        lineCandidate.Content.AppendInlineItem(inlineItem, style, {});
        continue;
      }
      if (inlineItem.IsOpaque())
      {
        lineCandidate.Content.AppendInlineItem(inlineItem, style, {});
        continue;
      }
      if (inlineItem.IsBlock())
      {
        // Blocks must be the only items in the list.
        lineCandidate.BlockItem = &inlineItem;
        krys_debug_assert(startEndIndex.first + 1 == startEndIndex.second);
        continue;
      }
      krys_debug_assert(false);
    }

    if (lineCandidate.FloatItem || lineCandidate.BlockItem)
    {
      return;
    }

    auto setupTrailingContent = [&]
    {
      lineCandidate.Content.SetHasTrailingClonedDecoration(contentHasInlineItemsWithDecorationClone);

      auto setLeadingAndTrailingHangingPunctuation = [&]
      {
        auto &inlineContent = lineCandidate.Content;
        auto hangingContentWidth = inlineContent.GetContinuousContent().HangingContentWidth();
        // Do not even try to check for trailing punctuation when the candidate content already has whitespace
        // type of hanging content.
        if (!hangingContentWidth)
        {
          hangingContentWidth += TrailingPunctuationOrStopOrCommaWidthForLineCandiate(
            lineCandidate, startEndIndex.second, layoutRange.EndIndex());
        }
        hangingContentWidth += LeadingPunctuationWidthForLineCandiate(lineCandidate);
        if (hangingContentWidth)
        {
          lineCandidate.Content.SetHangingContentWidth(hangingContentWidth);
        }
      };
      setLeadingAndTrailingHangingPunctuation();

      auto setTrailingSoftHyphenWidth = [&]
      {
        if (!trailingSoftHyphenInlineTextItemIndex)
        {
          return;
        }
        for (auto index = *trailingSoftHyphenInlineTextItemIndex; index < startEndIndex.second; ++index)
        {
          if (!Is<InlineTextItem>(_inlineItemList[index]))
          {
            return;
          }
        }
        auto &trailingInlineTextItem = _inlineItemList[*trailingSoftHyphenInlineTextItemIndex];
        auto &style = isFirstFormattedLineCandidate ? trailingInlineTextItem.FirstLineStyle()
                                                    : trailingInlineTextItem.Style();
        lineCandidate.Content.SetTrailingSoftHyphenWidth(TextUtil::HyphenWidth(style));
      };
      setTrailingSoftHyphenWidth();
      lineCandidate.Content.SetHasTrailingSoftWrapOpportunity(
        HasTrailingSoftWrapOpportunity(startEndIndex.second, layoutRange.EndIndex(), _inlineItemList));
    };
    setupTrailingContent();
    ApplyShapingIfNeeded(lineCandidate);
  }

  namespace
  {
    KRYS_NODISCARD inline InlineLayoutUnit
      AvailableWidth(const Line &line, InlineLayoutUnit lineWidth,
                     Maybe<IntrinsicWidthMode> intrinsicWidthMode) noexcept
    {
      // 1. Preferred width computation sums up floats while line breaker subtracts them.
      // 2. Available space is inherently a LayoutUnit based value (coming from block/flex etc layout) and it
      // is the result of a floored float. These can all lead to epsilon-scale differences.
      if (!intrinsicWidthMode || *intrinsicWidthMode == IntrinsicWidthMode::Maximum)
      {
        lineWidth += LayoutUnit::Epsilon();
      }
      auto availableWidth = lineWidth - line.ContentLogicalRight();
      return std::isnan(availableWidth) ? MaxInlineLayoutUnit() : availableWidth;
    }

    KRYS_NODISCARD bool HaveEnoughSpaceForFloatWithClear(const Rect &floatBoxMarginBox, bool isLeftPositioned,
                                                         const InlineRect &lineLogicalRect,
                                                         InlineLayoutUnit contentLogicalWidth) noexcept
    {
      auto adjustedLineLogicalLeft = lineLogicalRect.Left();
      auto adjustedLineLogicalRight = lineLogicalRect.Right();
      if (isLeftPositioned)
      {
        adjustedLineLogicalLeft =
          std::max<InlineLayoutUnit>(floatBoxMarginBox.Right().ToFloat(), adjustedLineLogicalLeft);
      }
      else
      {
        adjustedLineLogicalRight =
          std::min<InlineLayoutUnit>(floatBoxMarginBox.Left().ToFloat(), adjustedLineLogicalRight);
      }
      auto availableSpaceForContentWithPlacedFloat = adjustedLineLogicalRight - adjustedLineLogicalLeft;
      return contentLogicalWidth <= availableSpaceForContentWithPlacedFloat;
    }
  }

  LineBuilder::RectAndFloatConstraints
    LineBuilder::FloatAvoidingRect(const InlineRect &logicalRect,
                                   InlineLayoutUnit lineMarginStart) const noexcept
  {
    if (IsInIntrinsicWidthMode() || GetFloatingContext().IsEmpty())
    {
      return {logicalRect, {}};
    }

    auto constraints = GetFormattingContext().FormattingUtils().FloatConstraintsForLine(
      logicalRect.Top(), logicalRect.Height(), GetFloatingContext());
    if (!constraints.Start && !constraints.End)
    {
      return {logicalRect, {}};
    }

    auto constrainedSideSet = EnumSet<UsedFloat> {};
    // text-indent acts as (start)margin on the line. When looking for intrusive floats we need to check
    // against the line's _margin_ box.
    auto marginBoxRect = InlineRect {logicalRect.Top(), logicalRect.Left() - lineMarginStart,
                                     logicalRect.Width() + lineMarginStart, logicalRect.Height()};

    if (constraints.Start && constraints.Start->X.ToFloat() > marginBoxRect.Left())
    {
      marginBoxRect.ShiftLeftTo(constraints.Start->X.ToFloat());
      constrainedSideSet.Add(UsedFloat::Left);
    }
    if (constraints.End && constraints.End->X.ToFloat() < marginBoxRect.Right())
    {
      marginBoxRect.SetRight(std::max<InlineLayoutUnit>(marginBoxRect.Left(), constraints.End->X.ToFloat()));
      constrainedSideSet.Add(UsedFloat::Right);
    }

    auto lineLogicalRect = InlineRect {marginBoxRect.Top(), marginBoxRect.Left() + lineMarginStart,
                                       marginBoxRect.Width() - lineMarginStart, marginBoxRect.Height()};
    return {lineLogicalRect, constrainedSideSet};
  }

  LineBuilder::RectAndFloatConstraints
    LineBuilder::AdjustedLineRectWithCandidateInlineContent(const LineCandidate &lineCandidate) const noexcept
  {
    // Check if the candidate content would stretch the line and whether additional floats are getting in the
    // way.
    auto &inlineContent = lineCandidate.Content;
    if (IsInIntrinsicWidthMode())
    {
      return {_lineLogicalRect};
    }
    // FIXME: Use InlineFormattingUtils::inlineLevelBoxAffectsLineBox instead.
    auto candidateContentHeight = InlineLayoutUnit {};
    auto lineBoxContain = RootStyle().LineBoxContain();
    for (auto &run : inlineContent.GetContinuousContent().Runs())
    {
      auto &inlineItem = run.Item;
      if (inlineItem.IsText())
      {
        auto &styleToUse = IsFirstFormattedLineCandidate() ? inlineItem.FirstLineStyle() : inlineItem.Style();
        candidateContentHeight =
          std::max<InlineLayoutUnit>(candidateContentHeight, styleToUse.ComputedLineHeight());
      }
      else if (inlineItem.IsAtomicInlineBox()
               && lineBoxContain.Contains(style::WebkitLineBoxContainValue::Replaced))
      {
        candidateContentHeight =
          std::max(candidateContentHeight,
                   GetFormattingContext().GeometryForBox(inlineItem.LayoutBox()).MarginBoxHeight().ToFloat());
      }
    }
    if (candidateContentHeight <= _lineLogicalRect.Height())
    {
      return {_lineLogicalRect};
    }

    return FloatAvoidingRect({_lineLogicalRect.TopLeft(), _lineLogicalRect.Width(), candidateContentHeight},
                             _lineMarginStart);
  }

  Maybe<LineBuilder::InitialLetterOffsets>
    LineBuilder::AdjustLineRectForInitialLetterIfApplicable(const Box &floatBox) noexcept
  {
    auto drop = floatBox.Style().InitialLetter().Drop();
    auto isInitialLetter = floatBox.IsFloatingPositioned()
                           && floatBox.Style().GetPseudoElementType() == PseudoElementType::FirstLetter
                           && drop;
    if (!isInitialLetter)
    {
      return {};
    }

    // Here we try to set the vertical start position for the float in flush with the adjoining text content's
    // cap height. It's a super premature as at this point we don't normally deal with vertical geometry
    // -other than the incoming vertical constraint.
    auto initialLetterCapHeightOffset =
      GetFormattingContext().Quirks().InitialLetterAlignmentOffset(floatBox, RootStyle());
    // While initial-letter based floats do not set their clear property, intrusive floats from sibling IFCs
    // are supposed to be cleared.
    auto intrusiveBottom = GetBlockLayoutState().IntrusiveInitialLetterLogicalBottom();
    if (!initialLetterCapHeightOffset && !intrusiveBottom)
    {
      return {};
    }

    auto clearGapBeforeFirstLine = InlineLayoutUnit {};
    if (intrusiveBottom)
    {
      // When intrusive initial letter is cleared, we introduce a clear gap. This is (with proper floats)
      // normally computed before starting line layout but intrusive initial letters are cleared only when
      // another initial letter shows up. Regular inline content does not need clearance.
      auto intrusiveInitialLetterWidth =
        std::max(0.f, _lineLogicalRect.Left() - _lineInitialLogicalRect.Left());
      _lineLogicalRect.SetLeft(_lineInitialLogicalRect.Left());
      _lineLogicalRect.ExpandHorizontally(intrusiveInitialLetterWidth);
      clearGapBeforeFirstLine = intrusiveBottom->ToFloat();
    }

    auto sunkenBelowFirstLineOffset = LayoutUnit {};
    auto letterHeight = floatBox.Style().InitialLetter().Height();
    if (drop < letterHeight)
    {
      // Sunken/raised initial letter pushes contents of the first line down.
      auto numberOfSunkenLines = letterHeight - drop;
      auto verticalGapForInlineContent = numberOfSunkenLines * RootStyle().ComputedLineHeight();
      clearGapBeforeFirstLine += verticalGapForInlineContent;
      // And we pull the initial letter up.
      initialLetterCapHeightOffset =
        LayoutUnit {-verticalGapForInlineContent} + initialLetterCapHeightOffset.value_or(0_lu);
    }
    else if (drop > letterHeight)
    {
      // Initial letter is sunken below the first line.
      auto numberOfLinesAboveInitialLetter = drop - letterHeight;
      sunkenBelowFirstLineOffset =
        LayoutUnit {numberOfLinesAboveInitialLetter * RootStyle().ComputedLineHeight()};
    }

    _lineLogicalRect.MoveVertically(clearGapBeforeFirstLine);
    // There should never be multiple initial letters.
    krys_debug_assert(!_initialLetterClearGap);
    _initialLetterClearGap = clearGapBeforeFirstLine;
    return InitialLetterOffsets {initialLetterCapHeightOffset.value_or(0_lu), sunkenBelowFirstLineOffset};
  }

  bool LineBuilder::ShouldTryToPlaceFloatBox(const Box &floatBox, LayoutUnit floatBoxMarginBoxWidth,
                                             MayOverConstrainLine mayOverConstrainLine) const noexcept
  {
    switch (mayOverConstrainLine)
    {
      case MayOverConstrainLine::Yes: return true;
      case MayOverConstrainLine::OnlyWhenFirstFloatOnLine:
        // This is a resumed float from a previous line. Now we need to find a place for it.
        // (which also means that the current line can't have any floats that we couldn't place yet)
        krys_debug_assert(_suspendedFloats.empty());
        if (!IsLineConstrainedByFloat())
        {
          return true;
        }
        [[fallthrough]];
      case MayOverConstrainLine::No:
      {
        auto lineIsConsideredEmpty = !_line.HasContent() && !IsLineConstrainedByFloat();
        if (lineIsConsideredEmpty)
        {
          return true;
        }
        // Non-clear type of floats stack up (horizontally). It's easy to check if there's space for this
        // float at all, while floats with clear needs post-processing to see if they overlap existing line
        // content (and here we just check if they may fit at all).
        auto lineLogicalWidth =
          floatBox.HasFloatClear() ? _lineInitialLogicalRect.Width() : _lineLogicalRect.Width();
        auto availableWidthForFloat =
          lineLogicalWidth - _line.ContentLogicalRight() + _line.TrimmableTrailingWidth();
        return availableWidthForFloat >= floatBoxMarginBoxWidth.ToFloat();
      }
      default: krys_debug_assert(false); return true;
    }
  }

  bool LineBuilder::TryPlacingFloatBox(const Box &floatBox,
                                       MayOverConstrainLine mayOverConstrainLine) noexcept
  {
    if (IsFloatLayoutSuspended())
    {
      return false;
    }

    auto &floatingContext = GetFloatingContext();
    auto &boxGeometry = GetFormattingContext().GeometryForBox(floatBox);
    if (!ShouldTryToPlaceFloatBox(floatBox, boxGeometry.MarginBoxWidth(), mayOverConstrainLine))
    {
      return false;
    }

    auto lineMarginBoxLeft = std::max(0.f, _lineLogicalRect.Left() - _lineMarginStart);
    auto computeFloatBoxPosition = [&]
    {
      // Set static position first.
      auto staticPosition = LayoutPoint {LayoutUnit {lineMarginBoxLeft}, LayoutUnit {_lineLogicalRect.Top()}};
      if (auto additionalOffsets = AdjustLineRectForInitialLetterIfApplicable(floatBox))
      {
        staticPosition.SetY(LayoutUnit {_lineLogicalRect.Top()} + additionalOffsets->CapHeightOffset);
        boxGeometry.SetVerticalMargin(
          {boxGeometry.MarginBefore() + additionalOffsets->SunkenBelowFirstLineOffset,
           boxGeometry.MarginAfter()});
      }
      staticPosition.Move(boxGeometry.MarginStart(), boxGeometry.MarginBefore());
      boxGeometry.SetTopLeft(staticPosition);
      // Compute float position by running float layout.
      auto floatingPosition =
        floatingContext.PositionForFloat(floatBox, boxGeometry, RootHorizontalConstraints());
      boxGeometry.SetTopLeft(floatingPosition);
    };
    computeFloatBoxPosition();

    auto willFloatBoxShrinkLine = [&]
    {
      // Float boxes don't get positioned higher than the line.
      auto floatBoxMarginBox = BoxGeometry::MarginBoxRect(boxGeometry);
      if (floatBoxMarginBox.IsEmpty())
      {
        return false;
      }
      if (floatBoxMarginBox.Right().ToFloat() <= lineMarginBoxLeft)
      {
        // Previous floats already constrain the line horizontally more than this one.
        return false;
      }
      // Empty rect case: "line-height: 0px;" line still intersects with intrusive floats.
      return floatBoxMarginBox.Top().ToFloat() == _lineLogicalRect.Top()
             || floatBoxMarginBox.Top().ToFloat() < _lineLogicalRect.Bottom();
    }();

    auto willFloatBoxWithClearFit = [&]
    {
      if (!willFloatBoxShrinkLine)
      {
        return true;
      }
      auto lineIsConsideredEmpty = !_line.HasContent() && !IsLineConstrainedByFloat();
      if (lineIsConsideredEmpty)
      {
        return true;
      }
      // When floats with clear are placed under existing floats, we may find ourselves in an over-constrained
      // state and can't place this float here.
      auto contentLogicalWidth = _line.ContentLogicalWidth() - _line.TrimmableTrailingWidth();
      return HaveEnoughSpaceForFloatWithClear(BoxGeometry::MarginBoxRect(boxGeometry),
                                              floatingContext.IsStartPositioned(floatBox), _lineLogicalRect,
                                              contentLogicalWidth);
    };
    if (floatBox.HasFloatClear() && !willFloatBoxWithClearFit())
    {
      return false;
    }

    auto placeFloatBox = [&]
    {
      size_t lineIndex = _previousLine ? (_previousLine->LineIndex + 1) : 0;
      auto floatItem = floatingContext.MakeFloatItem(floatBox, boxGeometry, lineIndex);
      GetLayoutState().GetPlacedFloats().Add(floatItem);
      _placedFloats.push_back(floatItem);
    };
    placeFloatBox();

    auto adjustLineRectIfNeeded = [&]
    {
      if (!willFloatBoxShrinkLine)
      {
        // This float is placed outside the line box. No need to shrink the current line.
        return;
      }
      auto constraints = FloatAvoidingRect(_lineLogicalRect, _lineMarginStart);
      _lineLogicalRect = constraints.LogicalRect;
      _lineIsConstrainedByFloat.Add(constraints.ConstrainedSideSet);
    };
    adjustLineRectIfNeeded();

    return true;
  }

  void LineBuilder::HandleBlockContent(const InlineItem &blockItem) noexcept
  {
    krys_debug_assert(blockItem.IsBlock());
    // Blocks are always the only content on the line.
    krys_debug_assert(!_line.HasContentOrListMarker());
    if (!IsInIntrinsicWidthMode())
    {
      GetFormattingContext().GetIntegrationUtils().LayoutWithFormattingContextForBlockInInline(
        Downcast<ElementBox>(blockItem.LayoutBox()), ToLayoutPoint(_lineLogicalRect.TopLeft()),
        GetLayoutState());
    }
    auto marginBoxLogicalWidth =
      GetFormattingContext().FormattingUtils().InlineItemWidth(blockItem, {}, false);
    _line.AppendBlock(blockItem, marginBoxLogicalWidth);
    if (RootStyle().GetWritingMode().IsBidiRTL())
    {
      _line.SetContentNeedsBidiReordering();
    }
  }

  LineBuilder::Result LineBuilder::HandleInlineContent(const InlineItemRange &layoutRange,
                                                       LineCandidate &lineCandidate) noexcept
  {
    auto result = ApplyLineBreakingOnCandidateInlineContent(layoutRange, lineCandidate);
    if (!_line.HasContentOrListMarker())
    {
      return result;
    }

    auto applyMarginInBlockDirection = [&]() -> LayoutUnit
    {
      // We don't know if margin coming from previous content should be applied or not
      // until after we managed to put some inline content on the line.
      // e.g.
      // <span>text<div style="margin-bottom: 100px;"></div>more text</span> v.s
      // <span>text<div style="margin-bottom: 100px;"></div> <div></div></span>
      // where in the first example, the 100px gap is between the block container's edge and "more text"
      // while in the second case, it is somewhere after the second block container (can't tell).
      auto &marginState = GetBlockLayoutState().GetMarginState();
      auto marginValue = marginState.Margin();
      marginState.ResetMarginValues();

      if (marginState.AtBeforeSideOfBlock)
      {
        marginState.ResetBeforeSideOfBlock();
        return {};
      }
      return marginValue;
    };
    auto lineOffset = applyMarginInBlockDirection();
    if (!lineOffset)
    {
      return result;
    }

    // This is similar to what we do in block layout when the estimated top position turns out to be incorrect
    // and now we have to relayout the content with the adjusted vertical position to make sure we avoid
    // floats properly.
    _lineLogicalRect = {_lineLogicalRect.Top() + lineOffset.ToFloat(), _lineInitialLogicalRect.Left(),
                        _lineInitialLogicalRect.Width(), _lineInitialLogicalRect.Height()};
    if (GetFloatingContext().IsEmpty())
    {
      return result;
    }

    _line.Initialize(_lineSpanningInlineBoxes, IsFirstFormattedLineCandidate());

    auto commitPrecedingNonContentfulContent = [&]
    {
      LineCandidate precedingNonContentfulContent;
      auto &firstCandidateItem = lineCandidate.Content.GetContinuousContent().Runs().front().Item;
      // FIXME: There should not be any previous-non-contentful inline items here but since we "break"
      // before/after floats, content like <span><floatbox>content</span> may trigger this (where the <span>
      // is disconnected from the inline content)
      for (size_t index = layoutRange.StartIndex(); index < layoutRange.EndIndex(); ++index)
      {
        auto &inlineItem = _inlineItemList[index];
        if (&inlineItem == &firstCandidateItem)
        {
          break;
        }

        if (!inlineItem.IsFloat())
        {
          auto &styleToUse =
            IsFirstFormattedLineCandidate() ? inlineItem.FirstLineStyle() : inlineItem.Style();
          precedingNonContentfulContent.Content.AppendInlineItem(inlineItem, styleToUse, {});
        }
      }
      if (!precedingNonContentfulContent.Content.IsEmpty())
      {
        (void)ApplyLineBreakingOnCandidateInlineContent(layoutRange, precedingNonContentfulContent);
      }
    };
    commitPrecedingNonContentfulContent();
    return ApplyLineBreakingOnCandidateInlineContent(layoutRange, lineCandidate);
  }

  LineBuilder::Result
    LineBuilder::ApplyLineBreakingOnCandidateInlineContent(const InlineItemRange &layoutRange,
                                                           LineCandidate &lineCandidate) noexcept
  {
    using Action = InlineContentBreaker::Result::Action;
    using IsEndOfLine = InlineContentBreaker::IsEndOfLine;
    auto result = LineBuilder::Result {};
    auto &inlineContent = lineCandidate.Content;

    auto &continuousInlineContent = inlineContent.GetContinuousContent();
    if (continuousInlineContent.Runs().empty())
    {
      krys_debug_assert(inlineContent.TrailingLineBreak() || inlineContent.TrailingWordBreakOpportunity());
      result = {inlineContent.TrailingLineBreak() ? IsEndOfLine::Yes : IsEndOfLine::No};
      return result;
    }

    auto constraints = AdjustedLineRectWithCandidateInlineContent(lineCandidate);
    auto availableWidthForCandidateContent = [&]
    {
      size_t lineIndex = _previousLine ? (_previousLine->LineIndex + 1) : 0;
      // If width constraint overrides exist (e.g. text-wrap: balance), modify the available width
      // accordingly.
      const auto &availableLineWidthOverride = GetLayoutState().GetAvailableLineWidthOverride();
      auto widthOverride = availableLineWidthOverride.AvailableLineWidthOverrideForLine(lineIndex);
      auto availableTotalWidthForContent =
        widthOverride ? widthOverride->ToFloat() - _lineMarginStart : constraints.LogicalRect.Width();
      return AvailableWidth(_line, availableTotalWidthForContent, GetIntrinsicWidthMode());
    }();

    auto lineHasContent = _line.HasContentOrListMarker();
    auto verticalPositionHasFloatOrInlineContent =
      lineHasContent || IsLineConstrainedByFloat() || !constraints.ConstrainedSideSet.IsEmpty();
    auto lineBreakingResult = InlineContentBreaker::Result {Action::Keep, IsEndOfLine::No, {}, {}};

    if (auto minimumRequiredWidth = continuousInlineContent.MinimumRequiredWidth();
        minimumRequiredWidth && *minimumRequiredWidth > availableWidthForCandidateContent)
    {
      if (verticalPositionHasFloatOrInlineContent)
      {
        lineBreakingResult = InlineContentBreaker::Result {Action::Wrap, IsEndOfLine::Yes, {}, {}};
      }
    }
    else
    {
      auto lineStatus = InlineContentBreaker::LineStatus {
        _line.ContentLogicalRight(),         availableWidthForCandidateContent,
        _line.TrimmableTrailingWidth(),      _line.TrailingSoftHyphenWidth(),
        _line.IsTrailingRunFullyTrimmable(), verticalPositionHasFloatOrInlineContent,
        !_wrapOpportunityList.empty()};
      auto needsClonedDecorationHandling = inlineContent.HasTrailingClonedDecoration()
                                           || !_line.GetInlineBoxListWithClonedDecorationEnd().empty();
      if (needsClonedDecorationHandling)
      {
        lineBreakingResult = HandleInlineContentWithClonedDecoration(lineCandidate, lineStatus);
      }
      else if (continuousInlineContent.LogicalWidth() > availableWidthForCandidateContent)
      {
        lineBreakingResult =
          GetInlineContentBreaker().ProcessInlineContent(continuousInlineContent, lineStatus);
      }
    }
    result = ProcessLineBreakingResult(lineCandidate, layoutRange, lineBreakingResult);

    auto lineGainsNewContent =
      lineBreakingResult.ResultAction == Action::Keep || lineBreakingResult.ResultAction == Action::Break;
    if (lineGainsNewContent || !lineHasContent)
    {
      // In some cases in order to put this content on the line, we have to avoid float boxes that didn't
      // constrain the line initially. (e.g. when this new content is taller than any previous content and
      // there are vertically stacked floats) In some other cases we can't put any content on the line due to
      // such newly discovered floats (e.g. shape-outside floats with gaps in-between them in vertical axis)
      _lineLogicalRect = constraints.LogicalRect;
      _lineIsConstrainedByFloat.Add(constraints.ConstrainedSideSet);
    }
    _candidateContentMaximumHeight = constraints.LogicalRect.Height();
    return result;
  }

  namespace
  {
    KRYS_NODISCARD inline InlineLayoutUnit LineBreakingResultContentWidth(
      const InlineContentBreaker::ContinuousContent::RunList &runs,
      const InlineContentBreaker::Result::PartialTrailingContent &trailingContent) noexcept
    {
      if (trailingContent.TrailingRunIndex >= runs.size())
      {
        krys_debug_assert(false);
        return {};
      }

      auto contentWidth = InlineLayoutUnit {};
      for (size_t index = 0; index < trailingContent.TrailingRunIndex; ++index)
      {
        contentWidth += runs[index].ContentWidth();
      }

      if (auto partialTrailingRun = trailingContent.Partial)
      {
        return contentWidth + partialTrailingRun->LogicalWidth
               + partialTrailingRun->HyphenWidth.value_or(0.f);
      }

      auto &trailingRun = runs[trailingContent.TrailingRunIndex];
      return contentWidth + trailingRun.ContentWidth() + trailingContent.HyphenWidth.value_or(0.f);
    }
  }

  InlineLayoutUnit LineBuilder::PlacedClonedDecorationWidth(
    const InlineContentBreaker::ContinuousContent::RunList &runs) const noexcept
  {
    // Collect already placed, not yet closed inline boxes on the line (minus what we are about to close with
    // the candidate runs) e.g. <div><span>1 <span>2 3 4</span></span></div> At [3] we've got 2 inline boxes
    // placed on the line and they may have space taking (cloned) decoration ends. At [4</span></span>] all
    // inline boxes are closed.
    auto &formattingContext = GetFormattingContext();

    HashSet<const Box *> clonedInlineBoxes;
    auto clonedDecorationEndWidth = InlineLayoutUnit {};
    for (auto *box : _line.GetInlineBoxListWithClonedDecorationEnd())
    {
      clonedDecorationEndWidth += formattingContext.GeometryForBox(*box).BorderAndPaddingEnd().ToFloat();
      clonedInlineBoxes.insert(box);
    }

    for (size_t index = 0; index < runs.size(); ++index)
    {
      auto &inlineItem = runs[index].Item;
      if (inlineItem.IsInlineBoxEnd() && clonedInlineBoxes.contains(&inlineItem.LayoutBox()))
      {
        clonedDecorationEndWidth -=
          formattingContext.GeometryForBox(inlineItem.LayoutBox()).BorderAndPaddingEnd().ToFloat();
      }
    }

    return clonedDecorationEndWidth;
  }

  InlineLayoutUnit LineBuilder::ClonedDecorationAtBreakingPosition(
    const InlineContentBreaker::ContinuousContent::RunList &runs,
    const InlineContentBreaker::Result::PartialTrailingContent &trailingContent) const noexcept
  {
    // Compute how much decoration end we have to put as trailing content if we were to break the line at this
    // position. Collect already committed, but not yet closed inline boxes in addition to these new ones,
    // coming with the candidate content. e.g. <div><span>1 <span>2 3 4</span></span></div> At [<span>2], we
    // have to account for the leading inline box (provided it has cloned decoration) and the inline box
    // (again, if it has cloned decoration) in the candidate content.
    if (trailingContent.TrailingRunIndex >= runs.size())
    {
      krys_debug_assert(false);
      return {};
    }

    auto &formattingContext = GetFormattingContext();
    auto clonedDecorationWidth = InlineLayoutUnit {};

    for (auto *box : _line.GetInlineBoxListWithClonedDecorationEnd())
    {
      clonedDecorationWidth += formattingContext.GeometryForBox(*box).BorderAndPaddingEnd().ToFloat();
    }

    for (size_t index = 0; index <= trailingContent.TrailingRunIndex; ++index)
    {
      auto &inlineItem = runs[index].Item;
      if (!inlineItem.IsInlineBoxStartOrEnd()
          || inlineItem.Style().GetBoxDecorationBreak() != BoxDecorationBreak::Clone)
      {
        continue;
      }

      auto &inlineBoxGeometry = formattingContext.GeometryForBox(inlineItem.LayoutBox());
      if (inlineItem.IsInlineBoxStart())
      {
        clonedDecorationWidth += inlineBoxGeometry.BorderAndPaddingEnd().ToFloat();
        continue;
      }
      if (inlineItem.IsInlineBoxEnd())
      {
        clonedDecorationWidth -= inlineBoxGeometry.BorderAndPaddingEnd().ToFloat();
        continue;
      }
    }
    krys_debug_assert(clonedDecorationWidth >= 0);
    return std::max(0.f, clonedDecorationWidth);
  }

  InlineContentBreaker::Result
    LineBuilder::HandleInlineContentWithClonedDecoration(const LineCandidate &lineCandidate,
                                                         InlineContentBreaker::LineStatus lineStatus) noexcept
  {
    // 1. call content breaker to see whether the candidate content fits or not
    // 2. when content breaker tells us that this continuous content needs to be broken up, we have to check
    // whether the partial content we are planning to put on the line has cloned decoration and whether it
    // also fits
    // 3. traverse the candidate content up to the breaking position and compute the width of the cloned
    // decoration(s)
    // 4. check if there's enough space for both content and its cloned decoration(s)
    // 5. if not, let's try again (go to #1) with reduced available space
    // At some point we either manage to fit the content + its cloned decoration(s) or we run out of available
    // space e.g. <div style="width: 30px; word-break: break-all">ab<span style="-webkit-box-decoration-break:
    // clone; padding-right: 20px">cd</span>ef</div> (where each character is 10px wide) [ab<span>cd</span>ef]
    // is the continous content (there's no soft wrap opportunity in-between) The breaking position is between
    // [c] and [d]. We are going to put [abc] on the line which means we have to have space for the enclosing
    // inline box's (cloned) decoration end (20px) too, 50px altogether. -but we only have 30px space here.
    // And now we are at step (5); let's probe line breaking with reduced available space, go to step (1)
    // until we find a valid breaking position (which is after [b]).
    using Action = InlineContentBreaker::Result::Action;
    krys_debug_assert(lineCandidate.Content.HasTrailingClonedDecoration()
                      || !_line.GetInlineBoxListWithClonedDecorationEnd().empty());

    auto &inlineContent = lineCandidate.Content;
    auto &continuousInlineContent = inlineContent.GetContinuousContent();
    auto &runs = continuousInlineContent.Runs();
    auto initialAvailableWidth = lineStatus.AvailableWidth;

    lineStatus.AvailableWidth -= PlacedClonedDecorationWidth(runs);

    if (continuousInlineContent.LogicalWidth() <= lineStatus.AvailableWidth)
    {
      return {Action::Keep, InlineContentBreaker::IsEndOfLine::No, {}, {}};
    }

    while (lineStatus.AvailableWidth)
    {
      auto lineBreakingResult =
        GetInlineContentBreaker().ProcessInlineContent(continuousInlineContent, lineStatus);
      if (lineBreakingResult.ResultAction != Action::Break)
      {
        return lineBreakingResult;
      }

      if (!lineBreakingResult.PartialTrailing)
      {
        krys_debug_assert(false);
        return lineBreakingResult;
      }

      auto contentWidth = LineBreakingResultContentWidth(runs, *lineBreakingResult.PartialTrailing);
      auto clonedDecorationWidth =
        ClonedDecorationAtBreakingPosition(runs, *lineBreakingResult.PartialTrailing);

      if (contentWidth + clonedDecorationWidth <= initialAvailableWidth)
      {
        return lineBreakingResult;
      }
      lineStatus.AvailableWidth = std::max(0.f, std::min(lineStatus.AvailableWidth, contentWidth) - 1.f);
    }

    // In case of this unlikely scenario where we couldn't find a fitting setup, let's just go with the last
    // result -this will most likely produce decoration overflow which may be correct in some cases (e.g. 0px
    // available space)
    return GetInlineContentBreaker().ProcessInlineContent(continuousInlineContent, lineStatus);
  }

  void LineBuilder::CommitCandidateContent(
    LineCandidate &lineCandidate,
    Maybe<InlineContentBreaker::Result::PartialTrailingContent> partialTrailingContent) noexcept
  {
    using RunShapingBoundary = InlineContentBreaker::ContinuousContent::Run::ShapingBoundary;
    auto &inlineContent = lineCandidate.Content;
    auto &runs = inlineContent.GetContinuousContent().Runs();
    if (runs.empty())
    {
      krys_debug_assert(!partialTrailingContent);
      return;
    }

    auto shapingBoundaryStart = Maybe<size_t> {};
    auto appendRun = [&](size_t index)
    {
      auto &run = runs[index];
      auto &inlineItem = run.Item;

      if (inlineItem.BidiLevel() != UBIDI_DEFAULT_LTR)
      {
        _line.SetContentNeedsBidiReordering();
      }

      if (auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItem))
      {
        auto shapingBoundary = [&]() -> Maybe<Line::ShapingBoundary>
        {
          if (!GetLayoutState().ShouldShapeTextAcrossInlineBoxes())
          {
            return {};
          }

          // Special case trailing partial run as shaping end.
          if (shapingBoundaryStart && partialTrailingContent
              && partialTrailingContent->TrailingRunIndex == index)
          {
            return Line::ShapingBoundary::End;
          }

          if (run.Shaping == RunShapingBoundary::Start)
          {
            krys_debug_assert(!shapingBoundaryStart);
            shapingBoundaryStart = index;
            return Line::ShapingBoundary::Start;
          }

          if (run.Shaping == RunShapingBoundary::End)
          {
            krys_debug_assert(shapingBoundaryStart);
            shapingBoundaryStart = {};
            return Line::ShapingBoundary::End;
          }

          if (shapingBoundaryStart)
          {
            return Line::ShapingBoundary::Inside;
          }

          return {};
        };
        _line.AppendText(*inlineTextItem, run.Style, run.ContentWidth(), shapingBoundary());
        return;
      }

      if (inlineItem.IsLineBreak())
      {
        _line.AppendLineBreak(inlineItem, run.Style);
        return;
      }

      if (inlineItem.IsWordBreakOpportunity())
      {
        _line.AppendWordBreakOpportunity(inlineItem, run.Style);
        return;
      }

      if (inlineItem.IsInlineBoxStart())
      {
        _line.AppendInlineBoxStart(inlineItem, run.Style, run.ContentWidth(), run.TextSpacingAdjustment);
        return;
      }

      if (inlineItem.IsInlineBoxEnd())
      {
        _line.AppendInlineBoxEnd(inlineItem, run.Style, run.ContentWidth());
        return;
      }

      if (inlineItem.IsAtomicInlineBox())
      {
        _line.AppendAtomicInlineBox(inlineItem, run.Style, run.ContentWidth());
        return;
      }

      if (inlineItem.IsOpaque())
      {
        krys_debug_assert(!run.ContentWidth());
        _line.AppendOpaqueBox(inlineItem, run.Style);
        return;
      }

      krys_debug_assert(false);
    };

    if (partialTrailingContent && inlineContent.GetContinuousContent().HasShapedContent())
    {
      ShapePartialLineCandidate(lineCandidate, partialTrailingContent->TrailingRunIndex);
    }

    krys_debug_assert(!partialTrailingContent || partialTrailingContent->TrailingRunIndex <= runs.size());
    auto endOfNonPartialContent =
      (partialTrailingContent ? std::min(partialTrailingContent->TrailingRunIndex, runs.size())
                              : runs.size());
    for (size_t index = 0; index < endOfNonPartialContent; ++index)
    {
      appendRun(index);
    }

    if (partialTrailingContent)
    {
      auto trailingRunIndex = partialTrailingContent->TrailingRunIndex;
      if (trailingRunIndex >= runs.size())
      {
        krys_debug_assert(false);
        return;
      }

      if (auto partialRun = partialTrailingContent->Partial)
      {
        // Create and commit partial trailing item.
        if (auto *trailingInlineTextItem = DynamicDowncast<InlineTextItem>(runs[trailingRunIndex].Item))
        {
          auto partialTrailingTextItem =
            trailingInlineTextItem->Left(static_cast<uint32>(partialRun->Length));
          _line.AppendText(partialTrailingTextItem, trailingInlineTextItem->Style(), partialRun->LogicalWidth,
                           shapingBoundaryStart ? Maybe<Line::ShapingBoundary> {Line::ShapingBoundary::End}
                                                : null);
          if (trailingInlineTextItem->BidiLevel() != UBIDI_DEFAULT_LTR)
          {
            _line.SetContentNeedsBidiReordering();
          }
        }
        else
        {
          krys_debug_assert(false);
        }

        if (auto hyphenWidth = partialRun->HyphenWidth)
        {
          _line.AddTrailingHyphen(*hyphenWidth);
        }
      }
      else
      {
        appendRun(trailingRunIndex);
        if (auto hyphenWidth = partialTrailingContent->HyphenWidth)
        {
          _line.AddTrailingHyphen(*hyphenWidth);
        }
      }
    }
  }

  LineBuilder::Result
    LineBuilder::ProcessLineBreakingResult(LineCandidate &lineCandidate, const InlineItemRange &layoutRange,
                                           const InlineContentBreaker::Result &lineBreakingResult) noexcept
  {
    using Action = InlineContentBreaker::Result::Action;
    using IsEndOfLine = InlineContentBreaker::IsEndOfLine;
    auto &candidateRuns = lineCandidate.Content.GetContinuousContent().Runs();

    switch (lineBreakingResult.ResultAction)
    {
      case Action::Keep:
      {
        // This continuous content can be fully placed on the current line.
        CommitCandidateContent(lineCandidate, lineBreakingResult.PartialTrailing);
        // We are keeping this content on the line but we need to check if we could have wrapped here
        // in order to be able to revert back to this position if needed.
        // Let's just ignore cases like collapsed leading whitespace for now.
        if (lineCandidate.Content.HasTrailingSoftWrapOpportunity() && _line.HasContentOrListMarker())
        {
          auto &trailingRun = candidateRuns.back();
          auto &trailingInlineItem = trailingRun.Item;

          // Note that wrapping here could be driven both by the style of the parent and the inline item
          // itself. e.g inline boxes set the wrapping rules for their content and not for themselves.
          auto &layoutBoxParent = trailingInlineItem.LayoutBox().Parent();

          // Need to ensure we use the correct style here, so the content breaker and line builder remain in
          // sync.
          auto &parentStyle =
            IsFirstFormattedLineCandidate() ? layoutBoxParent.FirstLineStyle() : layoutBoxParent.Style();

          auto isWrapOpportunity = TextUtil::IsWrappingAllowed(parentStyle);
          if (!isWrapOpportunity && trailingInlineItem.IsInlineBoxStartOrEnd())
          {
            isWrapOpportunity = TextUtil::IsWrappingAllowed(trailingRun.Style);
          }
          if (isWrapOpportunity)
          {
            _wrapOpportunityList.push_back(&trailingInlineItem);
          }
        }
        return {lineBreakingResult.EndOfLine, {candidateRuns.size(), false}};
      }
      case Action::Wrap:
      {
        krys_debug_assert(lineBreakingResult.EndOfLine == IsEndOfLine::Yes);
        // This continuous content can't be placed on the current line. Nothing to commit at this time.
        // However there are cases when, due to whitespace collapsing, this overflowing content should not be
        // separated from the content on the line. <div>X <span> X</span></div> If the second 'X' overflows
        // the line, the trailing whitespace gets trimmed which introduces a stray inline box on the first
        // line ('X <span>' and 'X</span>' first and second line respectively). In such cases we need to
        // revert the content on the line to a previous wrapping opportunity to keep such content together.
        auto needsRevert =
          _line.TrimmableTrailingWidth() && !_line.Runs().empty() && _line.Runs().back().IsInlineBoxStart();
        if (needsRevert && _wrapOpportunityList.size() > 1)
        {
          _wrapOpportunityList.pop_back();
          return {IsEndOfLine::Yes,
                  {RebuildLineWithInlineContent(layoutRange, *_wrapOpportunityList.back()), true}};
        }
        return {
          IsEndOfLine::Yes, {}, {}, OverflowWidthAsLeadingForNextLine(candidateRuns, lineBreakingResult)};
      }
      case Action::WrapWithHyphen:
        krys_debug_assert(lineBreakingResult.EndOfLine == IsEndOfLine::Yes);
        // This continuous content can't be placed on the current line, nothing to commit.
        // However we need to make sure that the current line gains a trailing hyphen.
        krys_debug_assert(_line.TrailingSoftHyphenWidth());
        _line.AddTrailingHyphen(*_line.TrailingSoftHyphenWidth());
        return {IsEndOfLine::Yes};
      case Action::RevertToLastWrapOpportunity:
        krys_debug_assert(lineBreakingResult.EndOfLine == IsEndOfLine::Yes);
        // Not only this content can't be placed on the current line, but we even need to revert the line back
        // to an earlier position.
        krys_debug_assert(!_wrapOpportunityList.empty());
        return {IsEndOfLine::Yes,
                {RebuildLineWithInlineContent(layoutRange, *_wrapOpportunityList.back()), true}};
      case Action::RevertToLastNonOverflowingWrapOpportunity:
        krys_debug_assert(lineBreakingResult.EndOfLine == IsEndOfLine::Yes);
        krys_debug_assert(!_wrapOpportunityList.empty());
        if (auto committedCount = RebuildLineForTrailingSoftHyphen(layoutRange))
        {
          return {IsEndOfLine::Yes, {committedCount, true}};
        }
        return {IsEndOfLine::Yes};
      case Action::Break:
      {
        krys_debug_assert(lineBreakingResult.EndOfLine == IsEndOfLine::Yes);
        // Commit the combination of full and partial content on the current line.
        krys_debug_assert(lineBreakingResult.PartialTrailing);
        CommitCandidateContent(lineCandidate, lineBreakingResult.PartialTrailing);
        // When breaking multiple runs <span style="word-break: break-all">text</span><span>content</span>, we
        // might end up breaking them at run boundary. It simply means we don't really have a partial run.
        // Partial content yes, but not partial run.
        auto trailingRunIndex = lineBreakingResult.PartialTrailing->TrailingRunIndex;
        auto committedInlineItemCount = trailingRunIndex + 1;
        if (!lineBreakingResult.PartialTrailing->Partial)
        {
          return {IsEndOfLine::Yes, {committedInlineItemCount, false}};
        }

        auto partialRun = *lineBreakingResult.PartialTrailing->Partial;
        auto &trailingInlineTextItem = Downcast<InlineTextItem>(candidateRuns[trailingRunIndex].Item);
        krys_debug_assert(partialRun.Length < trailingInlineTextItem.Length());
        auto overflowLength = trailingInlineTextItem.Length() - partialRun.Length;
        return {IsEndOfLine::Yes,
                {committedInlineItemCount, false},
                overflowLength,
                OverflowWidthAsLeadingForNextLine(candidateRuns, lineBreakingResult)};
      }
    }
    krys_debug_assert(false);
    return {IsEndOfLine::No};
  }

  size_t LineBuilder::RebuildLineWithInlineContent(const InlineItemRange &layoutRange,
                                                   const InlineItem &lastInlineItemToAdd) noexcept
  {
    krys_debug_assert(!_wrapOpportunityList.empty());
    _line.Initialize(_lineSpanningInlineBoxes, IsFirstFormattedLineCandidate());

    size_t numberOfFloatsInRange = 0;
    auto endOfCandidateContent = layoutRange.StartIndex();
    for (; endOfCandidateContent < layoutRange.EndIndex(); ++endOfCandidateContent)
    {
      if (_inlineItemList[endOfCandidateContent].IsFloat())
      {
        ++numberOfFloatsInRange;
      }
      if (&_inlineItemList[endOfCandidateContent] == &lastInlineItemToAdd)
      {
        ++endOfCandidateContent;
        break;
      }
    }
    krys_debug_assert(endOfCandidateContent < layoutRange.EndIndex());

    LineCandidate lineCandidate;
    auto candidateStartEndIndex = std::pair<size_t, size_t> {layoutRange.StartIndex(), endOfCandidateContent};
    // We might already have added floats. They shrink the available horizontal space for the line.
    // Let's just reuse what the line has at this point.
    CandidateContentForLine(lineCandidate, candidateStartEndIndex, layoutRange, _line.ContentLogicalRight(),
                            SkipFloats::Yes);
    auto result = ProcessLineBreakingResult(
      lineCandidate, layoutRange,
      {InlineContentBreaker::Result::Action::Keep, InlineContentBreaker::IsEndOfLine::Yes, {}, {}});

    // Remove floats that are outside of this "rebuild" range to ensure we don't add them twice.
    auto unplaceFloatBox = [&](const Box &floatBox) -> bool
    {
      auto placedFloat =
        std::find_if(_placedFloats.begin(), _placedFloats.end(),
                     [&floatBox](auto &placedFloatItem) { return placedFloatItem.LayoutBox() == &floatBox; });
      if (placedFloat != _placedFloats.end())
      {
        _placedFloats.erase(placedFloat);
      }
      return GetLayoutState().GetPlacedFloats().Remove(floatBox);
    };
    for (auto index = endOfCandidateContent; index < layoutRange.EndIndex(); ++index)
    {
      auto &inlineItem = _inlineItemList[index];
      if (inlineItem.IsFloat() && unplaceFloatBox(inlineItem.LayoutBox()))
      {
        break;
      }
    }

    return result.CommittedCount.Value + numberOfFloatsInRange;
  }

  size_t LineBuilder::RebuildLineForTrailingSoftHyphen(const InlineItemRange &layoutRange) noexcept
  {
    if (_wrapOpportunityList.empty())
    {
      // We are supposed to have a wrapping opportunity on the current line at this point.
      krys_debug_assert(false);
      return {};
    }
    // Revert all the way back to a wrap opportunity when either a soft hyphen fits or no hyphen is required.
    for (auto i = _wrapOpportunityList.size(); i-- > 1;)
    {
      auto &softWrapOpportunityItem = *_wrapOpportunityList[i];
      // FIXME: If this turns out to be a perf issue, we could also traverse the wrap list and keep adding the
      // items while watching the available width very closely.
      auto committedCount = RebuildLineWithInlineContent(layoutRange, softWrapOpportunityItem);
      auto availableWidth = _lineLogicalRect.Width() - _line.ContentLogicalRight();
      auto trailingSoftHyphenWidth = _line.TrailingSoftHyphenWidth();
      // Check if the trailing hyphen now fits the line (or we don't need hyphen anymore).
      if (!trailingSoftHyphenWidth || *trailingSoftHyphenWidth <= availableWidth)
      {
        if (trailingSoftHyphenWidth)
        {
          _line.AddTrailingHyphen(*trailingSoftHyphenWidth);
        }
        return committedCount;
      }
    }
    // Have at least some content on the line.
    auto committedCount = RebuildLineWithInlineContent(layoutRange, *_wrapOpportunityList.front());
    if (auto trailingSoftHyphenWidth = _line.TrailingSoftHyphenWidth())
    {
      _line.AddTrailingHyphen(*trailingSoftHyphenWidth);
    }
    return committedCount;
  }

  bool LineBuilder::IsLastLineWithInlineContent(const LineContent &lineContent, size_t needsLayoutEnd,
                                                const Line::RunList &lineRuns) const noexcept
  {
    if (lineContent.PartialTrailingContentLength)
    {
      return false;
    }
    // FIXME: This needs work with partial layout.
    auto &formattingContext = GetFormattingContext();
    if (lineContent.Range.EndIndex() == needsLayoutEnd)
    {
      if (!lineContent.Range.Start)
      {
        // This is both the first and the last line.
        return true;
      }
      for (auto &lineRun : lineRuns | std::views::reverse)
      {
        if (Line::Run::IsContentfulOrHasDecoration(lineRun, formattingContext))
        {
          return true;
        }
      }
      return false;
    }
    // Look ahead to see if there's more inline type of inline items.
    for (auto i = lineContent.Range.EndIndex(); i < needsLayoutEnd && i < _inlineItemList.size(); ++i)
    {
      if (IsContentfulOrHasDecoration(_inlineItemList[i], formattingContext))
      {
        return false;
      }
    }
    return true;
  }
}
