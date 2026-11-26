/*
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
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

#include "Krystal.Booey/Svg/Elements/SVGFEDropShadowElement.hpp"
#include "Krystal.Booey/Graphics/Filters/FEDropShadow.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Svg/Graphics/Filters/SVGFilterRenderer.hpp"
#include "Krystal.Booey/Svg/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/Svg/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/Svg/SVGNames.hpp"

namespace krys::boo
{
  SVGFEDropShadowElement::SVGFEDropShadowElement(Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FEDropShadow),
        _in1(SVGAnimatedString::Create(this)), _dx(SVGAnimatedNumber::Create(this, 2)),
        _dy(SVGAnimatedNumber::Create(this, 2)), _stdDeviationX(SVGAnimatedNumber::Create(this, 2)),
        _stdDeviationY(SVGAnimatedNumber::Create(this, 2))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGFEDropShadowElement::_in1>(SVGNames::InAttr());
      PropertyRegistry::RegisterProperty<&SVGFEDropShadowElement::_dx>(SVGNames::DxAttr());
      PropertyRegistry::RegisterProperty<&SVGFEDropShadowElement::_dy>(SVGNames::DyAttr());
      PropertyRegistry::RegisterProperty<&SVGFEDropShadowElement::_stdDeviationX,
                                         &SVGFEDropShadowElement::_stdDeviationY>(
        SVGNames::StdDeviationAttr());
    }
  }

  void SVGFEDropShadowElement::SetStdDeviation(float x, float y) noexcept
  {
    _stdDeviationX->SetBaseValInternal(x);
    _stdDeviationY->SetBaseValInternal(y);
    UpdateSVGRendererForElementChange();
  }

  void SVGFEDropShadowElement::AttributeChanged(const QualifiedName &name, Maybe<DOMStringView> oldValue,
                                                Maybe<DOMStringView> value) noexcept
  {
    auto string = value.value_or(DOMStringView {});

    if (name == SVGNames::StdDeviationAttr())
    {
      if (auto result = svg::ParseNumberOptionalNumber(string))
      {
        _stdDeviationX->SetBaseValInternal(result->first);
        _stdDeviationY->SetBaseValInternal(result->second);
      }
    }
    else if (name == SVGNames::InAttr())
    {
      _in1->SetBaseValInternal(DOMString(string));
    }
    else if (name == SVGNames::DxAttr())
    {
      _dx->SetBaseValInternal(svg::ToFloatOrZero(string));
    }
    else if (name == SVGNames::DyAttr())
    {
      _dy->SetBaseValInternal(svg::ToFloatOrZero(string));
    }

    SVGFilterPrimitiveStandardAttributes::AttributeChanged(name, oldValue, value);
  }

  void SVGFEDropShadowElement::SVGAttributeChanged(const QualifiedName &attributeName) noexcept
  {
    if (attributeName == SVGNames::InAttr())
    {
      InstanceInvalidationGuard guard(*this);
      UpdateSVGRendererForElementChange();
      return;
    }

    if (attributeName == SVGNames::StdDeviationAttr() && (StdDeviationX() < 0 || StdDeviationY() < 0))
    {
      InstanceInvalidationGuard guard(*this);
      MarkFilterEffectForRebuild();
      return;
    }

    if (attributeName == SVGNames::StdDeviationAttr() || attributeName == SVGNames::DxAttr()
        || attributeName == SVGNames::DyAttr())
    {
      InstanceInvalidationGuard guard(*this);
      PrimitiveAttributeChanged(attributeName);
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  bool SVGFEDropShadowElement::SetFilterEffectAttribute(FilterEffect &filterEffect,
                                                        const QualifiedName &attributeName) noexcept
  {
    auto &effect = Downcast<FEDropShadow>(filterEffect);
    if (attributeName == SVGNames::StdDeviationAttr())
    {
      return effect.SetStdDeviationX(StdDeviationX()) || effect.SetStdDeviationY(StdDeviationY());
    }
    if (attributeName == SVGNames::DxAttr())
    {
      return effect.SetDx(Dx());
    }
    if (attributeName == SVGNames::DyAttr())
    {
      return effect.SetDy(Dy());
    }
    if (attributeName == SVGNames::FloodColorAttr())
    {
      const auto &style = Renderer()->Style();
      return effect.SetShadowColor(style.ColorResolvingCurrentColor(style.FloodColor()));
    }
    if (attributeName == SVGNames::FloodOpacityAttr())
    {
      return effect.SetShadowOpacity(Renderer()->Style().FloodOpacity().Value.Value);
    }

    krys_debug_assert_msg(false, "Not reached");
    return false;
  }

  bool SVGFEDropShadowElement::IsIdentity() const noexcept
  {
    return !StdDeviationX() && !StdDeviationY() && !Dx() && !Dy();
  }

  IntOutsets SVGFEDropShadowElement::Outsets(const RectF &targetBoundingBox,
                                             SVGUnitTypes::SVGUnitType primitiveUnits) const noexcept
  {
    auto offset = SVGFilterRenderer::CalculateResolvedSize({Dx(), Dy()}, targetBoundingBox, primitiveUnits);
    auto stdDeviation = SVGFilterRenderer::CalculateResolvedSize({StdDeviationX(), StdDeviationY()},
                                                                 targetBoundingBox, primitiveUnits);
    return FEDropShadow::CalculateOutsets(offset, stdDeviation);
  }

  RefPtr<FilterEffect> SVGFEDropShadowElement::CreateFilterEffect(const FilterEffectVector &,
                                                                  const GraphicsContext &) const noexcept
  {
    auto *renderer = Renderer();
    if (!renderer)
    {
      return nullptr;
    }

    if (StdDeviationX() < 0 || StdDeviationY() < 0)
    {
      return nullptr;
    }

    const auto &style = renderer->Style();

    return FEDropShadow::Create(StdDeviationX(), StdDeviationY(), Dx(), Dy(),
                                style.ColorResolvingCurrentColor(style.FloodColor()),
                                style.FloodOpacity().Value.Value);
  }
}
