/*
 * Copyright (C) 2005 Alexander Kellett <lypanov@kde.org>
 * Copyright (C) 2018-2024 Apple Inc. All rights reserved.
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

#include "Krystal.Web/Svg/Elements/SVGElement.hpp"
#include "Krystal.Web/Svg/Properties/SVGAnimatedPropertyImpl.hpp"
#include "Krystal.Web/Svg/SVGTests.hpp"
#include "Krystal.Web/Svg/SVGUnitTypes.hpp"

namespace krys::web
{
  /// @brief The `<mask>` element: graphics whose luminance or alpha masks the elements that reference it.
  /// @see https://drafts.fxtf.org/css-masking/#InterfaceSVGMaskElement
  class SVGMaskElement final : public SVGElement, public SVGTests
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGMaskElement);

  private:
    Ref<SVGAnimatedLength> _x;
    Ref<SVGAnimatedLength> _y;
    Ref<SVGAnimatedLength> _width;
    Ref<SVGAnimatedLength> _height;
    Ref<SVGAnimatedEnumeration> _maskUnits;
    Ref<SVGAnimatedEnumeration> _maskContentUnits;

  public:
    explicit SVGMaskElement(Document &document) noexcept;

    KRYS_NODISCARD const SVGLengthValue &X() const noexcept
    {
      return _x->CurrentValue();
    }

    KRYS_NODISCARD const SVGLengthValue &Y() const noexcept
    {
      return _y->CurrentValue();
    }

    KRYS_NODISCARD const SVGLengthValue &Width() const noexcept
    {
      return _width->CurrentValue();
    }

    KRYS_NODISCARD const SVGLengthValue &Height() const noexcept
    {
      return _height->CurrentValue();
    }

    KRYS_NODISCARD SVGUnitTypes::SVGUnitType MaskUnits() const noexcept
    {
      return _maskUnits->CurrentValue<SVGUnitTypes::SVGUnitType>();
    }

    KRYS_NODISCARD SVGUnitTypes::SVGUnitType MaskContentUnits() const noexcept
    {
      return _maskContentUnits->CurrentValue<SVGUnitTypes::SVGUnitType>();
    }

    KRYS_NODISCARD SVGAnimatedLength &XAnimated() noexcept
    {
      return *_x;
    }

    KRYS_NODISCARD SVGAnimatedLength &YAnimated() noexcept
    {
      return *_y;
    }

    KRYS_NODISCARD SVGAnimatedLength &WidthAnimated() noexcept
    {
      return *_width;
    }

    KRYS_NODISCARD SVGAnimatedLength &HeightAnimated() noexcept
    {
      return *_height;
    }

    KRYS_NODISCARD SVGAnimatedEnumeration &MaskUnitsAnimated() noexcept
    {
      return *_maskUnits;
    }

    KRYS_NODISCARD SVGAnimatedEnumeration &MaskContentUnitsAnimated() noexcept
    {
      return *_maskContentUnits;
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGMaskElement, SVGElement, SVGTests>;

  private:
    void AttributeChanged(const QualifiedName &name, Maybe<DOMStringView> oldValue,
                          Maybe<DOMStringView> value) noexcept final;

    void SVGAttributeChanged(const QualifiedName &attributeName) noexcept final;

    void OnChildrenChanged() noexcept final;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;

    KRYS_NODISCARD bool IsValid() const noexcept final
    {
      return SVGTests::IsValid();
    }

    KRYS_NODISCARD bool NeedsPendingResourceHandling() const noexcept final
    {
      return false;
    }

    KRYS_NODISCARD bool SelfHasRelativeLengths() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::SVGMaskElement)
  KRYS_NODISCARD static bool IsType(const krys::web::SVGElement &target) noexcept
  {
    return target.IsSVGMaskElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
