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

#include "Krystal.Booey/SVG/Elements/SVGPolyElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGPath.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGPolyElement::SVGPolyElement(dom::Document &document, SVGElementInterface interface) noexcept
      : SVGGeometryElement(document, CreateUnique<PropertyRegistry>(*this), interface),
        _points(SVGAnimatedPointList::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGPolyElement::_points>(SVGNames::PointsAttr());
    }
  }

  void SVGPolyElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                        Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == SVGNames::PointsAttr())
    {
      _points->BaseVal()->Parse(value.value_or(dom::DOMStringView {}));
    }

    SVGGeometryElement::AttributeChanged(name, oldValue, value);
  }

  void SVGPolyElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      krys_debug_assert(attributeName == SVGNames::PointsAttr());
      InstanceInvalidationGuard guard(*this);

      if (auto *path = DynamicDowncast<LegacyRenderSVGPath>(Renderer()))
      {
        path->SetNeedsShapeUpdate();
      }

      UpdateSVGRendererForElementChange();
      return;
    }

    SVGGeometryElement::SVGAttributeChanged(attributeName);
  }
}
