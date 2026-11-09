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

#include "Krystal.Booey/Graphics/Filters/FEColorMatrix.hpp"
#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"

namespace krys::boo::svg
{
  template <>
  struct SVGPropertyTraits<ColorMatrixType>
  {
    KRYS_NODISCARD static uint32 HighestEnumValue() noexcept
    {
      return static_cast<uint32>(ColorMatrixType::FECOLORMATRIX_TYPE_LUMINANCETOALPHA);
    }

    KRYS_NODISCARD static dom::DOMString ToString(ColorMatrixType type) noexcept
    {
      switch (type)
      {
        case ColorMatrixType::FECOLORMATRIX_TYPE_UNKNOWN:          return {};
        case ColorMatrixType::FECOLORMATRIX_TYPE_MATRIX:           return u8"matrix";
        case ColorMatrixType::FECOLORMATRIX_TYPE_SATURATE:         return u8"saturate";
        case ColorMatrixType::FECOLORMATRIX_TYPE_HUEROTATE:        return u8"hueRotate";
        case ColorMatrixType::FECOLORMATRIX_TYPE_LUMINANCETOALPHA: return u8"luminanceToAlpha";
      }

      krys_assert_msg(false, "Not reached");
      return {};
    }

    KRYS_NODISCARD static ColorMatrixType FromString(SVGElement &, dom::DOMStringView value) noexcept
    {
      if (value == u8"matrix")
      {
        return ColorMatrixType::FECOLORMATRIX_TYPE_MATRIX;
      }
      if (value == u8"saturate")
      {
        return ColorMatrixType::FECOLORMATRIX_TYPE_SATURATE;
      }
      if (value == u8"hueRotate")
      {
        return ColorMatrixType::FECOLORMATRIX_TYPE_HUEROTATE;
      }
      if (value == u8"luminanceToAlpha")
      {
        return ColorMatrixType::FECOLORMATRIX_TYPE_LUMINANCETOALPHA;
      }
      return ColorMatrixType::FECOLORMATRIX_TYPE_UNKNOWN;
    }
  };

  /// @brief The `<feColorMatrix>` element: its input's colours, transformed by a matrix.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEColorMatrixElement
  class SVGFEColorMatrixElement final : public SVGFilterPrimitiveStandardAttributes
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEColorMatrixElement);

  private:
    Ref<SVGAnimatedString> _in1;
    Ref<SVGAnimatedEnumeration> _type;
    Ref<SVGAnimatedNumberList> _values;

  public:
    explicit SVGFEColorMatrixElement(dom::Document &document) noexcept;

    KRYS_NODISCARD dom::DOMString In1() const noexcept
    {
      return _in1->CurrentValue();
    }
    KRYS_NODISCARD ColorMatrixType GetType() const noexcept
    {
      return _type->CurrentValue<ColorMatrixType>();
    }
    KRYS_NODISCARD List<float> Values() const noexcept
    {
      return _values->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedString &In1Animated() noexcept
    {
      return *_in1;
    }
    KRYS_NODISCARD SVGAnimatedEnumeration &TypeAnimated() noexcept
    {
      return *_type;
    }
    KRYS_NODISCARD SVGAnimatedNumberList &ValuesAnimated() noexcept
    {
      return *_values;
    }

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGFEColorMatrixElement, SVGFilterPrimitiveStandardAttributes>;

  private:
    KRYS_NODISCARD bool IsInvalidValuesLength() const noexcept;

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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFEColorMatrixElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFEColorMatrixElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
