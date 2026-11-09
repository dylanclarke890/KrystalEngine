/*
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#pragma once

#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"

namespace krys::boo::html
{
  class HTMLElement;
  class HTMLMeterElement;
}

namespace krys::boo
{
  /// @brief The renderer of a `<meter>` the theme draws itself.
  class RenderMeter final : public RenderBlockFlow
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMeter);

  public:
    RenderMeter(html::HTMLElement &, RenderStyle &&) noexcept;
    ~RenderMeter() noexcept override;

    KRYS_NODISCARD html::HTMLMeterElement *MeterElement() const noexcept;

    void UpdateFromElement() noexcept override;

  private:
    void UpdateLogicalWidth() noexcept override;
    KRYS_NODISCARD LogicalExtentComputedValues
      ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderMeter";
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMeter)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMeter();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
