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

#include "Krystal.Web/Svg/Elements/SVGFEOffsetElement.hpp"
#include "Krystal.Web/Graphics/Filters/FEOffset.hpp"
#include "Krystal.Web/Svg/Graphics/Filters/SVGFilterRenderer.hpp"
#include "Krystal.Web/Svg/Internals/SVGParserUtilities.hpp"
#include "Krystal.Web/Svg/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Web/Svg/SVGNames.hpp"

namespace krys::web
{
  SVGFEOffsetElement::SVGFEOffsetElement(Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FEOffset),
        _in1(SVGAnimatedString::Create(this)), _dx(SVGAnimatedNumber::Create(this)),
        _dy(SVGAnimatedNumber::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFEOffsetElement::_in1>(SVGNames::InAttr());
      PropertyRegistry::RegisterProperty<&SVGFEOffsetElement::_dx>(SVGNames::DxAttr());
      PropertyRegistry::RegisterProperty<&SVGFEOffsetElement::_dy>(SVGNames::DyAttr());
    }
  }

  void SVGFEOffsetElement::AttributeChanged(const QualifiedName &name, Maybe<DOMStringView> oldValue,
                                            Maybe<DOMStringView> value) noexcept
  {
    auto string = value.value_or(DOMStringView {});

    if (name == SVGNames::DxAttr())
    {
      _dx->SetBaseValInternal(svg::ToFloatOrZero(string));
    }
    else if (name == SVGNames::DyAttr())
    {
      _dy->SetBaseValInternal(svg::ToFloatOrZero(string));
    }
    else if (name == SVGNames::InAttr())
    {
      _in1->SetBaseValInternal(DOMString(string));
    }

    SVGFilterPrimitiveStandardAttributes::AttributeChanged(name, oldValue, value);
  }

  void SVGFEOffsetElement::SVGAttributeChanged(const QualifiedName &attributeName) noexcept
  {
    if (attributeName == SVGNames::InAttr())
    {
      InstanceInvalidationGuard guard(*this);
      UpdateSVGRendererForElementChange();
      return;
    }
    if (attributeName == SVGNames::DxAttr() || attributeName == SVGNames::DyAttr())
    {
      InstanceInvalidationGuard guard(*this);
      PrimitiveAttributeChanged(attributeName);
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  bool SVGFEOffsetElement::SetFilterEffectAttribute(FilterEffect &effect,
                                                    const QualifiedName &attributeName) noexcept
  {
    auto &offset = Downcast<FEOffset>(effect);

    if (attributeName == SVGNames::DxAttr())
    {
      return offset.SetDx(Dx());
    }
    if (attributeName == SVGNames::DyAttr())
    {
      return offset.SetDy(Dy());
    }

    krys_debug_assert_msg(false, "Not reached");
    return false;
  }

  bool SVGFEOffsetElement::IsIdentity() const noexcept
  {
    return !Dx() && !Dy();
  }

  IntOutsets SVGFEOffsetElement::Outsets(const RectF &targetBoundingBox,
                                         SVGUnitTypes::SVGUnitType primitiveUnits) const noexcept
  {
    auto offset = SVGFilterRenderer::CalculateResolvedSize({Dx(), Dy()}, targetBoundingBox, primitiveUnits);
    return FEOffset::CalculateOutsets(offset);
  }

  RefPtr<FilterEffect> SVGFEOffsetElement::CreateFilterEffect(const FilterEffectVector &,
                                                              const GraphicsContext &) const noexcept
  {
    return FEOffset::Create(Dx(), Dy());
  }
}
