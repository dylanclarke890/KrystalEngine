#include "Krystal.Booey/Rendering/RenderMeter.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/HTML/HTMLMeterElement.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"

namespace krys::boo
{
  RenderMeter::RenderMeter(html::HTMLElement &element, RenderStyle &&style) noexcept
      : RenderBlockFlow(Type::Meter, element, krys::move(style))
  {
    krys_debug_assert(IsRenderMeter());
  }

  RenderMeter::~RenderMeter() noexcept = default;

  html::HTMLMeterElement *RenderMeter::MeterElement() const noexcept
  {
    krys_debug_assert(GetElement());

    if (auto *meterElement = DynamicDowncast<html::HTMLMeterElement>(*GetElement()))
    {
      return meterElement;
    }

    krys_debug_assert(dom::TreeQueries::ShadowHost(*GetElement()));
    return Downcast<html::HTMLMeterElement>(dom::TreeQueries::ShadowHost(*GetElement()));
  }

  void RenderMeter::UpdateLogicalWidth() noexcept
  {
    RenderBox::UpdateLogicalWidth();

    auto frameSize =
      RenderTheme::Singleton().MeterSizeForBounds(*this, FloatRect(SnappedIntRect(FrameRect())));
    SetLogicalWidth(LayoutUnit(IsHorizontalWritingMode() ? frameSize.Width() : frameSize.Height()));
  }

  RenderBox::LogicalExtentComputedValues
    RenderMeter::ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept
  {
    auto computedValues = RenderBox::ComputeLogicalHeight(logicalHeight, logicalTop);
    LayoutRect frame = FrameRect();
    if (IsHorizontalWritingMode())
    {
      frame.SetHeight(computedValues.Extent);
    }
    else
    {
      frame.SetWidth(computedValues.Extent);
    }
    auto frameSize = RenderTheme::Singleton().MeterSizeForBounds(*this, FloatRect(SnappedIntRect(frame)));
    computedValues.Extent = LayoutUnit(IsHorizontalWritingMode() ? frameSize.Height() : frameSize.Width());
    return computedValues;
  }

  void RenderMeter::UpdateFromElement() noexcept
  {
    Repaint();
  }
}
