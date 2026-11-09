/*
 * Copyright (C) 2004, 2005, 2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2018-2019 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Elements/SVGGeometryElement.hpp"
#include "Krystal.Booey/SVG/SVGPointList.hpp"

namespace krys::boo::svg
{
  /// @brief The base of `<polygon>` and `<polyline>`: a shape through a list of points.
  class SVGPolyElement : public SVGGeometryElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGPolyElement);

  private:
    Ref<SVGAnimatedPointList> _points;

  public:
    KRYS_NODISCARD const SVGPointList &Points() const noexcept
    {
      return _points->CurrentValue();
    }
    /// @see https://svgwg.org/svg2-draft/shapes.html#__svg__SVGAnimatedPoints__points
    KRYS_NODISCARD SVGPointList &Points() noexcept
    {
      return *_points->BaseVal();
    }
    /// @see https://svgwg.org/svg2-draft/shapes.html#__svg__SVGAnimatedPoints__animatedPoints
    KRYS_NODISCARD SVGPointList &AnimatedPoints() noexcept
    {
      return *_points->AnimVal();
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGPolyElement, SVGGeometryElement>;

  protected:
    SVGPolyElement(dom::Document &document, SVGElementInterface interface) noexcept;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept override;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept override;

    KRYS_NODISCARD bool IsValid() const noexcept override
    {
      return SVGTests::IsValid();
    }
    KRYS_NODISCARD bool SupportsMarkers() const noexcept override
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGPolyElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGPolygonElement() || target.IsSVGPolylineElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
