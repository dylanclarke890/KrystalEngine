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

#pragma once

#include "Krystal.Web/Svg/SVGFilterPrimitiveStandardAttributes.hpp"

namespace krys::web
{
  /// @brief The `<feTile>` element: its subregion, tiled with its input.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFETileElement
  class SVGFETileElement final : public SVGFilterPrimitiveStandardAttributes
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFETileElement);

  private:
    Ref<SVGAnimatedString> _in1;

  public:
    explicit SVGFETileElement(Document &document) noexcept;

    KRYS_NODISCARD DOMString In1() const noexcept
    {
      return _in1->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedString &In1Animated() noexcept
    {
      return *_in1;
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGFETileElement, SVGFilterPrimitiveStandardAttributes>;

  private:
    void AttributeChanged(const QualifiedName &name, Maybe<DOMStringView> oldValue,
                          Maybe<DOMStringView> value) noexcept final;

    void SVGAttributeChanged(const QualifiedName &attributeName) noexcept final;

    KRYS_NODISCARD List<DOMStringAtom> FilterEffectInputsNames() const noexcept final
    {
      return {DOMStringAtom(In1())};
    }

    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateFilterEffect(const FilterEffectVector &inputs,
                         const GraphicsContext &destinationContext) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::SVGFETileElement)
  KRYS_NODISCARD static bool IsType(const krys::web::SVGElement &target) noexcept
  {
    return target.IsSVGFETileElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
