/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Oliver Hunt <oliver@nerget.com>
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

#include "Krystal.Web/Svg/Elements/SVGFESpecularLightingElement.hpp"
#include "Krystal.Web/Graphics/Filters/FESpecularLighting.hpp"
#include "Krystal.Web/Graphics/Filters/LightSource.hpp"
#include "Krystal.Web/Rendering/RenderElement.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Web/Svg/Elements/SVGFELightElement.hpp"
#include "Krystal.Web/Svg/Internals/SVGParserUtilities.hpp"
#include "Krystal.Web/Svg/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Web/Svg/SVGNames.hpp"

namespace krys::web
{
  SVGFESpecularLightingElement::SVGFESpecularLightingElement(Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FESpecularLighting),
        _in1(SVGAnimatedString::Create(this)), _specularConstant(SVGAnimatedNumber::Create(this, 1)),
        _specularExponent(SVGAnimatedNumber::Create(this, 1)),
        _surfaceScale(SVGAnimatedNumber::Create(this, 1)),
        _kernelUnitLengthX(SVGAnimatedNumber::Create(this)),
        _kernelUnitLengthY(SVGAnimatedNumber::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFESpecularLightingElement::_in1>(SVGNames::InAttr());
      PropertyRegistry::RegisterProperty<&SVGFESpecularLightingElement::_specularConstant>(
        SVGNames::SpecularConstantAttr());
      PropertyRegistry::RegisterProperty<&SVGFESpecularLightingElement::_specularExponent>(
        SVGNames::SpecularExponentAttr());
      PropertyRegistry::RegisterProperty<&SVGFESpecularLightingElement::_surfaceScale>(
        SVGNames::SurfaceScaleAttr());
      PropertyRegistry::RegisterProperty<&SVGFESpecularLightingElement::_kernelUnitLengthX,
                                         &SVGFESpecularLightingElement::_kernelUnitLengthY>(
        SVGNames::KernelUnitLengthAttr());
    }
  }

  void SVGFESpecularLightingElement::AttributeChanged(const QualifiedName &name,
                                                      Maybe<DOMStringView> oldValue,
                                                      Maybe<DOMStringView> value) noexcept
  {
    auto string = value.value_or(DOMStringView {});

    if (name == SVGNames::InAttr())
    {
      _in1->SetBaseValInternal(DOMString(string));
    }
    else if (name == SVGNames::SurfaceScaleAttr())
    {
      _surfaceScale->SetBaseValInternal(svg::ToFloatOrZero(string));
    }
    else if (name == SVGNames::SpecularConstantAttr())
    {
      _specularConstant->SetBaseValInternal(svg::ToFloatOrZero(string));
    }
    else if (name == SVGNames::SpecularExponentAttr())
    {
      _specularExponent->SetBaseValInternal(svg::ToFloatOrZero(string));
    }
    else if (name == SVGNames::KernelUnitLengthAttr())
    {
      if (auto result = svg::ParseNumberOptionalNumber(string))
      {
        _kernelUnitLengthX->SetBaseValInternal(result->first);
        _kernelUnitLengthY->SetBaseValInternal(result->second);
      }
    }

    SVGFilterPrimitiveStandardAttributes::AttributeChanged(name, oldValue, value);
  }

  bool SVGFESpecularLightingElement::SetFilterEffectAttribute(FilterEffect &filterEffect,
                                                              const QualifiedName &attributeName) noexcept
  {
    auto &effect = Downcast<FESpecularLighting>(filterEffect);
    auto lightElement = [this]
    {
      return SVGFELightElement::FindLightElement(this);
    };

    if (attributeName == SVGNames::LightingColorAttr())
    {
      const auto &style = Renderer()->Style();
      auto color = style.ColorResolvingCurrentColor(style.LightingColor());
      return effect.SetLightingColor(color);
    }
    if (attributeName == SVGNames::SurfaceScaleAttr())
    {
      return effect.SetSurfaceScale(SurfaceScale());
    }
    if (attributeName == SVGNames::SpecularConstantAttr())
    {
      return effect.SetSpecularConstant(SpecularConstant());
    }
    if (attributeName == SVGNames::SpecularExponentAttr())
    {
      return effect.SetSpecularExponent(SpecularExponent());
    }
    if (attributeName == SVGNames::AzimuthAttr())
    {
      return effect.GetLightSource().SetAzimuth(lightElement()->Azimuth());
    }
    if (attributeName == SVGNames::ElevationAttr())
    {
      return effect.GetLightSource().SetElevation(lightElement()->Elevation());
    }
    if (attributeName == SVGNames::XAttr())
    {
      return effect.GetLightSource().SetX(lightElement()->X());
    }
    if (attributeName == SVGNames::YAttr())
    {
      return effect.GetLightSource().SetY(lightElement()->Y());
    }
    if (attributeName == SVGNames::ZAttr())
    {
      return effect.GetLightSource().SetZ(lightElement()->Z());
    }
    if (attributeName == SVGNames::PointsAtXAttr())
    {
      return effect.GetLightSource().SetPointsAtX(lightElement()->PointsAtX());
    }
    if (attributeName == SVGNames::PointsAtYAttr())
    {
      return effect.GetLightSource().SetPointsAtY(lightElement()->PointsAtY());
    }
    if (attributeName == SVGNames::PointsAtZAttr())
    {
      return effect.GetLightSource().SetPointsAtZ(lightElement()->PointsAtZ());
    }
    if (attributeName == SVGNames::LimitingConeAngleAttr())
    {
      return effect.GetLightSource().SetLimitingConeAngle(lightElement()->LimitingConeAngle());
    }

    krys_debug_assert_msg(false, "Not reached");
    return false;
  }

  void SVGFESpecularLightingElement::SVGAttributeChanged(const QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      if (attributeName == SVGNames::InAttr())
      {
        UpdateSVGRendererForElementChange();
      }
      else
      {
        krys_debug_assert(attributeName == SVGNames::SpecularConstantAttr()
                          || attributeName == SVGNames::SpecularExponentAttr()
                          || attributeName == SVGNames::SurfaceScaleAttr()
                          || attributeName == SVGNames::KernelUnitLengthAttr());
        PrimitiveAttributeChanged(attributeName);
      }
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  void SVGFESpecularLightingElement::LightElementAttributeChanged(const SVGFELightElement *lightElement,
                                                                  const QualifiedName &attributeName) noexcept
  {
    if (SVGFELightElement::FindLightElement(this) != lightElement)
    {
      return;
    }

    // The light element has different attribute names so attrName can identify the requested attribute.
    PrimitiveAttributeChanged(attributeName);
  }

  RefPtr<FilterEffect>
    SVGFESpecularLightingElement::CreateFilterEffect(const FilterEffectVector &,
                                                     const GraphicsContext &) const noexcept
  {
    auto *lightElement = SVGFELightElement::FindLightElement(this);
    if (!lightElement)
    {
      return nullptr;
    }

    auto *renderer = Renderer();
    if (!renderer)
    {
      return nullptr;
    }

    auto lightSource = lightElement->GetLightSource();
    const auto &style = renderer->Style();

    auto color = style.ColorResolvingCurrentColor(style.LightingColor());

    return FESpecularLighting::Create(color, SurfaceScale(), SpecularConstant(), SpecularExponent(),
                                      KernelUnitLengthX(), KernelUnitLengthY(), krys::move(lightSource));
  }
}
