/*
 * Copyright (C) 2009 Dirk Schulze <krit@webkit.org>
 * Copyright (C) 2018-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2014 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Filters/FEMorphology.hpp"
#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"

namespace krys::boo::svg
{
  template <>
  struct SVGPropertyTraits<MorphologyOperatorType>
  {
    KRYS_NODISCARD static uint32 HighestEnumValue() noexcept
    {
      return static_cast<uint32>(MorphologyOperatorType::Dilate);
    }

    KRYS_NODISCARD static dom::DOMString ToString(MorphologyOperatorType type) noexcept
    {
      switch (type)
      {
        case MorphologyOperatorType::Unknown: return {};
        case MorphologyOperatorType::Erode:   return u8"erode";
        case MorphologyOperatorType::Dilate:  return u8"dilate";
      }

      krys_assert_msg(false, "Not reached");
      return {};
    }

    KRYS_NODISCARD static MorphologyOperatorType FromString(SVGElement &, dom::DOMStringView value) noexcept
    {
      if (value == u8"erode")
      {
        return MorphologyOperatorType::Erode;
      }
      if (value == u8"dilate")
      {
        return MorphologyOperatorType::Dilate;
      }
      return MorphologyOperatorType::Unknown;
    }
  };

  /// @brief The `<feMorphology>` element: its input thinned (eroded) or fattened (dilated).
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEMorphologyElement
  class SVGFEMorphologyElement final : public SVGFilterPrimitiveStandardAttributes
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEMorphologyElement);

  private:
    Ref<SVGAnimatedString> _in1;
    Ref<SVGAnimatedEnumeration> _svgOperator;
    Ref<SVGAnimatedNumber> _radiusX;
    Ref<SVGAnimatedNumber> _radiusY;

  public:
    explicit SVGFEMorphologyElement(dom::Document &document) noexcept;

    KRYS_NODISCARD dom::DOMString In1() const noexcept
    {
      return _in1->CurrentValue();
    }
    KRYS_NODISCARD MorphologyOperatorType SVGOperator() const noexcept
    {
      return _svgOperator->CurrentValue<MorphologyOperatorType>();
    }
    KRYS_NODISCARD float RadiusX() const noexcept
    {
      return _radiusX->CurrentValue();
    }
    KRYS_NODISCARD float RadiusY() const noexcept
    {
      return _radiusY->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedString &In1Animated() noexcept
    {
      return *_in1;
    }
    KRYS_NODISCARD SVGAnimatedEnumeration &SVGOperatorAnimated() noexcept
    {
      return *_svgOperator;
    }
    KRYS_NODISCARD SVGAnimatedNumber &RadiusXAnimated() noexcept
    {
      return *_radiusX;
    }
    KRYS_NODISCARD SVGAnimatedNumber &RadiusYAnimated() noexcept
    {
      return *_radiusY;
    }

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGFEMorphologyElement, SVGFilterPrimitiveStandardAttributes>;

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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFEMorphologyElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFEMorphologyElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
