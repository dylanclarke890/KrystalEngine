#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Inline/AbstractLineBuilder.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentCache.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  class PlacedFloats;
  struct CandidateTextContent;
  struct TextOnlyLineBreakResult;

  /// @brief A faster line builder for content that is only text and line breaks with simple style.
  class TextOnlySimpleLineBuilder final : public AbstractLineBuilder
  {
  private:
    InlineLayoutUnit _trimmedTrailingWhitespaceWidth {0.f};
    Maybe<InlineLayoutUnit> _overflowContentLogicalWidth {};

  public:
    TextOnlySimpleLineBuilder(InlineFormattingContext &inlineFormattingContext, const ElementBox &rootBox,
                              HorizontalConstraints rootHorizontalConstraints,
                              const InlineItemList &inlineItemList) noexcept;
    KRYS_NODISCARD LineLayoutResult LayoutInlineContent(const LineInput &lineInput,
                                                        const Maybe<PreviousLine> &previousLine,
                                                        bool isFirstFormattedLineCandidate) noexcept final;

    KRYS_NODISCARD static bool
      IsEligibleForSimplifiedTextOnlyInlineLayoutByContent(const InlineContentCache::InlineItems &inlineItems,
                                                           const PlacedFloats &placedFloats) noexcept;
    KRYS_NODISCARD static bool IsEligibleForSimplifiedInlineLayoutByStyle(const Box &box) noexcept;

  private:
    KRYS_NODISCARD InlineItemPosition PlaceInlineTextContent(const RenderStyle &rootStyle,
                                                             const InlineItemRange &layoutRange) noexcept;
    KRYS_NODISCARD InlineItemPosition PlaceNonWrappingInlineTextContent(
      const RenderStyle &rootStyle, const InlineItemRange &layoutRange) noexcept;
    KRYS_NODISCARD Maybe<LineLayoutResult>
      PlaceSingleCharacterContentIfApplicable(const LineInput &lineInput,
                                              bool isFirstFormattedLineCandidate) noexcept;
    KRYS_NODISCARD TextOnlyLineBreakResult HandleOverflowingTextContent(
      const RenderStyle &rootStyle, const InlineContentBreaker::ContinuousContent &candidateContent,
      const InlineItemRange &layoutRange) noexcept;
    KRYS_NODISCARD TextOnlyLineBreakResult
      CommitCandidateContent(const RenderStyle &rootStyle, const CandidateTextContent &candidateContent,
                             const InlineItemRange &layoutRange) noexcept;
    void Initialize(const InlineItemRange &layoutRange, const InlineRect &initialLogicalRect,
                    const Maybe<PreviousLine> &previousLine, bool isFirstFormattedLineCandidate) noexcept;
    void HandleLineEnding(const RenderStyle &rootStyle, InlineItemPosition placedContentEnd,
                          size_t layoutRangeEndIndex) noexcept;
    KRYS_NODISCARD size_t RevertToTrailingItem(const RenderStyle &rootStyle,
                                               const InlineItemRange &layoutRange,
                                               const InlineTextItem &trailingInlineItem) noexcept;
    KRYS_NODISCARD size_t RevertToLastNonOverflowingItem(const RenderStyle &rootStyle,
                                                         const InlineItemRange &layoutRange) noexcept;
    KRYS_NODISCARD InlineLayoutUnit AvailableWidth() const noexcept;
  };
}
