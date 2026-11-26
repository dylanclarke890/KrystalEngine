/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Oliver Hunt <oliver@nerget.com>
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

#include "Krystal.Booey/Svg/Elements/SVGFELightElement.hpp"
#include "Krystal.Booey/Graphics/Filters/LightSource.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEDiffuseLightingElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFESpecularLightingElement.hpp"
#include "Krystal.Booey/Svg/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/Svg/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/Svg/SVGFilterPrimitiveStandardAttributes.hpp"
#include "Krystal.Booey/Svg/SVGNames.hpp"

namespace krys::boo
{
  SVGFELightElement::SVGFELightElement(Document &document, SVGElementInterface interface) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), interface),
        _azimuth(SVGAnimatedNumber::Create(this)), _elevation(SVGAnimatedNumber::Create(this)),
        _x(SVGAnimatedNumber::Create(this)), _y(SVGAnimatedNumber::Create(this)),
        _z(SVGAnimatedNumber::Create(this)), _pointsAtX(SVGAnimatedNumber::Create(this)),
        _pointsAtY(SVGAnimatedNumber::Create(this)), _pointsAtZ(SVGAnimatedNumber::Create(this)),
        _specularExponent(SVGAnimatedNumber::Create(this, 1)),
        _limitingConeAngle(SVGAnimatedNumber::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_azimuth>(SVGNames::AzimuthAttr());
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_elevation>(SVGNames::ElevationAttr());
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_x>(SVGNames::XAttr());
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_y>(SVGNames::YAttr());
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_z>(SVGNames::ZAttr());
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_pointsAtX>(SVGNames::PointsAtXAttr());
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_pointsAtY>(SVGNames::PointsAtYAttr());
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_pointsAtZ>(SVGNames::PointsAtZAttr());
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_specularExponent>(
        SVGNames::SpecularExponentAttr());
      PropertyRegistry::RegisterProperty<&SVGFELightElement::_limitingConeAngle>(
        SVGNames::LimitingConeAngleAttr());
    }
  }

  SVGFELightElement *SVGFELightElement::FindLightElement(const SVGElement *svgElement) noexcept
  {
    for (auto *child = svgElement->FirstChild(); child; child = child->NextSibling())
    {
      if (auto *element = DynamicDowncast<SVGFELightElement>(*child))
      {
        return element;
      }
    }
    return nullptr;
  }

  void SVGFELightElement::AttributeChanged(const QualifiedName &name, Maybe<DOMStringView> oldValue,
                                           Maybe<DOMStringView> value) noexcept
  {
    auto string = value.value_or(DOMStringView {});

    if (name == SVGNames::AzimuthAttr())
    {
      _azimuth->SetBaseValInternal(svg::ToFloatOrZero(string));
    }
    else if (name == SVGNames::ElevationAttr())
    {
      _elevation->SetBaseValInternal(svg::ToFloatOrZero(string));
    }
    else if (name == SVGNames::XAttr())
    {
      _x->SetBaseValInternal(svg::ToFloatOrZero(string));
    }
    else if (name == SVGNames::YAttr())
    {
      _y->SetBaseValInternal(svg::ToFloatOrZero(string));
    }
    else if (name == SVGNames::ZAttr())
    {
      _z->SetBaseValInternal(svg::ToFloatOrZero(string));
    }
    else if (name == SVGNames::PointsAtXAttr())
    {
      _pointsAtX->SetBaseValInternal(svg::ToFloatOrZero(string));
    }
    else if (name == SVGNames::PointsAtYAttr())
    {
      _pointsAtY->SetBaseValInternal(svg::ToFloatOrZero(string));
    }
    else if (name == SVGNames::PointsAtZAttr())
    {
      _pointsAtZ->SetBaseValInternal(svg::ToFloatOrZero(string));
    }
    else if (name == SVGNames::SpecularExponentAttr())
    {
      _specularExponent->SetBaseValInternal(svg::ToFloatOrZero(string));
    }
    else if (name == SVGNames::LimitingConeAngleAttr())
    {
      _limitingConeAngle->SetBaseValInternal(svg::ToFloatOrZero(string));
    }

    SVGElement::AttributeChanged(name, oldValue, value);
  }

  void SVGFELightElement::SVGAttributeChanged(const QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      auto *parent = DynamicDowncast<SVGElement>(ParentElement());
      if (!parent)
      {
        return;
      }

      auto *renderer = parent->Renderer();
      if (!renderer || !renderer->IsLegacyRenderSVGResourceFilterPrimitive())
      {
        return;
      }

      if (auto *diffuseLighting = DynamicDowncast<SVGFEDiffuseLightingElement>(*parent))
      {
        InstanceInvalidationGuard guard(*this);
        diffuseLighting->LightElementAttributeChanged(this, attributeName);
      }
      else if (auto *specularLighting = DynamicDowncast<SVGFESpecularLightingElement>(*parent))
      {
        InstanceInvalidationGuard guard(*this);
        specularLighting->LightElementAttributeChanged(this, attributeName);
      }

      return;
    }

    SVGElement::SVGAttributeChanged(attributeName);
  }

  void SVGFELightElement::OnChildrenChanged() noexcept
  {
    SVGElement::OnChildrenChanged();

    if (_nodeFlags.contains(dom::NodeFlag::IsParsingChildren))
    {
      return;
    }

    SVGFilterPrimitiveStandardAttributes::InvalidateFilterPrimitiveParent(this);
  }
}
