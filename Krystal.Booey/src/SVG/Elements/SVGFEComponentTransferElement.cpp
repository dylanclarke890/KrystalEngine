/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2018-2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Elements/SVGFEComponentTransferElement.hpp"
#include "Krystal.Booey/Graphics/Filters/FEComponentTransfer.hpp"
#include "Krystal.Booey/SVG/Elements/SVGComponentTransferFunctionElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  namespace
  {
    KRYS_NODISCARD bool
      IsRelevantTransferFunctionElement(const SVGComponentTransferFunctionElement &child) noexcept
    {
      auto name = child.Interface();

      for (auto laterSibling = child.NextElementSibling(); laterSibling;
           laterSibling = laterSibling->NextElementSibling())
      {
        auto *svgSibling = DynamicDowncast<SVGElement>(*laterSibling);
        if (svgSibling && svgSibling->Interface() == name)
        {
          return false;
        }
      }

      return true;
    }
  }

  SVGFEComponentTransferElement::SVGFEComponentTransferElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FEComponentTransfer),
        _in1(SVGAnimatedString::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFEComponentTransferElement::_in1>(SVGNames::InAttr());
    }
  }

  void SVGFEComponentTransferElement::AttributeChanged(const dom::QualifiedName &name,
                                                       Maybe<dom::DOMStringView> oldValue,
                                                       Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == SVGNames::InAttr())
    {
      _in1->SetBaseValInternal(dom::DOMString(value.value_or(dom::DOMStringView {})));
    }

    SVGFilterPrimitiveStandardAttributes::AttributeChanged(name, oldValue, value);
  }

  void SVGFEComponentTransferElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (attributeName == SVGNames::InAttr())
    {
      InstanceInvalidationGuard guard(*this);
      UpdateSVGRendererForElementChange();
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  RefPtr<FilterEffect>
    SVGFEComponentTransferElement::CreateFilterEffect(const FilterEffectVector &,
                                                      const GraphicsContext &) const noexcept
  {
    ComponentTransferFunctions functions;

    for (auto *child = FirstChild(); child; child = child->NextSibling())
    {
      if (auto *function = DynamicDowncast<SVGComponentTransferFunctionElement>(*child))
      {
        functions[static_cast<size_t>(function->Channel())] = function->TransferFunction();
      }
    }

    return FEComponentTransfer::Create(krys::move(functions));
  }

  bool SVGFEComponentTransferElement::SetFilterEffectAttributeFromChild(
    FilterEffect &filterEffect, const dom::Element &childElement,
    const dom::QualifiedName &attributeName) noexcept
  {
    auto *child = DynamicDowncast<SVGComponentTransferFunctionElement>(childElement);
    krys_debug_assert(child);
    if (!child)
    {
      return false;
    }

    krys_debug_assert(IsRelevantTransferFunctionElement(*child));

    auto &effect = Downcast<FEComponentTransfer>(filterEffect);

    if (attributeName == SVGNames::TypeAttr())
    {
      return effect.SetType(child->Channel(), child->GetType());
    }
    if (attributeName == SVGNames::SlopeAttr())
    {
      return effect.SetSlope(child->Channel(), child->Slope());
    }
    if (attributeName == SVGNames::InterceptAttr())
    {
      return effect.SetIntercept(child->Channel(), child->Intercept());
    }
    if (attributeName == SVGNames::AmplitudeAttr())
    {
      return effect.SetAmplitude(child->Channel(), child->Amplitude());
    }
    if (attributeName == SVGNames::ExponentAttr())
    {
      return effect.SetExponent(child->Channel(), child->Exponent());
    }
    if (attributeName == SVGNames::OffsetAttr())
    {
      return effect.SetOffset(child->Channel(), child->Offset());
    }
    if (attributeName == SVGNames::TableValuesAttr())
    {
      return effect.SetTableValues(child->Channel(), child->TableValues());
    }

    return false;
  }

  void SVGFEComponentTransferElement::TransferFunctionAttributeChanged(
    SVGComponentTransferFunctionElement &child, const dom::QualifiedName &attributeName) noexcept
  {
    krys_debug_assert(child.ParentNode() == this);

    if (!IsRelevantTransferFunctionElement(child))
    {
      return;
    }

    PrimitiveAttributeOnChildChanged(child, attributeName);
  }
}
