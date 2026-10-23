#include "Krystal.Booey/Rendering/RenderSlider.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  RenderSlider::RenderSlider(html::HTMLInputElement &element, RenderStyle &&style) noexcept
      : RenderFlexibleBox(Type::Slider, element, krys::move(style))
  {
    // We assume RenderSlider works only with <input type=range>.
    krys_debug_assert(element.IsRangeControl());
    krys_debug_assert(IsRenderSlider());
  }

  RenderSlider::~RenderSlider() noexcept = default;

  html::HTMLInputElement &RenderSlider::InputElement() const noexcept
  {
    return Downcast<html::HTMLInputElement>(NodeForNonAnonymous());
  }

  void RenderSlider::ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                                   LayoutUnit &maxLogicalWidth) const noexcept
  {
    if (ShouldApplySizeOrInlineSizeContainment())
    {
      if (auto width = ExplicitIntrinsicInnerLogicalWidth())
      {
        minLogicalWidth = *width;
        maxLogicalWidth = *width;
      }
      return;
    }

    maxLogicalWidth = LayoutUnit(DefaultTrackLength * Style().UsedZoom());
    auto &logicalWidth = Style().LogicalWidth();
    if (logicalWidth.IsCalculated())
    {
      minLogicalWidth =
        std::max(0_lu, style::Evaluate<LayoutUnit>(logicalWidth, 0_lu, Style().UsedZoomForLength()));
    }
    else if (!logicalWidth.IsPercent())
    {
      minLogicalWidth = maxLogicalWidth;
    }
  }

  void RenderSlider::ComputePreferredLogicalWidths() noexcept
  {
    _minPreferredLogicalWidth = 0_lu;
    _maxPreferredLogicalWidth = 0_lu;

    if (auto fixedLogicalWidth = Style().LogicalWidth().TryFixed())
    {
      _minPreferredLogicalWidth = _maxPreferredLogicalWidth = AdjustContentBoxLogicalWidthForBoxSizing(
        LayoutUnit {fixedLogicalWidth->ResolveZoom(Style().UsedZoomForLength())});
    }
    else
    {
      ComputeIntrinsicLogicalWidths(_minPreferredLogicalWidth, _maxPreferredLogicalWidth);
    }

    RenderBox::ComputePreferredLogicalWidths(Style().LogicalMinWidth(), Style().LogicalMaxWidth(),
                                             GetWritingMode().IsHorizontal()
                                               ? HorizontalBorderAndPaddingExtent()
                                               : VerticalBorderAndPaddingExtent());

    ClearNeedsPreferredWidthsUpdate();
  }

  bool RenderSlider::InDragMode() const noexcept
  {
    return InputElement().SliderThumbElement()->Active();
  }

  double RenderSlider::ValueRatio() const noexcept
  {
    auto &element = InputElement();
    auto min = element.Minimum();
    auto max = element.Maximum();
    auto value = element.ValueAsNumber();
    if (max <= min)
    {
      return 0;
    }
    return (value - min) / (max - min);
  }
}
