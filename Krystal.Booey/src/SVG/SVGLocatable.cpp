/*
 * Copyright (C) 2004, 2005 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
 * Copyright (C) 2009 Google, Inc. All rights reserved.
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#include "Krystal.Booey/SVG/SVGLocatable.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGForeignObjectElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGImageElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGSymbolElement.hpp"

namespace krys::boo::svg
{
  namespace
  {
    // FIXME: This doesn't match SVGElement::viewportElement() as it has an extra check for
    // foreign object.
    KRYS_NODISCARD bool IsViewportElement(const SVGElement *element) noexcept
    {
      if (!element)
      {
        return false;
      }

      return Is<SVGSvgElement>(*element) || Is<SVGSymbolElement>(*element)
             || Is<SVGForeignObjectElement>(*element) || Is<SVGImageElement>(*element);
    }

    constexpr EnumSet<dom::LayoutOptions> LayoutOptionsForQueries {
      dom::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
      dom::LayoutOptions::TreatContentVisibilityAutoAsVisible};
  }

  SVGElement *SVGLocatable::NearestViewportElement(const SVGElement *element) noexcept
  {
    krys_debug_assert(element);
    for (RefPtr<dom::Element> current = ShareRefPtr(dom::TreeQueries::ParentOrShadowHostElement(*element));
         current; current = ShareRefPtr(dom::TreeQueries::ParentOrShadowHostElement(*current)))
    {
      auto *svgElement = DynamicDowncast<SVGElement>(*current);
      if (IsViewportElement(svgElement))
      {
        return svgElement;
      }
    }

    return nullptr;
  }

  SVGElement *SVGLocatable::FarthestViewportElement(const SVGElement *element) noexcept
  {
    krys_debug_assert(element);
    SVGElement *farthest = nullptr;
    for (RefPtr<dom::Element> current = ShareRefPtr(dom::TreeQueries::ParentOrShadowHostElement(*element));
         current; current = ShareRefPtr(dom::TreeQueries::ParentOrShadowHostElement(*current)))
    {
      auto *svgElement = DynamicDowncast<SVGElement>(*current);
      if (IsViewportElement(svgElement))
      {
        farthest = svgElement;
      }
    }
    return farthest;
  }

  FloatRect SVGLocatable::GetBBox(SVGElement *element, StyleUpdateStrategy styleUpdateStrategy) noexcept
  {
    krys_debug_assert(element);
    if (styleUpdateStrategy == AllowStyleUpdate)
    {
      (void)element->NodeDocument().UpdateLayoutIgnorePendingStylesheets(LayoutOptionsForQueries, element);
    }

    // FIXME: Eventually we should support getBBox for detached elements.
    if (!element->Renderer())
    {
      return FloatRect();
    }

    return element->Renderer()->ObjectBoundingBox();
  }

  AffineTransform SVGLocatable::ComputeCTM(SVGElement *element, CTMScope mode,
                                           StyleUpdateStrategy styleUpdateStrategy) noexcept
  {
    krys_debug_assert(element);
    if (styleUpdateStrategy == AllowStyleUpdate)
    {
      (void)element->NodeDocument().UpdateLayoutIgnorePendingStylesheets(LayoutOptionsForQueries, element);
    }

    RefPtr<SVGElement> stopAtElement =
      ShareRefPtr(mode == CTMScope::NearestViewportScope ? NearestViewportElement(element) : nullptr);

    AffineTransform ctm;

    for (dom::Element *currentElement = element; currentElement;
         currentElement = dom::TreeQueries::ParentOrShadowHostElement(*currentElement))
    {
      RefPtr<SVGElement> svgElement = ShareRefPtr(DynamicDowncast<SVGElement>(*currentElement));
      if (!svgElement)
      {
        break;
      }

      ctm = svgElement->LocalCoordinateSpaceTransform(mode).Multiply(ctm);

      // For getCTM() computation, stop at the nearest viewport element
      if (currentElement == stopAtElement.get())
      {
        break;
      }
    }

    return ctm;
  }
}
