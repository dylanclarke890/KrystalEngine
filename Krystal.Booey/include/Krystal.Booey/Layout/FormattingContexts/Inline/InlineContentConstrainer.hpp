#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/FormattingConstraints.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingUtils.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineTypes.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <limits>

namespace krys::boo::layout
{
  class InlineFormattingContext;

  /// @brief Computes per-line available widths for `text-wrap: balance` and `text-wrap: pretty`, by choosing
  /// line breaks that minimise the raggedness of the paragraph.
  class InlineContentConstrainer
  {
    friend class SlidingWidth;

  private:
    struct EntryBalance
    {
      float AccumulatedCost {std::numeric_limits<float>::infinity()};
      size_t PreviousBreakIndex {0};
    };

    struct EntryPretty
    {
      float AccumulatedCost {std::numeric_limits<float>::infinity()};
      size_t PreviousBreakIndex {0};
      size_t LineIndex {0};
      InlineLayoutUnit LastLineWidth {0};
      InlineItemPosition LineEnd {};
      Maybe<PreviousLine> PreviousLineInfo {};
    };

    InlineFormattingContext &_inlineFormattingContext;
    const InlineItemList &_inlineItemList;
    const HorizontalConstraints _horizontalConstraints;

    List<InlineItemRange> _originalLineInlineItemRanges;
    List<LayoutUnit> _originalLineConstraints;
    LayoutUnit _maximumLineWidthConstraint {0};
    List<bool> _originalLineEndsWithForcedBreak;
    InlineLayoutUnit _inlineItemWidthsMax {0};
    List<InlineLayoutUnit> _inlineItemWidths;
    List<InlineLayoutUnit> _firstLineStyleInlineItemWidths;
    size_t _numberOfLinesInOriginalLayout {0};
    size_t _numberOfInlineItems {0};
    bool _cannotConstrainContent {false};
    bool _hasSingleLineVisibleContent {false};
    bool _hasValidInlineItemWidthCache {false};

  public:
    InlineContentConstrainer(InlineFormattingContext &inlineFormattingContext,
                             const InlineItemList &inlineItemList,
                             HorizontalConstraints horizontalConstraints) noexcept;

    KRYS_NODISCARD Maybe<List<LayoutUnit>> ComputeParagraphLevelConstraints(TextWrapStyle wrapStyle) noexcept;

  private:
    void Initialize() noexcept;
    void UpdateCachedWidths() noexcept;
    void CheckCanConstrainInlineItems() noexcept;
    KRYS_NODISCARD EntryPretty LayoutSingleLineForPretty(InlineItemRange layoutRange,
                                                         InlineLayoutUnit idealLineWidth,
                                                         const EntryPretty &lastValidEntry,
                                                         size_t previousBreakIndex) noexcept;

    KRYS_NODISCARD Maybe<List<LayoutUnit>> BalanceRangeWithLineRequirement(InlineItemRange range,
                                                                           InlineLayoutUnit idealLineWidth,
                                                                           size_t numberOfLines,
                                                                           bool isFirstChunk) noexcept;
    KRYS_NODISCARD Maybe<List<LayoutUnit>> BalanceRangeWithNoLineRequirement(InlineItemRange range,
                                                                             InlineLayoutUnit idealLineWidth,
                                                                             bool isFirstChunk) noexcept;
    KRYS_NODISCARD Maybe<List<LayoutUnit>>
      PrettifyRange(InlineItemRange range, InlineLayoutUnit idealLineWidth, bool isFirstChunk) noexcept;

    KRYS_NODISCARD InlineLayoutUnit InlineItemWidth(size_t inlineItemIndex,
                                                    bool useFirstLineStyle) const noexcept;
    KRYS_NODISCARD bool ShouldTrimLeading(size_t inlineItemIndex, bool useFirstLineStyle,
                                          bool isFirstLineInChunk) const noexcept;
    KRYS_NODISCARD bool ShouldTrimTrailing(size_t inlineItemIndex, bool useFirstLineStyle) const noexcept;
    KRYS_NODISCARD List<size_t> ComputeBreakOpportunities(InlineItemRange range) const noexcept;
    KRYS_NODISCARD List<LayoutUnit> ComputeLineWidthsFromBreaks(InlineItemRange inlineItems,
                                                                const List<size_t> &breaks,
                                                                bool isFirstChunk) const noexcept;
    KRYS_NODISCARD InlineLayoutUnit ComputeMaxTextIndent() const noexcept;
    KRYS_NODISCARD InlineLayoutUnit ComputedTextIndent(
      IsFirstFormattedLine isFirstFormattedLine,
      Maybe<InlineFormattingUtils::LineEndsWithLineBreak> previousLineEndsWithLineBreak) const noexcept;
  };

  /// @brief The width of a window of inline items, without its leading and trailing trimmable content.
  class SlidingWidth
  {
  private:
    const InlineContentConstrainer &_inlineContentConstrainer;
#if KRYS_DEBUG
    const InlineItemList &_inlineItemList;
#endif
    size_t _start {0};
    size_t _end {0};
    bool _useFirstLineStyle {false};
    bool _isFirstLineInChunk {false};
    InlineLayoutUnit _totalWidth {0};
    InlineLayoutUnit _leadingTrimmableWidth {0};
    InlineLayoutUnit _trailingTrimmableWidth {0};
    Maybe<size_t> _firstLeadingNonTrimmedItem;

  public:
    SlidingWidth(const InlineContentConstrainer &inlineContentConstrainer,
                 const InlineItemList &inlineItemList, size_t start, size_t end, bool useFirstLineStyle,
                 bool isFirstLineInChunk) noexcept;
    KRYS_NODISCARD InlineLayoutUnit Width() const noexcept;
    void AdvanceStart() noexcept;
    void AdvanceStartTo(size_t newStart) noexcept;
    void AdvanceEnd() noexcept;
    void AdvanceEndTo(size_t newEnd) noexcept;
  };
}
