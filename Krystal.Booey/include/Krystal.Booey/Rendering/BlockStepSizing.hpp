#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class RenderBox;
  class RenderStyle;
  class WritingMode;
}

/// @brief Block step sizing (`block-step-size`): rounding a block-level box's outer size up to a multiple of
/// the step.
namespace krys::boo::BlockStepSizing
{
  /// @brief Whether the child's block step properties are the ones block layout supports.
  KRYS_NODISCARD bool ChildHasSupportedStyle(const RenderStyle &childStyle) noexcept;

  /// @brief The space that rounds @p boxOuterSize up to a multiple of @p stepSize.
  KRYS_NODISCARD LayoutUnit ComputeExtraSpace(LayoutUnit stepSize, LayoutUnit boxOuterSize) noexcept;

  void DistributeExtraSpaceToChildMargins(RenderBox &child, LayoutUnit extraSpace,
                                          WritingMode containingBlockWritingMode) noexcept;
  void DistributeExtraSpaceToChildPadding(RenderBox &child, LayoutUnit extraSpace,
                                          WritingMode containingBlockWritingMode) noexcept;
  void DistributeExtraSpaceToChildContentArea(RenderBox &child, LayoutUnit extraSpace,
                                              WritingMode containingBlockWritingMode) noexcept;
}
