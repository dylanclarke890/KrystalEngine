/*
 * Copyright (C) 2006, 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "Krystal.Booey/Rendering/FormControls/RenderSlider.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDataListElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
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

  Maybe<LayoutUnit> RenderSlider::TickedTrackHeight() const noexcept
  {
    auto &input = InputElement();
    if (Style().UsedAppearance() != StyleAppearance::SliderHorizontal || !input.List())
    {
      return {};
    }
    auto &theme = RenderTheme::Singleton();
    int32 offsetFromCenter = theme.SliderTickOffsetFromTrackCenter();
    LayoutUnit trackHeight(offsetFromCenter < 0 ? -2 * offsetFromCenter
                                                : 2 * (offsetFromCenter + theme.SliderTickSize().Height()));
    return LayoutUnit(trackHeight.ToFloat() * Style().UsedZoom());
  }

  RenderBox::LogicalExtentComputedValues
    RenderSlider::ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept
  {
    // The track is centred, so the ticks above it need room above and below. (The container's height is
    // stretched to the slider's, so it's the slider that grows.)
    if (auto trackHeight = TickedTrackHeight())
    {
      logicalHeight = std::max(logicalHeight, *trackHeight + BorderAndPaddingLogicalHeight());
    }
    return RenderFlexibleBox::ComputeLogicalHeight(logicalHeight, logicalTop);
  }

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
