#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/FormattingConstraints.hpp"
#include "Krystal.Booey/Layout/LayoutUnits.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::layout
{
  class InlineItem;
  class InlineTextItem;
  struct CandidateTextRunForBreaking;

  /// @brief Decides what happens to content that doesn't fit on a line: whether it stays, breaks (and
  /// where), wraps to the next line or reverts to an earlier wrap opportunity.
  class InlineContentBreaker
  {
  public:
    struct PartialRun
    {
      size_t Length {0};
      InlineLayoutUnit LogicalWidth {0};
      // FIXME: Remove this and collapse the rest of PartialRun over to PartialTrailingContent.
      Maybe<InlineLayoutUnit> HyphenWidth {};
    };

    enum class IsEndOfLine : bool
    {
      No,
      Yes
    };

    struct Result
    {
      enum class Action : uint8
      {
        Keep,           // Keep content on the current line.
        Break,          // Partial content is on the current line.
        Wrap,           // Content is wrapped to the next line.
        WrapWithHyphen, // Content is wrapped to the next line and the current line ends with a visible
                        // hyphen.
        // The current content overflows and can't get broken up into smaller bits.
        RevertToLastWrapOpportunity, // The content needs to be reverted back to the last wrap opportunity.
        RevertToLastNonOverflowingWrapOpportunity // The content needs to be reverted back to a wrap
                                                  // opportunity that still fits the line.
      };
      struct PartialTrailingContent
      {
        size_t TrailingRunIndex {0};
        Maybe<PartialRun> Partial; // nullopt partial run means the trailing run is a complete run.
        // Hyphen may be at the end of a full run in the middle of the continuous content (e.g. with adjacent
        // InlineTextItems).
        Maybe<InlineLayoutUnit> HyphenWidth {};
      };
      Action ResultAction {Action::Keep};
      IsEndOfLine EndOfLine {IsEndOfLine::No};
      Maybe<PartialTrailingContent> PartialTrailing {};
      const InlineItem *LastWrapOpportunityItem {nullptr};
    };

    // This struct represents the amount of continuous content committed to content breaking at a time (no
    // in-between wrap opportunities). e.g. <div>text content
    // <span>span1</span>between<span>span2</span></div> [text][ ][content][ ][inline box start][span1][inline
    // box end][between][inline box start][span2][inline box end] continuous candidate content at a time:
    // 1. [text]
    // 2. [ ]
    // 3. [content]
    // 4. [ ]
    // 5. [inline box start][span1][inline box end][between][inline box start][span2][inline box end]
    // see https://drafts.csswg.org/css-text-3/#line-break-details
    struct ContinuousContent
    {
      struct Run
      {
        Run(const InlineItem &inlineItem, const RenderStyle &style, InlineLayoutUnit offset,
            InlineLayoutUnit contentWidth, InlineLayoutUnit textSpacingAdjustment = 0.f) noexcept
            : Item(inlineItem), Style(style), Offset(offset), TextSpacingAdjustment(textSpacingAdjustment),
              _contentWidth(contentWidth)
        {
        }

        KRYS_NODISCARD InlineLayoutUnit SpaceRequired() const noexcept
        {
          return Offset + ContentWidth();
        }
        void AdjustContentWidth(InlineLayoutUnit contentWidth) noexcept
        {
          _contentWidth = contentWidth;
        }
        KRYS_NODISCARD InlineLayoutUnit ContentWidth() const noexcept
        {
          return _contentWidth;
        }

        const InlineItem &Item;
        const RenderStyle &Style;
        InlineLayoutUnit Offset {0};
        InlineLayoutUnit TextSpacingAdjustment {0};
        enum class ShapingBoundary : bool
        {
          Start,
          End
        };
        Maybe<ShapingBoundary> Shaping {};

      private:
        InlineLayoutUnit _contentWidth {0};
      };
      using RunList = SmallList<Run, 3>;

    private:
      RunList _runs;
      InlineLayoutUnit _logicalWidth {0.f};
      InlineLayoutUnit _leadingTrimmableWidth {0.f};
      InlineLayoutUnit _trailingTrimmableWidth {0.f};
      Maybe<InlineLayoutUnit> _hangingContentWidth {};
      Maybe<InlineLayoutUnit> _minimumRequiredWidth {};
      bool _hasTextContent {false};
      bool _isTextOnlyContent {true};
      bool _isFullyTrimmable {false};
      bool _hasTrailingWordSeparator {false};
      bool _hasTrailingSoftHyphen {false};
      bool _hasShapedContent {false};

    public:
      KRYS_NODISCARD InlineLayoutUnit LogicalWidth() const noexcept
      {
        return _logicalWidth;
      }
      void AdjustLogicalWidth(InlineLayoutUnit logicalWidth) noexcept
      {
        _logicalWidth = logicalWidth;
      }
      KRYS_NODISCARD Maybe<InlineLayoutUnit> MinimumRequiredWidth() const noexcept
      {
        return _minimumRequiredWidth;
      }
      KRYS_NODISCARD InlineLayoutUnit LeadingTrimmableWidth() const noexcept
      {
        return _leadingTrimmableWidth;
      }
      KRYS_NODISCARD InlineLayoutUnit TrailingTrimmableWidth() const noexcept
      {
        return _trailingTrimmableWidth;
      }
      KRYS_NODISCARD InlineLayoutUnit HangingContentWidth() const noexcept
      {
        return _hangingContentWidth.value_or(0.f);
      }
      KRYS_NODISCARD bool HasTrimmableSpace() const noexcept
      {
        return TrailingTrimmableWidth() || LeadingTrimmableWidth();
      }
      KRYS_NODISCARD bool HasHangingSpace() const noexcept
      {
        return HangingContentWidth() != 0.f;
      }
      KRYS_NODISCARD bool HasTrailingSoftHyphen() const noexcept
      {
        return _hasTrailingSoftHyphen;
      }
      KRYS_NODISCARD bool HasTextContent() const noexcept
      {
        return _hasTextContent;
      }
      KRYS_NODISCARD bool IsTextOnlyContent() const noexcept
      {
        return _isTextOnlyContent;
      }
      KRYS_NODISCARD bool IsFullyTrimmable() const noexcept
      {
        return _isFullyTrimmable;
      }
      KRYS_NODISCARD bool IsHangingContent() const noexcept
      {
        return _hangingContentWidth && *_hangingContentWidth == LogicalWidth();
      }

      void Append(const InlineItem &inlineItem, const RenderStyle &style, InlineLayoutUnit logicalWidth,
                  InlineLayoutUnit textSpacingAdjustment = 0.f) noexcept;
      void AppendTextContent(const InlineTextItem &inlineTextItem, const RenderStyle &style,
                             InlineLayoutUnit logicalWidth) noexcept;
      void SetHangingContentWidth(InlineLayoutUnit logicalWidth) noexcept
      {
        _hangingContentWidth = logicalWidth;
      }
      void SetTrailingSoftHyphenWidth(InlineLayoutUnit hyphenWidth) noexcept;
      void SetMinimumRequiredWidth(InlineLayoutUnit minimumRequiredWidth) noexcept
      {
        _minimumRequiredWidth = minimumRequiredWidth;
      }
      void SetHasShapedContent() noexcept
      {
        _hasShapedContent = true;
      }
      KRYS_NODISCARD bool HasShapedContent() const noexcept
      {
        return _hasShapedContent;
      }
      void Reset() noexcept;

      KRYS_NODISCARD const RunList &Runs() const noexcept
      {
        return _runs;
      }
      KRYS_NODISCARD RunList &Runs() noexcept
      {
        return _runs;
      }

    private:
      void AppendToRunList(const InlineItem &inlineItem, const RenderStyle &style, InlineLayoutUnit offset,
                           InlineLayoutUnit contentWidth,
                           InlineLayoutUnit textSpacingAdjustment = 0.f) noexcept;
      void ResetTrailingTrimmableContent() noexcept;
    };

    struct LineStatus
    {
      InlineLayoutUnit ContentLogicalRight {0};
      InlineLayoutUnit AvailableWidth {0};
      // Both of these types of trailing content may be ignored when checking for content fit.
      InlineLayoutUnit TrimmableOrHangingWidth {0};
      Maybe<InlineLayoutUnit> TrailingSoftHyphenWidth;
      bool HasFullyTrimmableTrailingContent {false};
      bool HasContent {false};
      bool HasWrapOpportunityAtPreviousPosition {false};
    };

  private:
    struct OverflowingTextContent
    {
      size_t RunIndex {0}; // Overflowing run index. There's always an overflowing run.
      struct BreakingPosition
      {
        size_t RunIndex {0};
        struct TrailingContent
        {
          // Trailing content is either the run's left side (when we break the run somewhere in the middle) or
          // the previous run. Sometimes the breaking position is at the very beginning of the first run, so
          // there's no trailing run at all.
          bool Overflows {false};
          Maybe<InlineContentBreaker::PartialRun> Partial {};
          Maybe<InlineLayoutUnit> HyphenWidth {};
        };
        Maybe<TrailingContent> Trailing {};
      };
      Maybe<BreakingPosition> Breaking {}; // Where we actually break this overflowing content.
    };

    enum class WordBreakRule : uint8
    {
      AtArbitraryPositionWithinWords,
      AtArbitraryPosition,
      AtHyphenationOpportunities
    };

    bool _isMinimumInIntrinsicWidthMode {false};
    bool _hyphenationIsDisabled {false};

  public:
    KRYS_NODISCARD Result ProcessInlineContent(const ContinuousContent &candidateContent,
                                               const LineStatus &lineStatus) noexcept;
    void SetHyphenationDisabled(bool hyphenationIsDisabled) noexcept
    {
      _hyphenationIsDisabled = hyphenationIsDisabled;
    }
    void SetIsMinimumInIntrinsicWidthMode(bool isMinimumInIntrinsicWidthMode) noexcept
    {
      _isMinimumInIntrinsicWidthMode = isMinimumInIntrinsicWidthMode;
    }

  private:
    KRYS_NODISCARD Result ProcessOverflowingContent(const ContinuousContent &continuousContent,
                                                    const LineStatus &lineStatus) const noexcept;
    KRYS_NODISCARD OverflowingTextContent ProcessOverflowingContentWithText(
      const ContinuousContent &continuousContent, const LineStatus &lineStatus) const noexcept;
    KRYS_NODISCARD Maybe<Result>
      SimplifiedMinimumInstrinsicWidthBreak(const ContinuousContent &candidateContent,
                                            const LineStatus &lineStatus) const noexcept;
    KRYS_NODISCARD Maybe<PartialRun> TryBreakingTextRun(const ContinuousContent::RunList &runs,
                                                        const CandidateTextRunForBreaking &candidateTextRun,
                                                        InlineLayoutUnit availableWidth,
                                                        const LineStatus &lineStatus) const noexcept;
    KRYS_NODISCARD Maybe<OverflowingTextContent::BreakingPosition>
      TryBreakingOverflowingRun(const LineStatus &lineStatus, const ContinuousContent::RunList &runs,
                                size_t overflowingRunIndex,
                                InlineLayoutUnit nonOverflowingContentWidth) const noexcept;
    KRYS_NODISCARD Maybe<OverflowingTextContent::BreakingPosition> TryBreakingPreviousNonOverflowingRuns(
      const LineStatus &lineStatus, const ContinuousContent::RunList &runs, size_t overflowingRunIndex,
      InlineLayoutUnit nonOverflowingContentWidth) const noexcept;
    KRYS_NODISCARD Maybe<OverflowingTextContent::BreakingPosition>
      TryBreakingNextOverflowingRuns(const LineStatus &lineStatus, const ContinuousContent::RunList &runs,
                                     size_t overflowingRunIndex,
                                     InlineLayoutUnit nonOverflowingContentWidth) const noexcept;
    KRYS_NODISCARD Maybe<OverflowingTextContent::BreakingPosition>
      TryHyphenationAcrossOverflowingInlineTextItems(const LineStatus &lineStatus,
                                                     const ContinuousContent::RunList &runs,
                                                     size_t overflowingRunIndex) const noexcept;

    KRYS_NODISCARD EnumSet<WordBreakRule>
      WordBreakBehavior(const RenderStyle &style, bool hasWrapOpportunityAtPreviousPosition) const noexcept;
    KRYS_NODISCARD bool IsMinimumInIntrinsicWidthMode() const noexcept
    {
      return _isMinimumInIntrinsicWidthMode;
    }
  };
}
