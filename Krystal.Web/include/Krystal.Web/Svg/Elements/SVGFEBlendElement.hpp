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

#pragma once

#include "Krystal.Web/Graphics/GraphicsTypes.hpp"
#include "Krystal.Web/Svg/SVGFilterPrimitiveStandardAttributes.hpp"

namespace krys::web
{
  template <>
  struct SVGPropertyTraits<BlendMode>
  {
    KRYS_NODISCARD static uint32 HighestEnumValue() noexcept
    {
      return static_cast<uint32>(BlendMode::Luminosity);
    }

    KRYS_NODISCARD static BlendMode FromString(SVGElement &, DOMStringView string) noexcept
    {
      BlendMode mode = BlendMode::Normal;
      ParseBlendMode(string, mode);
      return mode;
    }

    KRYS_NODISCARD static DOMString ToString(BlendMode type) noexcept
    {
      if (type < BlendMode::PlusDarker)
      {
        auto name = BlendModeName(type);
        return DOMString(name.begin(), name.end());
      }

      return {};
    }
  };

  /// @brief The `<feBlend>` element: its first input blended over its second.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEBlendElement
  class SVGFEBlendElement final : public SVGFilterPrimitiveStandardAttributes
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEBlendElement);

  private:
    Ref<SVGAnimatedString> _in1;
    Ref<SVGAnimatedString> _in2;
    Ref<SVGAnimatedEnumeration> _mode;

  public:
    explicit SVGFEBlendElement(Document &document) noexcept;

    KRYS_NODISCARD DOMString In1() const noexcept
    {
      return _in1->CurrentValue();
    }

    KRYS_NODISCARD DOMString In2() const noexcept
    {
      return _in2->CurrentValue();
    }

    KRYS_NODISCARD BlendMode Mode() const noexcept
    {
      return _mode->CurrentValue<BlendMode>();
    }

    KRYS_NODISCARD SVGAnimatedString &In1Animated() noexcept
    {
      return *_in1;
    }

    KRYS_NODISCARD SVGAnimatedString &In2Animated() noexcept
    {
      return *_in2;
    }

    KRYS_NODISCARD SVGAnimatedEnumeration &ModeAnimated() noexcept
    {
      return *_mode;
    }

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGFEBlendElement, SVGFilterPrimitiveStandardAttributes>;

  private:
    void AttributeChanged(const QualifiedName &name, Maybe<DOMStringView> oldValue,
                          Maybe<DOMStringView> value) noexcept final;

    void SVGAttributeChanged(const QualifiedName &attributeName) noexcept final;

    bool SetFilterEffectAttribute(FilterEffect &effect, const QualifiedName &attributeName) noexcept final;

    KRYS_NODISCARD List<DOMStringAtom> FilterEffectInputsNames() const noexcept final
    {
      return {DOMStringAtom(In1()), DOMStringAtom(In2())};
    }

    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateFilterEffect(const FilterEffectVector &inputs,
                         const GraphicsContext &destinationContext) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::SVGFEBlendElement)
  KRYS_NODISCARD static bool IsType(const krys::web::SVGElement &target) noexcept
  {
    return target.IsSVGFEBlendElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
