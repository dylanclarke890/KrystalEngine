/*
 * Copyright (C) 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/Rendering/SVG/RenderSVGTextPath.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGPathData.hpp"
#include "Krystal.Booey/SVG/Elements/SVGGeometryElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGTextPathElement.hpp"

namespace krys::boo
{
  RenderSVGTextPath::RenderSVGTextPath(svg::SVGTextPathElement &element, RenderStyle &&style) noexcept
      : RenderSVGInline(Type::SVGTextPath, element, krys::move(style))
  {
    krys_debug_assert(IsRenderSVGTextPath());
  }

  RenderSVGTextPath::~RenderSVGTextPath() noexcept = default;

  svg::SVGTextPathElement &RenderSVGTextPath::TextPathElement() const noexcept
  {
    return Downcast<svg::SVGTextPathElement>(RenderSVGInline::GraphicsElement());
  }

  svg::SVGGeometryElement *RenderSVGTextPath::TargetElement() const noexcept
  {
    auto target = svg::SVGURIReference::TargetElementFromIRIString(
      TextPathElement().Href(), TextPathElement().TreeScopeForSVGReferences());
    auto *svgElement = DynamicDowncast<svg::SVGElement>(target.Element.get());
    return svgElement ? DynamicDowncast<svg::SVGGeometryElement>(*svgElement) : nullptr;
  }

  Path RenderSVGTextPath::LayoutPath() const noexcept
  {
    RefPtr element = ShareRefPtr(TargetElement());
    if (!element)
    {
      return {};
    }

    auto path = PathFromGraphicsElement(*element);

    // Spec:  The transform attribute on the referenced 'path' element represents a
    // supplemental transformation relative to the current user coordinate system for
    // the current 'text' element, including any adjustments to the current user coordinate
    // system due to a possible transform attribute on the current 'text' element.
    // http://www.w3.org/TR/SVG/text.html#TextPathElement
    path.Transform(element->AnimatedLocalTransform());
    return path;
  }

  const svg::SVGLengthValue &RenderSVGTextPath::StartOffset() const noexcept
  {
    return TextPathElement().StartOffset();
  }
}
