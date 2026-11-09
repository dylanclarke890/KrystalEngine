/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2010 Rob Buis <buis@kde.org>
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

#include "Krystal.Booey/SVG/Elements/SVGTSpanElement.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGTSpan.hpp"
#include "Krystal.Booey/SVG/Elements/SVGAElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGTextElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGTextPathElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGTSpanElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"

namespace krys::boo::svg
{
  SVGTSpanElement::SVGTSpanElement(dom::Document &document) noexcept
      : SVGTextPositioningElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::TSpan)
  {
  }

  RenderPtr<RenderElement> SVGTSpanElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<RenderSVGTSpan>(*this, krys::move(style));
  }

  bool SVGTSpanElement::ChildShouldCreateRenderer(const dom::Node &child) const noexcept
  {
    if (Is<dom::Text>(child))
    {
      return true;
    }
    auto *element = DynamicDowncast<SVGElement>(child);
    return element && (Is<SVGAElement>(*element) || Is<SVGTSpanElement>(*element));
  }

  bool SVGTSpanElement::RendererIsNeeded(const RenderStyle &style) noexcept
  {
    if (auto *parent = DynamicDowncast<SVGElement>(ParentNode()))
    {
      if (Is<SVGAElement>(*parent) || Is<SVGTextElement>(*parent) || Is<SVGTextPathElement>(*parent)
          || Is<SVGTSpanElement>(*parent))
      {
        return dom::StyledElement::RendererIsNeeded(style);
      }
    }
    return false;
  }
}
