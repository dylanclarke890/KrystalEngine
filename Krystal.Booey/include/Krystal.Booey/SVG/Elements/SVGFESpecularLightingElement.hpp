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

#pragma once

#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"

namespace krys::boo::svg
{
  class SVGFELightElement;

  /// @brief The `<feSpecularLighting>` element: its input's alpha as a bump map, with its light source
  /// child's specular reflection.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFESpecularLightingElement
  class SVGFESpecularLightingElement final : public SVGFilterPrimitiveStandardAttributes
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFESpecularLightingElement);

  private:
    Ref<SVGAnimatedString> _in1;
    Ref<SVGAnimatedNumber> _specularConstant;
    Ref<SVGAnimatedNumber> _specularExponent;
    Ref<SVGAnimatedNumber> _surfaceScale;
    Ref<SVGAnimatedNumber> _kernelUnitLengthX;
    Ref<SVGAnimatedNumber> _kernelUnitLengthY;

  public:
    explicit SVGFESpecularLightingElement(dom::Document &document) noexcept;

    void LightElementAttributeChanged(const SVGFELightElement *lightElement,
                                      const dom::QualifiedName &attributeName) noexcept;

    KRYS_NODISCARD dom::DOMString In1() const noexcept
    {
      return _in1->CurrentValue();
    }
    KRYS_NODISCARD float SpecularConstant() const noexcept
    {
      return _specularConstant->CurrentValue();
    }
    KRYS_NODISCARD float SpecularExponent() const noexcept
    {
      return _specularExponent->CurrentValue();
    }
    KRYS_NODISCARD float SurfaceScale() const noexcept
    {
      return _surfaceScale->CurrentValue();
    }
    KRYS_NODISCARD float KernelUnitLengthX() const noexcept
    {
      return _kernelUnitLengthX->CurrentValue();
    }
    KRYS_NODISCARD float KernelUnitLengthY() const noexcept
    {
      return _kernelUnitLengthY->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedString &In1Animated() noexcept
    {
      return *_in1;
    }
    KRYS_NODISCARD SVGAnimatedNumber &SpecularConstantAnimated() noexcept
    {
      return *_specularConstant;
    }
    KRYS_NODISCARD SVGAnimatedNumber &SpecularExponentAnimated() noexcept
    {
      return *_specularExponent;
    }
    KRYS_NODISCARD SVGAnimatedNumber &SurfaceScaleAnimated() noexcept
    {
      return *_surfaceScale;
    }
    KRYS_NODISCARD SVGAnimatedNumber &KernelUnitLengthXAnimated() noexcept
    {
      return *_kernelUnitLengthX;
    }
    KRYS_NODISCARD SVGAnimatedNumber &KernelUnitLengthYAnimated() noexcept
    {
      return *_kernelUnitLengthY;
    }

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGFESpecularLightingElement, SVGFilterPrimitiveStandardAttributes>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;

    bool SetFilterEffectAttribute(FilterEffect &effect,
                                  const dom::QualifiedName &attributeName) noexcept final;
    KRYS_NODISCARD List<dom::DOMStringAtom> FilterEffectInputsNames() const noexcept final
    {
      return {dom::DOMStringAtom(In1())};
    }
    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateFilterEffect(const FilterEffectVector &inputs,
                         const GraphicsContext &destinationContext) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFESpecularLightingElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFESpecularLightingElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
