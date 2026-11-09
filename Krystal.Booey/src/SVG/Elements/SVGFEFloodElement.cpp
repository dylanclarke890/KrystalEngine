/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2007, 2008 Rob Buis <buis@kde.org>
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Elements/SVGFEFloodElement.hpp"
#include "Krystal.Booey/Graphics/Filters/FEFlood.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGFEFloodElement::SVGFEFloodElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FEFlood)
  {
  }

  bool SVGFEFloodElement::SetFilterEffectAttribute(FilterEffect &effect,
                                                   const dom::QualifiedName &attributeName) noexcept
  {
    auto *renderer = Renderer();
    krys_debug_assert(renderer);
    const auto &style = renderer->Style();

    auto &feFlood = Downcast<FEFlood>(effect);
    if (attributeName == SVGNames::FloodColorAttr())
    {
      return feFlood.SetFloodColor(style.ColorResolvingCurrentColor(style.FloodColor()));
    }
    if (attributeName == SVGNames::FloodOpacityAttr())
    {
      return feFlood.SetFloodOpacity(style.FloodOpacity().Value.Value);
    }

    krys_debug_assert_msg(false, "Not reached");
    return false;
  }

  RefPtr<FilterEffect> SVGFEFloodElement::CreateFilterEffect(const FilterEffectVector &,
                                                             const GraphicsContext &) const noexcept
  {
    auto *renderer = Renderer();
    if (!renderer)
    {
      return nullptr;
    }

    const auto &style = renderer->Style();

    auto color = style.ColorResolvingCurrentColor(style.FloodColor());
    float opacity = style.FloodOpacity().Value.Value;

    return FEFlood::Create(color, opacity);
  }
}
