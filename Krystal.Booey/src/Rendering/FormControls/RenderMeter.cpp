/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include "Krystal.Booey/Rendering/FormControls/RenderMeter.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLMeterElement.hpp"
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
