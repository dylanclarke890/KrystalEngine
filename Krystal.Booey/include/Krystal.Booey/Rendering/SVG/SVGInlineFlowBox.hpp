/*
 * Copyright (C) 2006 Oliver Hunt <ojh16@student.canterbury.ac.nz>
 * Copyright (C) 2006 Apple Inc. All rights reserved.
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
 */

#pragma once

#include "Krystal.Booey/Rendering/Line/LegacyInlineFlowBox.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInline.hpp"

namespace krys::boo
{
  class RenderSVGInlineText;

  /// @brief An SVG inline renderer's legacy inline box.
  class SVGInlineFlowBox final : public LegacyInlineFlowBox
  {
  private:
    float _logicalHeight {0};

  public:
    explicit SVGInlineFlowBox(RenderSVGInline &renderer) noexcept : LegacyInlineFlowBox(renderer)
    {
    }

    KRYS_NODISCARD RenderSVGInline &Renderer() const noexcept
    {
      return static_cast<RenderSVGInline &>(LegacyInlineFlowBox::Renderer());
    }

    KRYS_NODISCARD FloatRect CalculateBoundaries() const noexcept;

    void SetLogicalHeight(float h) noexcept
    {
      _logicalHeight = h;
    }

  private:
    KRYS_NODISCARD bool IsSVGInlineFlowBox() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD float VirtualLogicalHeight() const noexcept override
    {
      return _logicalHeight;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::SVGInlineFlowBox)
  KRYS_NODISCARD static bool IsType(const krys::boo::LegacyInlineBox &box) noexcept
  {
    return box.IsSVGInlineFlowBox();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
