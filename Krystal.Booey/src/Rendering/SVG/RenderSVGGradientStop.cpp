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

#include "Krystal.Booey/Rendering/SVG/RenderSVGGradientStop.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceContainer.hpp"
#include "Krystal.Booey/SVG/Elements/SVGGradientElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGStopElement.hpp"

namespace krys::boo
{
  RenderSVGGradientStop::RenderSVGGradientStop(svg::SVGStopElement &element, RenderStyle &&style) noexcept
      : RenderElement(Type::SVGGradientStop, element, krys::move(style), {}, {})
  {
    krys_debug_assert(IsRenderSVGGradientStop());
  }

  RenderSVGGradientStop::~RenderSVGGradientStop() noexcept = default;

  svg::SVGStopElement &RenderSVGGradientStop::GetElement() const noexcept
  {
    return Downcast<svg::SVGStopElement>(Downcast<svg::SVGElement>(NodeForNonAnonymous()));
  }

  void RenderSVGGradientStop::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderElement::StyleDidChange(diff, oldStyle);
    if (diff == style::DifferenceResult::Equal)
    {
      return;
    }

    // <stop> elements should only be allowed to make renderers under gradient elements
    // but I can imagine a few cases we might not be catching, so let's not crash if our parent isn't a
    // gradient.
    RefPtr<svg::SVGGradientElement> gradient = ShareRefPtr(GradientElement());
    if (!gradient)
    {
      return;
    }

    auto *renderer = gradient->Renderer();
    if (!renderer)
    {
      return;
    }

    Downcast<LegacyRenderSVGResourceContainer>(*renderer).RemoveAllClientsFromCacheAndMarkForInvalidation();
  }

  void RenderSVGGradientStop::Layout() noexcept
  {
    ClearNeedsLayout();
  }

  svg::SVGGradientElement *RenderSVGGradientStop::GradientElement() noexcept
  {
    return DynamicDowncast<svg::SVGGradientElement>(GetElement().ParentElement());
  }
}
