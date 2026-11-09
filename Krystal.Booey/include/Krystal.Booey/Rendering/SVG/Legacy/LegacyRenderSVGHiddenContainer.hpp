/*
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
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

#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGContainer.hpp"

namespace krys::boo
{
  // This class is for containers which are never drawn, but do need to support style
  // <defs>, <linearGradient>, <radialGradient> are all good examples
  class LegacyRenderSVGHiddenContainer : public LegacyRenderSVGContainer
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGHiddenContainer);

  public:
    LegacyRenderSVGHiddenContainer(Type, svg::SVGElement &, RenderStyle &&,
                                   EnumSet<SVGModelObjectFlag> = {}) noexcept;

  protected:
    void Layout() noexcept override;

  private:
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGHiddenContainer";
    }

    void Paint(PaintInfo &, const LayoutPoint &) noexcept final;
    KRYS_NODISCARD LayoutRect ClippedOverflowRect(const RenderLayerModelObject *,
                                                  VisibleRectContext) const noexcept final;
    void AbsoluteQuads(List<FloatQuad> &, bool *wasFixed) const noexcept final;
    bool NodeAtFloatPoint(const HitTestRequest &, HitTestResult &, const FloatPoint &pointInParent,
                          HitTestAction) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGHiddenContainer)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGHiddenContainer();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
