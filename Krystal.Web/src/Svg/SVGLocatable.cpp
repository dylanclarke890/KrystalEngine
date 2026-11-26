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

#include "Krystal.Web/Svg/SVGLocatable.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Traversal/TreeTraversal.hpp"
#include "Krystal.Web/Rendering/RenderElement.hpp"
#include "Krystal.Web/Svg/Elements/SVGElement.hpp"
#include "Krystal.Web/Svg/Elements/SVGForeignObjectElement.hpp"
#include "Krystal.Web/Svg/Elements/SVGImageElement.hpp"
#include "Krystal.Web/Svg/Elements/SVGSvgElement.hpp"
#include "Krystal.Web/Svg/Elements/SVGSymbolElement.hpp"

namespace krys::web
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

    constexpr EnumSet<layout::LayoutOptions> LayoutOptionsForQueries {
      layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
      layout::LayoutOptions::TreatContentVisibilityAutoAsVisible};
  }

  SVGElement *SVGLocatable::NearestViewportElement(const SVGElement *element) noexcept
  {
    krys_debug_assert(element);
    for (RefPtr<Element> current = ShareRefPtr(dom::TreeTraversal::ParentOrShadowHostElement(*element));
         current; current = ShareRefPtr(dom::TreeTraversal::ParentOrShadowHostElement(*current)))
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
    for (RefPtr<Element> current = ShareRefPtr(dom::TreeTraversal::ParentOrShadowHostElement(*element));
         current; current = ShareRefPtr(dom::TreeTraversal::ParentOrShadowHostElement(*current)))
    {
      auto *svgElement = DynamicDowncast<SVGElement>(*current);
      if (IsViewportElement(svgElement))
      {
        farthest = svgElement;
      }
    }
    return farthest;
  }

  RectF SVGLocatable::GetBBox(SVGElement *element, StyleUpdateStrategy styleUpdateStrategy) noexcept
  {
    krys_debug_assert(element);
    if (styleUpdateStrategy == AllowStyleUpdate)
    {
      (void)element->NodeDocument().UpdateLayoutIgnorePendingStylesheets(LayoutOptionsForQueries, element);
    }

    // FIXME: Eventually we should support getBBox for detached elements.
    if (!element->Renderer())
    {
      return RectF();
    }

    return element->Renderer()->ObjectBoundingBox();
  }

  AffineTransform SVGLocatable::ComputeCTM(SVGElement *element, svg::CTMScope mode,
                                           StyleUpdateStrategy styleUpdateStrategy) noexcept
  {
    krys_debug_assert(element);
    if (styleUpdateStrategy == AllowStyleUpdate)
    {
      (void)element->NodeDocument().UpdateLayoutIgnorePendingStylesheets(LayoutOptionsForQueries, element);
    }

    RefPtr<SVGElement> stopAtElement =
      ShareRefPtr(mode == svg::CTMScope::NearestViewportScope ? NearestViewportElement(element) : nullptr);

    AffineTransform ctm;

    for (Element *currentElement = element; currentElement;
         currentElement = dom::TreeTraversal::ParentOrShadowHostElement(*currentElement))
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
