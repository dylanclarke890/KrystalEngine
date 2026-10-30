#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Inline/AbstractLineBuilder.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentCache.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/TextOnlySimpleLineBuilder.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::layout
{
  class PlacedFloats;

  /// @brief A line builder for text-only content wrapped in a single inline box (or inline boxes alone): it
  /// runs the text-only builder on the content and adds the inline box runs around it.
  class RangeBasedLineBuilder final : public AbstractLineBuilder
  {
  private:
    TextOnlySimpleLineBuilder _textOnlySimpleLineBuilder;
    size_t _inlineBoxCount {0};

  public:
    RangeBasedLineBuilder(InlineFormattingContext &inlineFormattingContext,
                          HorizontalConstraints rootHorizontalConstraints,
                          const InlineContentCache::InlineItems &inlineItems) noexcept;
    KRYS_NODISCARD LineLayoutResult LayoutInlineContent(const LineInput &lineInput,
                                                        const Maybe<PreviousLine> &previousLine,
                                                        bool isFirstFormattedLineCandidate) noexcept final;

    KRYS_NODISCARD static bool IsEligibleForRangeInlineLayout(
      const InlineFormattingContext &inlineFormattingContext, InlineItemRange needsLayoutRange,
      const InlineContentCache::InlineItems &inlineItems, const PlacedFloats &placedFloats) noexcept;
  };
}
