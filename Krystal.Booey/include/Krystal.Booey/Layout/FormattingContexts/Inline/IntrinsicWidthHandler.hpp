#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/FormattingConstraints.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentCache.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/LineLayoutResult.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  class AbstractLineBuilder;
  class InlineFormattingContext;

  /// @brief Computes the minimum and maximum content widths of inline content by laying it out with the line
  /// builders in intrinsic width mode, or with shortcuts for simple content.
  class IntrinsicWidthHandler
  {
    enum class MayCacheLayoutResult : bool
    {
      No,
      Yes
    };

  private:
    InlineFormattingContext &_inlineFormattingContext;
    const InlineContentCache::InlineItems &_inlineItems;
    InlineItemRange _inlineItemRange;
    bool _mayUseSimplifiedTextOnlyInlineLayoutInRange {false};

    Maybe<InlineLayoutUnit> _maximumContentWidthBetweenLineBreaks {};
    Maybe<LineLayoutResult> _maximumIntrinsicWidthResultForSingleLine {};

  public:
    IntrinsicWidthHandler(InlineFormattingContext &inlineFormattingContext,
                          const InlineContentCache::InlineItems &inlineItems) noexcept;

    KRYS_NODISCARD InlineLayoutUnit MinimumContentSize() noexcept;
    KRYS_NODISCARD InlineLayoutUnit MaximumContentSize() noexcept;

    KRYS_NODISCARD Maybe<LineLayoutResult> &MaximumIntrinsicWidthLineContent() noexcept
    {
      return _maximumIntrinsicWidthResultForSingleLine;
    }

  private:
    KRYS_NODISCARD InlineLayoutUnit ComputedIntrinsicWidthForConstraint(
      IntrinsicWidthMode intrinsicWidthMode, AbstractLineBuilder &lineBuilder,
      MayCacheLayoutResult mayCacheLayoutResult = MayCacheLayoutResult::No) noexcept;
    KRYS_NODISCARD InlineLayoutUnit SimplifiedMinimumWidth(const ElementBox &root) const noexcept;
    KRYS_NODISCARD InlineLayoutUnit
      SimplifiedMaximumWidth(MayCacheLayoutResult mayCacheLayoutResult = MayCacheLayoutResult::No) noexcept;

    KRYS_NODISCARD InlineFormattingContext &GetFormattingContext() noexcept
    {
      return _inlineFormattingContext;
    }
    KRYS_NODISCARD const InlineFormattingContext &GetFormattingContext() const noexcept
    {
      return _inlineFormattingContext;
    }
    KRYS_NODISCARD const ElementBox &FormattingContextRoot() const noexcept;
    KRYS_NODISCARD const ElementBox &LineBuilerRoot() const noexcept;
    KRYS_NODISCARD const InlineItemList &GetInlineItemList() const noexcept
    {
      return _inlineItems.Content();
    }
  };
}
