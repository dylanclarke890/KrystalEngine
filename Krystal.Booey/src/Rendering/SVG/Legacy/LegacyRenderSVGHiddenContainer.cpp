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

#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGHiddenContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"

namespace krys::boo
{
  LegacyRenderSVGHiddenContainer::LegacyRenderSVGHiddenContainer(
    Type type, svg::SVGElement &element, RenderStyle &&style, EnumSet<SVGModelObjectFlag> svgFlags) noexcept
      : LegacyRenderSVGContainer(type, element, krys::move(style),
                                 svgFlags | SVGModelObjectFlag::IsHiddenContainer)
  {
  }

  void LegacyRenderSVGHiddenContainer::Layout() noexcept
  {
    krys_debug_assert(NeedsLayout());
    SVGRenderSupport::LayoutChildren(*this, SelfNeedsLayout());
    ClearNeedsLayout();
  }

  void LegacyRenderSVGHiddenContainer::Paint(PaintInfo &, const LayoutPoint &) noexcept
  {
    // This subtree does not paint.
  }

  LayoutRect LegacyRenderSVGHiddenContainer::ClippedOverflowRect(const RenderLayerModelObject *,
                                                                 VisibleRectContext) const noexcept
  {
    return {};
  }

  void LegacyRenderSVGHiddenContainer::AbsoluteQuads(List<FloatQuad> &, bool *) const noexcept
  {
    // This subtree does not take up space or paint
  }

  bool LegacyRenderSVGHiddenContainer::NodeAtFloatPoint(const HitTestRequest &, HitTestResult &,
                                                        const FloatPoint &, HitTestAction) noexcept
  {
    return false;
  }
}
