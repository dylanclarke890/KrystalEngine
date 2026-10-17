#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  RenderFlexibleBox::RenderFlexibleBox(Type type, dom::Element &element, RenderStyle &&style) noexcept
      : RenderBlock(type, element, krys::move(style), TypeFlag::IsFlexibleBox)
  {
    krys_debug_assert(IsRenderFlexibleBox());
    SetChildrenInline(false); // All of our children must be block-level.
  }

  RenderFlexibleBox::RenderFlexibleBox(Type type, dom::Document &document, RenderStyle &&style) noexcept
      : RenderBlock(type, document, krys::move(style), TypeFlag::IsFlexibleBox)
  {
    krys_debug_assert(IsRenderFlexibleBox());
    SetChildrenInline(false); // All of our children must be block-level.
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderFlexibleBox::~RenderFlexibleBox() noexcept = default;

  bool RenderFlexibleBox::MainAxisIsFlexItemInlineAxis(const RenderBox &flexItem) const noexcept
  {
    return IsHorizontalFlow() == flexItem.IsHorizontalWritingMode();
  }

  bool RenderFlexibleBox::IsColumnFlow() const noexcept
  {
    return Style().IsColumnFlexDirection();
  }

  bool RenderFlexibleBox::IsColumnOrRowReverse() const noexcept
  {
    return Style().GetFlexDirection() == FlexDirection::ColumnReverse
           || Style().GetFlexDirection() == FlexDirection::RowReverse;
  }

  bool RenderFlexibleBox::IsWrapReverse() const noexcept
  {
    return Style().GetFlexWrap() == FlexWrap::Reverse;
  }

  bool RenderFlexibleBox::IsHorizontalFlow() const noexcept
  {
    if (IsHorizontalWritingMode())
    {
      return !IsColumnFlow();
    }
    return IsColumnFlow();
  }

  bool RenderFlexibleBox::ShouldApplyMinSizeAutoForFlexItem(const RenderBox &flexItem) const noexcept
  {
    const auto &minSize = MinMainSizeLengthForFlexItem(flexItem);
    // min, max and fit-content are equivalent to the automatic size for block sizes
    // https://drafts.csswg.org/css-sizing-3/#valdef-width-min-content.
    bool flexItemBlockSizeIsEquivalentToAutomaticSize =
      !MainAxisIsFlexItemInlineAxis(flexItem)
      && (minSize.IsMinContent() || minSize.IsMaxContent() || minSize.IsFitContent());

    auto computedOverflowIsNotScrollable = [this, &flexItem]()
    {
      auto overflow = MainAxisOverflowForFlexItem(flexItem);
      return overflow == Overflow::Visible || overflow == Overflow::Clip;
    };

    return (minSize.IsAuto() || flexItemBlockSizeIsEquivalentToAutomaticSize)
           && computedOverflowIsNotScrollable();
  }

  bool RenderFlexibleBox::ShouldApplyMinBlockSizeAutoForFlexItem(const RenderBox &flexItem) const noexcept
  {
    return !MainAxisIsFlexItemInlineAxis(flexItem) && ShouldApplyMinSizeAutoForFlexItem(flexItem);
  }

  const style::MinimumSize &
    RenderFlexibleBox::MinMainSizeLengthForFlexItem(const RenderBox &flexItem) const noexcept
  {
    return IsHorizontalFlow() ? flexItem.Style().MinWidth() : flexItem.Style().MinHeight();
  }

  Overflow RenderFlexibleBox::MainAxisOverflowForFlexItem(const RenderBox &flexItem) const noexcept
  {
    if (IsHorizontalFlow())
    {
      return flexItem.Style().OverflowX();
    }
    return flexItem.Style().OverflowY();
  }

  bool RenderFlexibleBox::CanUseFlexItemForPercentageResolution(const RenderBox &flexItem) const noexcept
  {
    krys_debug_assert(flexItem.IsFlexItem());
    // FIXME: Whether the flex layout phase and the item's style let percentages resolve against the item's
    // flexed size, once there's flex layout. Until then a flex item's size is never definite for its
    // children.
    return false;
  }

  Maybe<LayoutUnit> RenderFlexibleBox::UsedFlexItemOverridingLogicalHeightForPercentageResolution(
    const RenderBox &flexItem) const noexcept
  {
    return CanUseFlexItemForPercentageResolution(flexItem) ? flexItem.OverridingBorderBoxLogicalHeight()
                                                           : null;
  }
}
