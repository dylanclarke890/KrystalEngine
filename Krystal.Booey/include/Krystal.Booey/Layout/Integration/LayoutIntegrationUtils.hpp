#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Block/BlockLayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::layout
{
  class BoxGeometry;
  class ElementBox;
  class InlineLayoutState;
  class LayoutState;

  /// @brief How the layout formatting contexts reach back into render tree layout: laying out and measuring
  /// the formatting context roots they contain, and converting block layout state between the two.
  class IntegrationUtils
  {
  private:
    const CheckedRef<const LayoutState> _globalLayoutState;

  public:
    explicit IntegrationUtils(const LayoutState &globalLayoutState) noexcept;

    void LayoutWithFormattingContextForBox(const ElementBox &box, Maybe<LayoutUnit> widthConstraint = null,
                                           Maybe<LayoutUnit> heightConstraint = null) const noexcept;
    KRYS_NODISCARD LayoutUnit MaxContentWidth(const ElementBox &box) const noexcept;
    KRYS_NODISCARD LayoutUnit MinContentWidth(const ElementBox &box) const noexcept;
    KRYS_NODISCARD LayoutUnit MinContentHeight(const ElementBox &box) const noexcept;
    KRYS_NODISCARD LayoutUnit PreferredMinWidth(const ElementBox &box) const noexcept;
    KRYS_NODISCARD LayoutUnit PreferredMaxWidth(const ElementBox &box) const noexcept;

    void
      LayoutWithFormattingContextForBlockInInline(const ElementBox &block,
                                                  LayoutPoint blockLineLogicalTopLeft,
                                                  const InlineLayoutState &inlineLayoutState) const noexcept;

    KRYS_NODISCARD static BlockLayoutState::MarginState
      ToMarginState(const RenderBlockFlow::MarginInfo &marginInfo,
                    LayoutUnit contentOffsetAfterSelfCollapsingBlock) noexcept;
    KRYS_NODISCARD static RenderBlockFlow::MarginInfo
      ToMarginInfo(const BlockLayoutState::MarginState &marginState) noexcept;
    KRYS_NODISCARD static Pair<LayoutRect, LayoutRect>
      ToMarginAndBorderBoxVisualRect(const BoxGeometry &logicalGeometry, const LayoutSize &containerSize,
                                     WritingMode writingMode) noexcept;
  };
}
