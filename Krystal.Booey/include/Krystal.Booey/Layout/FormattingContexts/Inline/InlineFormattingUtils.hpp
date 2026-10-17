#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Graphics/FontBaseline.hpp"
#include "Krystal.Booey/Layout/Floats/FloatingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Block/BlockLayoutState.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLine.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineTypes.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineRect.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/LineLayoutResult.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo
{
  class FontMetrics;
}

namespace krys::boo::layout
{
  class InlineFormattingContext;
  class InlineLevelBox;

  /// @brief The geometry queries of inline layout: line positions, text indent, alignment, wrap
  /// opportunities, item widths and the snapping of vertical metrics.
  class InlineFormattingUtils
  {
  public:
    enum class IsIntrinsicWidthMode : bool
    {
      No,
      Yes
    };
    enum class LineEndsWithLineBreak : bool
    {
      No,
      Yes
    };
    enum class SnapDirection : uint8
    {
      Floor,
      Ceil,
      Round
    };

  private:
    const InlineFormattingContext &_inlineFormattingContext;

  public:
    explicit InlineFormattingUtils(const InlineFormattingContext &inlineFormattingContext) noexcept;

    KRYS_NODISCARD InlineLayoutUnit
      LogicalTopForNextLine(const LineLayoutResult &lineLayoutResult, const InlineRect &lineLogicalRect,
                            const FloatingContext &floatingContext,
                            const BlockLayoutState::MarginState &marginState) const noexcept;

    KRYS_NODISCARD InlineLayoutUnit
      ComputedTextIndent(IsIntrinsicWidthMode isIntrinsicWidthMode, IsFirstFormattedLine isFirstFormattedLine,
                         Maybe<LineEndsWithLineBreak> previousLineEndsWithLineBreak,
                         InlineLayoutUnit availableWidth) const noexcept;

    KRYS_NODISCARD bool InlineLevelBoxAffectsLineBox(const InlineLevelBox &inlineLevelBox) const noexcept;

    KRYS_NODISCARD InlineLayoutUnit InitialLineHeight(bool isFirstLine) const noexcept;

    KRYS_NODISCARD FloatingContext::Constraints
      FloatConstraintsForLine(InlineLayoutUnit lineLogicalTop, InlineLayoutUnit contentLogicalHeight,
                              const FloatingContext &floatingContext) const noexcept;

    KRYS_NODISCARD static InlineRect FlipVisualRectToLogicalForWritingMode(const InlineRect &visualRect,
                                                                           WritingMode writingMode) noexcept;

    KRYS_NODISCARD static InlineLayoutUnit
      HorizontalAlignmentOffset(const RenderStyle &rootStyle, InlineLayoutUnit contentLogicalRight,
                                InlineLayoutUnit lineLogicalWidth, InlineLayoutUnit hangingTrailingWidth,
                                bool isLastLineOrLineEndsWithForcedLineBreak,
                                Maybe<TextDirection> inlineBaseDirectionOverride = null) noexcept;

    KRYS_NODISCARD static InlineItemPosition LeadingInlineItemPositionForNextLine(
      InlineItemPosition lineContentEnd, Maybe<InlineItemPosition> previousLineContentEnd,
      bool lineHasIntrusiveOrNewlyPlacedFloat, InlineItemPosition layoutRangeEnd) noexcept;

    KRYS_NODISCARD InlineLayoutUnit InlineItemWidth(const InlineItem &inlineItem,
                                                    InlineLayoutUnit contentLogicalLeft,
                                                    bool useFirstLineStyle) const noexcept;

    KRYS_NODISCARD size_t NextWrapOpportunity(size_t startIndex, const InlineItemRange &layoutRange,
                                              Span<const InlineItem> inlineItemList) const noexcept;

    KRYS_NODISCARD static std::pair<InlineLayoutUnit, InlineLayoutUnit>
      TextEmphasisForInlineBox(const Box &layoutBox, const ElementBox &rootBox) noexcept;

    KRYS_NODISCARD static LineEndingTruncationPolicy
      GetLineEndingTruncationPolicy(const RenderStyle &rootStyle, size_t numberOfContentfulLines,
                                    Maybe<size_t> numberOfVisibleLinesAllowed,
                                    bool currentLineIsContentful) noexcept;

    KRYS_NODISCARD static Maybe<LineLayoutResult::InlineContentEnding>
      InlineContentEnding(const Line::Result &lineContent) noexcept;

    KRYS_NODISCARD bool ShouldDiscardRemainingContentInBlockDirection() const noexcept;

    KRYS_NODISCARD static InlineLayoutUnit SnapToInt(InlineLayoutUnit value,
                                                     const InlineLevelBox &inlineLevelBox,
                                                     SnapDirection direction = SnapDirection::Round) noexcept;
    KRYS_NODISCARD static InlineLayoutUnit SnapToInt(InlineLayoutUnit value, const Box &layoutBox,
                                                     SnapDirection direction = SnapDirection::Round) noexcept;

    KRYS_NODISCARD static InlineLayoutUnit Ascent(const FontMetrics &fontMetrics, FontBaseline fontBaseline,
                                                  const InlineLevelBox &inlineLevelBox) noexcept;
    KRYS_NODISCARD static InlineLayoutUnit Descent(const FontMetrics &fontMetrics, FontBaseline fontBaseline,
                                                   const InlineLevelBox &inlineLevelBox) noexcept;

    KRYS_NODISCARD static InlineLayoutUnit Ascent(const FontMetrics &fontMetrics, FontBaseline fontBaseline,
                                                  const Box &layoutBox) noexcept;
    KRYS_NODISCARD static InlineLayoutUnit Descent(const FontMetrics &fontMetrics, FontBaseline fontBaseline,
                                                   const Box &layoutBox) noexcept;

  private:
    KRYS_NODISCARD bool IsAtSoftWrapOpportunity(const InlineItem &previous,
                                                const InlineItem &next) const noexcept;

    KRYS_NODISCARD const InlineFormattingContext &GetFormattingContext() const noexcept
    {
      return _inlineFormattingContext;
    }
  };
}
