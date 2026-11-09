/*
 * Copyright (C) 2004, 2005, 2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2018-2019 Apple Inc. All rights reserved.
 * Copyright (C) 2023 Igalia S.L.
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

#include "Krystal.Booey/SVG/Elements/SVGGradientElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGStopElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include <algorithm>

namespace krys::boo::svg
{
  SVGGradientElement::SVGGradientElement(dom::Document &document,
                                         UniquePtr<SVGPropertyRegistry> &&propertyRegistry,
                                         SVGElementInterface interface) noexcept
      : SVGElement(document, krys::move(propertyRegistry), interface), SVGURIReference(this),
        _spreadMethod(SVGAnimatedEnumeration::Create(this, SVGSpreadMethodPad)),
        _gradientUnits(SVGAnimatedEnumeration::Create(this, SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)),
        _gradientTransform(SVGAnimatedTransformList::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<SVGSpreadMethodType, &SVGGradientElement::_spreadMethod>(
        SVGNames::SpreadMethodAttr());
      PropertyRegistry::RegisterProperty<SVGUnitTypes::SVGUnitType, &SVGGradientElement::_gradientUnits>(
        SVGNames::GradientUnitsAttr());
      PropertyRegistry::RegisterProperty<&SVGGradientElement::_gradientTransform>(
        SVGNames::GradientTransformAttr());
    }
  }

  void SVGGradientElement::AttributeChanged(const dom::QualifiedName &name,
                                            Maybe<dom::DOMStringView> oldValue,
                                            Maybe<dom::DOMStringView> value) noexcept
  {
    auto string = value.value_or(dom::DOMStringView {});
    if (name == SVGNames::GradientUnitsAttr())
    {
      auto propertyValue = SVGPropertyTraits<SVGUnitTypes::SVGUnitType>::FromString(*this, string);
      if (propertyValue > 0)
      {
        _gradientUnits->SetBaseValInternal<SVGUnitTypes::SVGUnitType>(propertyValue);
      }
    }
    else if (name == SVGNames::GradientTransformAttr())
    {
      _gradientTransform->BaseVal()->Parse(string);
    }
    else if (name == SVGNames::SpreadMethodAttr())
    {
      auto propertyValue = SVGPropertyTraits<SVGSpreadMethodType>::FromString(*this, string);
      if (propertyValue > 0)
      {
        _spreadMethod->SetBaseValInternal<SVGSpreadMethodType>(propertyValue);
      }
    }

    SVGURIReference::ParseAttribute(name, value);
    SVGElement::AttributeChanged(name, oldValue, value);
  }

  void SVGGradientElement::InvalidateGradientResource() noexcept
  {
    UpdateSVGRendererForElementChange();
  }

  void SVGGradientElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName) || SVGURIReference::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      InvalidateGradientResource();
      return;
    }

    SVGElement::SVGAttributeChanged(attributeName);
  }

  void SVGGradientElement::OnChildrenChanged() noexcept
  {
    SVGElement::OnChildrenChanged();
    if (!IsFinishedParsingChildren())
    {
      return;
    }

    InvalidateGradientResource();
  }

  GradientColorStops SVGGradientElement::BuildStops() noexcept
  {
    GradientColorStops stops;
    float previousOffset = 0.0f;
    for (auto *child = FirstChild(); child; child = child->NextSibling())
    {
      auto *svgChild = DynamicDowncast<SVGElement>(*child);
      auto *stop = svgChild ? DynamicDowncast<SVGStopElement>(*svgChild) : nullptr;
      if (!stop)
      {
        continue;
      }
      auto monotonicallyIncreasingOffset = std::clamp(stop->Offset(), previousOffset, 1.0f);
      previousOffset = monotonicallyIncreasingOffset;
      stops.AddColorStop({monotonicallyIncreasingOffset, stop->StopColorIncludingOpacity()});
    }
    return stops;
  }
}
