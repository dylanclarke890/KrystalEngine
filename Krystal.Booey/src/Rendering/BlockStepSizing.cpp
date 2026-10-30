#include "Krystal.Booey/Rendering/BlockStepSizing.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::BlockStepSizing
{
  bool ChildHasSupportedStyle(const RenderStyle &childStyle) noexcept
  {
    return childStyle.GetBlockStepInsert() == BlockStepInsert::MarginBox
           && childStyle.GetBlockStepAlign() == BlockStepAlign::Auto
           && childStyle.GetBlockStepRound() == BlockStepRound::Up;
  }

  LayoutUnit ComputeExtraSpace(LayoutUnit stepSize, LayoutUnit boxOuterSize) noexcept
  {
    if (!stepSize)
    {
      return {};
    }

    if (!boxOuterSize)
    {
      return stepSize;
    }

    if (auto remainder = IntMod(boxOuterSize, stepSize))
    {
      return stepSize - remainder;
    }
    return {};
  }

  void DistributeExtraSpaceToChildMargins(RenderBox &child, LayoutUnit extraSpace,
                                          WritingMode containingBlockWritingMode) noexcept
  {
    auto halfExtraSpace = extraSpace / 2;
    child.SetMarginBefore(child.MarginBefore(containingBlockWritingMode) + halfExtraSpace);
    child.SetMarginAfter(child.MarginAfter(containingBlockWritingMode) + halfExtraSpace);
  }

  void DistributeExtraSpaceToChildPadding(RenderBox &, LayoutUnit, WritingMode) noexcept
  {
    // FIXME: Not implemented yet.
    krys_debug_assert(false);
  }

  void DistributeExtraSpaceToChildContentArea(RenderBox &, LayoutUnit, WritingMode) noexcept
  {
    // FIXME: Not implemented yet.
    krys_debug_assert(false);
  }
}
