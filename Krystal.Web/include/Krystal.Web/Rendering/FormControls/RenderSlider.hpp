/*
 * Copyright (C) 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
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

#include "Krystal.Web/Rendering/RenderFlexibleBox.hpp"

namespace krys::web
{
  class HTMLInputElement;

  /// @brief The renderer of `<input type=range>`: a flex box holding its track and thumb.
  class RenderSlider final : public RenderFlexibleBox
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderSlider);

  public:
    constexpr static int32 DefaultTrackLength = 129;

    RenderSlider(HTMLInputElement &, RenderStyle &&) noexcept;

    ~RenderSlider() noexcept override;

    KRYS_NODISCARD HTMLInputElement &InputElement() const noexcept;

    KRYS_NODISCARD bool CanHaveGeneratedChildren() const noexcept override
    {
      return false;
    }

    KRYS_NODISCARD bool InDragMode() const noexcept;

    KRYS_NODISCARD double ValueRatio() const noexcept;

    /// @brief The content height a horizontal slider with a datalist's ticks needs to fit them; none
    /// without them.
    KRYS_NODISCARD Maybe<LayoutUnit> TickedTrackHeight() const noexcept;

    KRYS_NODISCARD LogicalExtentComputedValues
      ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept override;

  private:
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSlider";
    }

    void ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                       LayoutUnit &maxLogicalWidth) const noexcept override;

    void ComputePreferredLogicalWidths() noexcept override;

    KRYS_NODISCARD bool IsFlexibleBoxImpl() const noexcept override
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::RenderSlider)
  KRYS_NODISCARD static bool IsType(const krys::web::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderSlider();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
