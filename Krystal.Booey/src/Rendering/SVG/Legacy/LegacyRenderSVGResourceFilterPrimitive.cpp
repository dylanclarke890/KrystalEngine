/*
 * Copyright (C) 2010 University of Szeged
 * Copyright (C) 2010 Zoltan Herczeg
 * Copyright (C) 2011 Renata Hodovan (reni@webkit.org)
 * Copyright (C) 2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY UNIVERSITY OF SZEGED ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL UNIVERSITY OF SZEGED OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceFilterPrimitive.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceFilter.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEDiffuseLightingElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEDropShadowElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEFloodElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFESpecularLightingElement.hpp"
#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo
{
  LegacyRenderSVGResourceFilterPrimitive::LegacyRenderSVGResourceFilterPrimitive(
    svg::SVGFilterPrimitiveStandardAttributes &filterPrimitiveElement, RenderStyle &&style) noexcept
      : LegacyRenderSVGHiddenContainer(Type::LegacySVGResourceFilterPrimitive, filterPrimitiveElement,
                                       krys::move(style))
  {
  }

  svg::SVGFilterPrimitiveStandardAttributes &
    LegacyRenderSVGResourceFilterPrimitive::FilterPrimitiveElement() const noexcept
  {
    return static_cast<svg::SVGFilterPrimitiveStandardAttributes &>(
      LegacyRenderSVGHiddenContainer::GetElement());
  }

  void LegacyRenderSVGResourceFilterPrimitive::StyleDidChange(style::Difference diff,
                                                              const RenderStyle *oldStyle) noexcept
  {
    LegacyRenderSVGHiddenContainer::StyleDidChange(diff, oldStyle);

    if (diff == style::DifferenceResult::Equal || !oldStyle)
    {
      return;
    }

    const auto &newStyle = Style();
    auto &element = FilterPrimitiveElement();
    if (Is<svg::SVGFEFloodElement>(element) || Is<svg::SVGFEDropShadowElement>(element))
    {
      if (newStyle.FloodColor() != oldStyle->FloodColor())
      {
        element.PrimitiveAttributeChanged(svg::SVGNames::FloodColorAttr());
      }
      if (newStyle.FloodOpacity() != oldStyle->FloodOpacity())
      {
        element.PrimitiveAttributeChanged(svg::SVGNames::FloodOpacityAttr());
      }
    }
    else if (Is<svg::SVGFEDiffuseLightingElement>(element) || Is<svg::SVGFESpecularLightingElement>(element))
    {
      if (newStyle.LightingColor() != oldStyle->LightingColor())
      {
        element.PrimitiveAttributeChanged(svg::SVGNames::LightingColorAttr());
      }
    }
  }

  void LegacyRenderSVGResourceFilterPrimitive::MarkFilterEffectForRepaint(FilterEffect *effect) noexcept
  {
    auto *parent = DynamicDowncast<LegacyRenderSVGResourceFilter>(Parent());
    if (!parent)
    {
      return;
    }

    if (effect)
    {
      parent->MarkFilterForRepaint(*effect);
    }

    parent->MarkAllClientLayersForInvalidation();
  }

  void LegacyRenderSVGResourceFilterPrimitive::MarkFilterEffectForRebuild() noexcept
  {
    auto *parent = DynamicDowncast<LegacyRenderSVGResourceFilter>(Parent());
    if (!parent)
    {
      return;
    }

    parent->MarkFilterForRebuild();
    parent->MarkAllClientLayersForInvalidation();
  }
}
