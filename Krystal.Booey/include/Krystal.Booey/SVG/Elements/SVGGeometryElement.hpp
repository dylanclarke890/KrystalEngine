/*
 * Copyright (C) 2004, 2005, 2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2018 Adobe Systems Incorporated. All rights reserved.
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

#include "Krystal.Booey/DOM/Geometry/DOMPointReadOnly.hpp"
#include "Krystal.Booey/SVG/Elements/SVGGraphicsElement.hpp"

namespace krys::boo::svg
{
  class SVGPoint;

  /// @brief A shape element: its geometry is a path that can be measured and hit tested.
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGGeometryElement
  class SVGGeometryElement : public SVGGraphicsElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGGeometryElement);

  private:
    Ref<SVGAnimatedNumber> _pathLength;

  public:
    /// @see https://svgwg.org/svg2-draft/types.html#__svg__SVGGeometryElement__getTotalLength
    KRYS_NODISCARD virtual float GetTotalLength() const noexcept;
    /// @see https://svgwg.org/svg2-draft/types.html#__svg__SVGGeometryElement__getPointAtLength
    KRYS_NODISCARD virtual dom::ExceptionOr<Ref<SVGPoint>> GetPointAtLength(float distance) const noexcept;

    /// @see https://svgwg.org/svg2-draft/types.html#__svg__SVGGeometryElement__isPointInFill
    KRYS_NODISCARD bool IsPointInFill(dom::DOMPointInit &&) noexcept;
    /// @see https://svgwg.org/svg2-draft/types.html#__svg__SVGGeometryElement__isPointInStroke
    KRYS_NODISCARD bool IsPointInStroke(dom::DOMPointInit &&) noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGGeometryElement, SVGGraphicsElement>;

    KRYS_NODISCARD float PathLength() const noexcept
    {
      return _pathLength->CurrentValue();
    }
    /// @see https://svgwg.org/svg2-draft/types.html#__svg__SVGGeometryElement__pathLength
    KRYS_NODISCARD SVGAnimatedNumber &PathLengthAnimated() noexcept
    {
      return *_pathLength;
    }

  protected:
    SVGGeometryElement(dom::Document &document, UniquePtr<SVGPropertyRegistry> &&propertyRegistry,
                       SVGElementInterface interface) noexcept;

    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept override;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGGeometryElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGGeometryElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
