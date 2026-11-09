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

#pragma once

#include "Krystal.Booey/SVG/Elements/SVGFEConvolveMatrixElement.hpp"
#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"

namespace krys::boo::svg
{
  /// @brief The `<feGaussianBlur>` element: its input, blurred.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEGaussianBlurElement
  class SVGFEGaussianBlurElement final : public SVGFilterPrimitiveStandardAttributes
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEGaussianBlurElement);

  private:
    Ref<SVGAnimatedString> _in1;
    Ref<SVGAnimatedNumber> _stdDeviationX;
    Ref<SVGAnimatedNumber> _stdDeviationY;
    Ref<SVGAnimatedEnumeration> _edgeMode;

  public:
    explicit SVGFEGaussianBlurElement(dom::Document &document) noexcept;

    void SetStdDeviation(float stdDeviationX, float stdDeviationY) noexcept;

    KRYS_NODISCARD dom::DOMString In1() const noexcept
    {
      return _in1->CurrentValue();
    }
    KRYS_NODISCARD float StdDeviationX() const noexcept
    {
      return _stdDeviationX->CurrentValue();
    }
    KRYS_NODISCARD float StdDeviationY() const noexcept
    {
      return _stdDeviationY->CurrentValue();
    }
    KRYS_NODISCARD EdgeModeType EdgeMode() const noexcept
    {
      return _edgeMode->CurrentValue<EdgeModeType>();
    }

    KRYS_NODISCARD SVGAnimatedString &In1Animated() noexcept
    {
      return *_in1;
    }
    KRYS_NODISCARD SVGAnimatedNumber &StdDeviationXAnimated() noexcept
    {
      return *_stdDeviationX;
    }
    KRYS_NODISCARD SVGAnimatedNumber &StdDeviationYAnimated() noexcept
    {
      return *_stdDeviationY;
    }
    KRYS_NODISCARD SVGAnimatedEnumeration &EdgeModeAnimated() noexcept
    {
      return *_edgeMode;
    }

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGFEGaussianBlurElement, SVGFilterPrimitiveStandardAttributes>;

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
    KRYS_NODISCARD bool IsIdentity() const noexcept final;
    KRYS_NODISCARD IntOutsets Outsets(const FloatRect &targetBoundingBox,
                                      SVGUnitTypes::SVGUnitType primitiveUnits) const noexcept final;
    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateFilterEffect(const FilterEffectVector &inputs,
                         const GraphicsContext &destinationContext) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFEGaussianBlurElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFEGaussianBlurElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
