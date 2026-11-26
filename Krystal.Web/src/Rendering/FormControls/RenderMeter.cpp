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

#include "Krystal.Web/Rendering/FormControls/RenderMeter.hpp"
#include "Krystal.Web/Dom/Algorithms/TreeQueries.hpp"
#include "Krystal.Web/Html/Elements/HTMLMeterElement.hpp"
#include "Krystal.Web/Rendering/RenderTheme.hpp"

namespace krys::web
{
  RenderMeter::RenderMeter(HTMLElement &element, RenderStyle &&style) noexcept
      : RenderBlockFlow(Type::Meter, element, krys::move(style))
  {
    krys_debug_assert(IsRenderMeter());
  }

  RenderMeter::~RenderMeter() noexcept = default;

  HTMLMeterElement *RenderMeter::MeterElement() const noexcept
  {
    krys_debug_assert(GetElement());

    if (auto *meterElement = DynamicDowncast<HTMLMeterElement>(*GetElement()))
    {
      return meterElement;
    }

    krys_debug_assert(dom::TreeQueries::ShadowHost(*GetElement()));
    return Downcast<HTMLMeterElement>(dom::TreeQueries::ShadowHost(*GetElement()));
  }

  void RenderMeter::UpdateLogicalWidth() noexcept
  {
    RenderBox::UpdateLogicalWidth();

    auto frameSize = RenderTheme::Singleton().MeterSizeForBounds(*this, RectF(SnappedRectI(FrameRect())));
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
    auto frameSize = RenderTheme::Singleton().MeterSizeForBounds(*this, RectF(SnappedRectI(frame)));
    computedValues.Extent = LayoutUnit(IsHorizontalWritingMode() ? frameSize.Height() : frameSize.Width());
    return computedValues;
  }

  void RenderMeter::UpdateFromElement() noexcept
  {
    Repaint();
  }
}
