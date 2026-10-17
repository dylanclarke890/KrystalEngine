#pragma once

#include "Krystal.Booey/Layout/Floats/FloatingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/AbstractLineBuilder.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::layout
{
  struct LineContent;
  struct LineCandidate;

  /// @brief The general line builder: places inline items, floats and blocks in inline content on lines,
  /// breaking content that doesn't fit at its wrap opportunities.
  class LineBuilder final : public AbstractLineBuilder
  {
    enum class SkipFloats : bool
    {
      No,
      Yes
    };

    struct Result
    {
      InlineContentBreaker::IsEndOfLine EndOfLine {InlineContentBreaker::IsEndOfLine::No};
      struct CommittedContentCount
      {
        size_t Value {0};
        bool IsRevert {false};
      };
      CommittedContentCount CommittedCount {};
      size_t PartialTrailingContentLength {0};
      Maybe<InlineLayoutUnit> OverflowLogicalWidth {};
    };

    enum MayOverConstrainLine : uint8
    {
      No,
      Yes,
      OnlyWhenFirstFloatOnLine
    };

    struct RectAndFloatConstraints
    {
      InlineRect LogicalRect;
      EnumSet<UsedFloat> ConstrainedSideSet {};
    };

    struct InitialLetterOffsets
    {
      LayoutUnit CapHeightOffset;
      LayoutUnit SunkenBelowFirstLineOffset;
    };

  private:
    const FloatingContext &_floatingContext;
    InlineRect _lineInitialLogicalRect;
    InlineLayoutUnit _lineMarginStart {0.f};
    InlineLayoutUnit _initialIntrusiveFloatsWidth {0.f};
    InlineLayoutUnit _candidateContentMaximumHeight {0.f};
    LineLayoutResult::PlacedFloatList _placedFloats;
    LineLayoutResult::SuspendedFloatList _suspendedFloats;
    Maybe<InlineLayoutUnit> _overflowingLogicalWidth;
    SmallList<InlineItem, 1> _lineSpanningInlineBoxes;
    EnumSet<UsedFloat> _lineIsConstrainedByFloat {};
    Maybe<InlineLayoutUnit> _initialLetterClearGap;
    TextSpacingContext _textSpacingContext {};

  public:
    LineBuilder(InlineFormattingContext &inlineFormattingContext,
                HorizontalConstraints rootHorizontalConstraints, const InlineItemList &inlineItemList,
                TextSpacingContext textSpacingContext = {}) noexcept;
    ~LineBuilder() noexcept override;

    KRYS_NODISCARD LineLayoutResult LayoutInlineContent(const LineInput &lineInput,
                                                        const Maybe<PreviousLine> &previousLine,
                                                        bool isFirstFormattedLineCandidate) noexcept final;

  private:
    void CandidateContentForLine(LineCandidate &lineCandidate, std::pair<size_t, size_t> startEndIndex,
                                 const InlineItemRange &needsLayoutRange,
                                 InlineLayoutUnit currentLogicalRight,
                                 SkipFloats skipFloats = SkipFloats::No) noexcept;
    void ApplyShapingIfNeeded(LineCandidate &lineCandidate) noexcept;
    KRYS_NODISCARD List<std::pair<size_t, size_t>>
      CollectShapeRanges(const LineCandidate &lineCandidate) const noexcept;
    void ApplyShapingOnRunRange(LineCandidate &lineCandidate, std::pair<size_t, size_t> range) const noexcept;
    void ShapePartialLineCandidate(LineCandidate &lineCandidate, size_t trailingRunIndex) const noexcept;
    KRYS_NODISCARD InlineLayoutUnit
      LeadingPunctuationWidthForLineCandiate(const LineCandidate &lineCandidate) const noexcept;
    KRYS_NODISCARD InlineLayoutUnit TrailingPunctuationOrStopOrCommaWidthForLineCandiate(
      const LineCandidate &lineCandidate, size_t startIndexAfterCandidateContent,
      size_t layoutRangeEnd) const noexcept;

    KRYS_NODISCARD bool TryPlacingFloatBox(const Box &floatBox,
                                           MayOverConstrainLine mayOverConstrainLine) noexcept;
    KRYS_NODISCARD Result HandleInlineContent(const InlineItemRange &needsLayoutRange,
                                              LineCandidate &lineCandidate) noexcept;
    void HandleBlockContent(const InlineItem &blockItem) noexcept;
    KRYS_NODISCARD Result
      ProcessLineBreakingResult(LineCandidate &lineCandidate, const InlineItemRange &layoutRange,
                                const InlineContentBreaker::Result &lineBreakingResult) noexcept;
    KRYS_NODISCARD RectAndFloatConstraints FloatAvoidingRect(const InlineRect &lineLogicalRect,
                                                             InlineLayoutUnit lineMarginStart) const noexcept;
    KRYS_NODISCARD RectAndFloatConstraints
      AdjustedLineRectWithCandidateInlineContent(const LineCandidate &lineCandidate) const noexcept;

    KRYS_NODISCARD Result ApplyLineBreakingOnCandidateInlineContent(const InlineItemRange &needsLayoutRange,
                                                                    LineCandidate &lineCandidate) noexcept;
    void CommitCandidateContent(
      LineCandidate &lineCandidate,
      Maybe<InlineContentBreaker::Result::PartialTrailingContent> partialTrailingContent) noexcept;
    KRYS_NODISCARD size_t RebuildLineWithInlineContent(const InlineItemRange &needsLayoutRange,
                                                       const InlineItem &lastInlineItemToAdd) noexcept;
    KRYS_NODISCARD size_t RebuildLineForTrailingSoftHyphen(const InlineItemRange &layoutRange) noexcept;
    void Initialize(const InlineRect &initialLineLogicalRect, const InlineItemRange &needsLayoutRange,
                    const Maybe<PreviousLine> &previousLine, bool isFirstFormattedLineCandidate) noexcept;
    KRYS_NODISCARD UniquePtr<LineContent>
      PlaceInlineAndFloatContent(const InlineItemRange &needsLayoutRange) noexcept;
    KRYS_NODISCARD Maybe<InitialLetterOffsets>
      AdjustLineRectForInitialLetterIfApplicable(const Box &floatBox) noexcept;
    KRYS_NODISCARD bool IsLastLineWithInlineContent(const LineContent &lineContent, size_t needsLayoutEnd,
                                                    const Line::RunList &lineRuns) const noexcept;
    KRYS_NODISCARD InlineContentBreaker::Result
      HandleInlineContentWithClonedDecoration(const LineCandidate &lineCandidate,
                                              InlineContentBreaker::LineStatus lineStatus) noexcept;
    KRYS_NODISCARD InlineLayoutUnit ClonedDecorationAtBreakingPosition(
      const InlineContentBreaker::ContinuousContent::RunList &runs,
      const InlineContentBreaker::Result::PartialTrailingContent &trailingContent) const noexcept;
    KRYS_NODISCARD InlineLayoutUnit PlacedClonedDecorationWidth(
      const InlineContentBreaker::ContinuousContent::RunList &runs) const noexcept;

    KRYS_NODISCARD bool IsFloatLayoutSuspended() const noexcept
    {
      return !_suspendedFloats.empty();
    }
    KRYS_NODISCARD bool ShouldTryToPlaceFloatBox(const Box &floatBox, LayoutUnit floatBoxMarginBoxWidth,
                                                 MayOverConstrainLine mayOverConstrainLine) const noexcept;

    KRYS_NODISCARD bool IsLineConstrainedByFloat() const noexcept
    {
      return !_lineIsConstrainedByFloat.IsEmpty();
    }
    KRYS_NODISCARD const FloatingContext &GetFloatingContext() const noexcept
    {
      return _floatingContext;
    }
  };
}
