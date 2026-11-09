/*
 * Copyright (C) 2004, 2005, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
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

#include "Krystal.Booey/SVG/Elements/SVGTextPositioningElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<tspan>` element: a span of SVG text with its own positions and style.
  /// @see https://svgwg.org/svg2-draft/text.html#InterfaceSVGTSpanElement
  class SVGTSpanElement final : public SVGTextPositioningElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGTSpanElement);

  public:
    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGTSpanElement, SVGTextPositioningElement>;

    explicit SVGTSpanElement(dom::Document &) noexcept;

  private:
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;
    KRYS_NODISCARD bool ChildShouldCreateRenderer(const dom::Node &) const noexcept final;
    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGTSpanElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGTSpanElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
