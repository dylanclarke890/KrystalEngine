/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
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

#include "Krystal.Booey/Svg/Elements/SVGFETurbulenceElement.hpp"
#include "Krystal.Booey/Svg/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/Svg/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/Svg/SVGNames.hpp"
#include "Krystal.Core/Strings/StringToIntegerConversion.hpp"

namespace krys::boo
{
  SVGFETurbulenceElement::SVGFETurbulenceElement(Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FETurbulence),
        _baseFrequencyX(SVGAnimatedNumber::Create(this)), _baseFrequencyY(SVGAnimatedNumber::Create(this)),
        _numOctaves(SVGAnimatedInteger::Create(this, InitialOctavesValue)),
        _seed(SVGAnimatedNumber::Create(this)),
        _stitchTiles(SVGAnimatedEnumeration::Create(this, SVG_STITCHTYPE_NOSTITCH)),
        _type(SVGAnimatedEnumeration::Create(this, TurbulenceType::Turbulence))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFETurbulenceElement::_baseFrequencyX,
                                         &SVGFETurbulenceElement::_baseFrequencyY>(
        SVGNames::BaseFrequencyAttr());
      PropertyRegistry::RegisterProperty<&SVGFETurbulenceElement::_numOctaves>(SVGNames::NumOctavesAttr());
      PropertyRegistry::RegisterProperty<&SVGFETurbulenceElement::_seed>(SVGNames::SeedAttr());
      PropertyRegistry::RegisterProperty<SVGStitchOptions, &SVGFETurbulenceElement::_stitchTiles>(
        SVGNames::StitchTilesAttr());
      PropertyRegistry::RegisterProperty<TurbulenceType, &SVGFETurbulenceElement::_type>(
        SVGNames::TypeAttr());
    }
  }

  void SVGFETurbulenceElement::AttributeChanged(const QualifiedName &name, Maybe<DOMStringView> oldValue,
                                                Maybe<DOMStringView> value) noexcept
  {
    auto string = value.value_or(DOMStringView {});

    if (name == SVGNames::TypeAttr())
    {
      auto propertyValue = SVGPropertyTraits<TurbulenceType>::FromString(*this, string);
      if (propertyValue != TurbulenceType::Unknown)
      {
        _type->SetBaseValInternal<TurbulenceType>(propertyValue);
      }
    }
    else if (name == SVGNames::StitchTilesAttr())
    {
      auto propertyValue = SVGPropertyTraits<SVGStitchOptions>::FromString(*this, string);
      if (propertyValue > 0)
      {
        _stitchTiles->SetBaseValInternal<SVGStitchOptions>(propertyValue);
      }
    }
    else if (name == SVGNames::BaseFrequencyAttr())
    {
      if (auto result = svg::ParseNumberOptionalNumber(string))
      {
        _baseFrequencyX->SetBaseValInternal(result->first);
        _baseFrequencyY->SetBaseValInternal(result->second);
      }
    }
    else if (name == SVGNames::SeedAttr())
    {
      _seed->SetBaseValInternal(svg::ToFloatOrZero(string));
    }
    else if (name == SVGNames::NumOctavesAttr())
    {
      _numOctaves->SetBaseValInternal(text::ParseInteger<int32>(string).value_or(InitialOctavesValue));
    }

    SVGFilterPrimitiveStandardAttributes::AttributeChanged(name, oldValue, value);
  }

  bool SVGFETurbulenceElement::SetFilterEffectAttribute(FilterEffect &filterEffect,
                                                        const QualifiedName &attributeName) noexcept
  {
    auto &effect = Downcast<FETurbulence>(filterEffect);
    if (attributeName == SVGNames::TypeAttr())
    {
      return effect.SetType(GetType());
    }
    if (attributeName == SVGNames::StitchTilesAttr())
    {
      return effect.SetStitchTiles(StitchTiles() == SVG_STITCHTYPE_STITCH);
    }
    if (attributeName == SVGNames::BaseFrequencyAttr())
    {
      bool baseFrequencyXChanged = effect.SetBaseFrequencyX(BaseFrequencyX());
      bool baseFrequencyYChanged = effect.SetBaseFrequencyY(BaseFrequencyY());
      return baseFrequencyXChanged || baseFrequencyYChanged;
    }
    if (attributeName == SVGNames::SeedAttr())
    {
      return effect.SetSeed(Seed());
    }
    if (attributeName == SVGNames::NumOctavesAttr())
    {
      return effect.SetNumOctaves(NumOctaves());
    }

    krys_debug_assert_msg(false, "Not reached");
    return false;
  }

  void SVGFETurbulenceElement::SVGAttributeChanged(const QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      PrimitiveAttributeChanged(attributeName);
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  RefPtr<FilterEffect> SVGFETurbulenceElement::CreateFilterEffect(const FilterEffectVector &,
                                                                  const GraphicsContext &) const noexcept
  {
    if (BaseFrequencyX() < 0 || BaseFrequencyY() < 0)
    {
      return nullptr;
    }

    return FETurbulence::Create(GetType(), BaseFrequencyX(), BaseFrequencyY(), NumOctaves(), Seed(),
                                StitchTiles() == SVG_STITCHTYPE_STITCH);
  }
}
