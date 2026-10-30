#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  class ElementBox;
  class InlineLayoutState;
  class LayoutState;
}

namespace krys::boo::layout_integration
{
  /// @brief Lays out @p box with its own formatting context (through its renderer), optionally under width
  /// and height constraints, and copies the result to its box geometry.
  void LayoutWithFormattingContextForBox(const layout::ElementBox &box, Maybe<LayoutUnit> widthConstraint,
                                         Maybe<LayoutUnit> heightConstraint,
                                         layout::LayoutState &layoutState) noexcept;

  enum class LogicalWidthType : uint8
  {
    PreferredMaximum,
    PreferredMinimum,
    MaxContent,
    MinContent
  };
  KRYS_NODISCARD LayoutUnit FormattingContextRootLogicalWidthForType(
    const layout::ElementBox &box, LogicalWidthType logicalWidthType) noexcept;

  enum class LogicalHeightType : uint8
  {
    MinContent
  };
  KRYS_NODISCARD LayoutUnit FormattingContextRootLogicalHeightForType(
    const layout::ElementBox &box, LogicalHeightType logicalHeightType) noexcept;

  /// @brief Lays out a block in inline content (through the block container's renderer) at the line's logical
  /// top left, and copies the result to its box geometry and the inline layout state.
  void LayoutWithFormattingContextForBlockInInline(const layout::ElementBox &block,
                                                   LayoutPoint blockLineLogicalTopLeft,
                                                   layout::InlineLayoutState &inlineLayoutState,
                                                   layout::LayoutState &layoutState) noexcept;
}
