/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
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

#include "Krystal.Booey/SVG/Elements/SVGComponentTransferFunctionElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEComponentTransferElement.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGComponentTransferFunctionElement::SVGComponentTransferFunctionElement(
    dom::Document &document, SVGElementInterface interface) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), interface),
        _type(SVGAnimatedEnumeration::Create(this, ComponentTransferType::FECOMPONENTTRANSFER_TYPE_IDENTITY)),
        _tableValues(SVGAnimatedNumberList::Create(this)), _slope(SVGAnimatedNumber::Create(this, 1)),
        _intercept(SVGAnimatedNumber::Create(this)), _amplitude(SVGAnimatedNumber::Create(this, 1)),
        _exponent(SVGAnimatedNumber::Create(this, 1)), _offset(SVGAnimatedNumber::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<ComponentTransferType, &SVGComponentTransferFunctionElement::_type>(
        SVGNames::TypeAttr());
      PropertyRegistry::RegisterProperty<&SVGComponentTransferFunctionElement::_tableValues>(
        SVGNames::TableValuesAttr());
      PropertyRegistry::RegisterProperty<&SVGComponentTransferFunctionElement::_slope>(SVGNames::SlopeAttr());
      PropertyRegistry::RegisterProperty<&SVGComponentTransferFunctionElement::_intercept>(
        SVGNames::InterceptAttr());
      PropertyRegistry::RegisterProperty<&SVGComponentTransferFunctionElement::_amplitude>(
        SVGNames::AmplitudeAttr());
      PropertyRegistry::RegisterProperty<&SVGComponentTransferFunctionElement::_exponent>(
        SVGNames::ExponentAttr());
      PropertyRegistry::RegisterProperty<&SVGComponentTransferFunctionElement::_offset>(
        SVGNames::OffsetAttr());
    }
  }

  void SVGComponentTransferFunctionElement::AttributeChanged(const dom::QualifiedName &name,
                                                             Maybe<dom::DOMStringView> oldValue,
                                                             Maybe<dom::DOMStringView> value) noexcept
  {
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::TypeAttr())
    {
      auto propertyValue = SVGPropertyTraits<ComponentTransferType>::FromString(*this, string);
      if (propertyValue != ComponentTransferType::FECOMPONENTTRANSFER_TYPE_UNKNOWN)
      {
        _type->SetBaseValInternal<ComponentTransferType>(propertyValue);
      }
    }
    else if (name == SVGNames::TableValuesAttr())
    {
      _tableValues->BaseVal()->Parse(string);
    }
    else if (name == SVGNames::SlopeAttr())
    {
      _slope->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::InterceptAttr())
    {
      _intercept->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::AmplitudeAttr())
    {
      _amplitude->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::ExponentAttr())
    {
      _exponent->SetBaseValInternal(ToFloatOrZero(string));
    }
    else if (name == SVGNames::OffsetAttr())
    {
      _offset->SetBaseValInternal(ToFloatOrZero(string));
    }

    SVGElement::AttributeChanged(name, oldValue, value);
  }

  void
    SVGComponentTransferFunctionElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      if (auto *transferElement = DynamicDowncast<SVGFEComponentTransferElement>(ParentElement()))
      {
        InstanceInvalidationGuard guard(*this);
        transferElement->TransferFunctionAttributeChanged(*this, attributeName);
      }
      return;
    }

    SVGElement::SVGAttributeChanged(attributeName);
  }

  ComponentTransferFunction SVGComponentTransferFunctionElement::TransferFunction() const noexcept
  {
    return {GetType(), Slope(), Intercept(), Amplitude(), Exponent(), Offset(), TableValues()};
  }
}
