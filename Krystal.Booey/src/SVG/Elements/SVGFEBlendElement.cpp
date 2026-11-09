/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
 * Copyright (C) 2014 Adobe Systems Incorporated. All rights reserved.
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

#include "Krystal.Booey/SVG/Elements/SVGFEBlendElement.hpp"
#include "Krystal.Booey/Graphics/Filters/FEBlend.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGFEBlendElement::SVGFEBlendElement(dom::Document &document) noexcept
      : SVGFilterPrimitiveStandardAttributes(document, CreateUnique<PropertyRegistry>(*this),
                                             SVGElementInterface::FEBlend),
        _in1(SVGAnimatedString::Create(this)), _in2(SVGAnimatedString::Create(this)),
        _mode(SVGAnimatedEnumeration::Create(this, BlendMode::Normal))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<BlendMode, &SVGFEBlendElement::_mode>(SVGNames::ModeAttr());
      PropertyRegistry::RegisterProperty<&SVGFEBlendElement::_in1>(SVGNames::InAttr());
      PropertyRegistry::RegisterProperty<&SVGFEBlendElement::_in2>(SVGNames::In2Attr());
    }
  }

  void SVGFEBlendElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                           Maybe<dom::DOMStringView> value) noexcept
  {
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::ModeAttr())
    {
      BlendMode mode = BlendMode::Normal;
      if (ParseBlendMode(string, mode))
      {
        _mode->SetBaseValInternal<BlendMode>(mode);
      }
    }
    else if (name == SVGNames::InAttr())
    {
      _in1->SetBaseValInternal(dom::DOMString(string));
    }
    else if (name == SVGNames::In2Attr())
    {
      _in2->SetBaseValInternal(dom::DOMString(string));
    }

    SVGFilterPrimitiveStandardAttributes::AttributeChanged(name, oldValue, value);
  }

  bool SVGFEBlendElement::SetFilterEffectAttribute(FilterEffect &effect,
                                                   const dom::QualifiedName &attributeName) noexcept
  {
    auto &feBlend = Downcast<FEBlend>(effect);
    if (attributeName == SVGNames::ModeAttr())
    {
      return feBlend.SetBlendMode(Mode());
    }

    krys_debug_assert_msg(false, "Not reached");
    return false;
  }

  void SVGFEBlendElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      if (attributeName == SVGNames::ModeAttr())
      {
        PrimitiveAttributeChanged(attributeName);
      }
      else
      {
        krys_debug_assert(attributeName == SVGNames::InAttr() || attributeName == SVGNames::In2Attr());
        UpdateSVGRendererForElementChange();
      }
      return;
    }

    SVGFilterPrimitiveStandardAttributes::SVGAttributeChanged(attributeName);
  }

  RefPtr<FilterEffect> SVGFEBlendElement::CreateFilterEffect(const FilterEffectVector &,
                                                             const GraphicsContext &) const noexcept
  {
    return FEBlend::Create(Mode());
  }
}
