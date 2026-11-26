/*
 * Copyright (C) 2004, 2005, 2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
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
#include "Krystal.Web/Svg/SVGFitToViewBox.hpp"
#include "Krystal.Web/Svg/SVGTests.hpp"
#include "Krystal.Web/Svg/SVGTransformList.hpp"
#include "Krystal.Web/Svg/SVGUnitTypes.hpp"
#include "Krystal.Web/Svg/SVGURIReference.hpp"

namespace krys::web::svg
{
  struct PatternAttributes;
}

namespace krys::web
{
  /// @brief The `<pattern>` element: a tile of graphics repeated to fill or stroke a shape.
  /// @see https://svgwg.org/svg2-draft/pservers.html#InterfaceSVGPatternElement
  class SVGPatternElement final : public SVGElement,
                                  public SVGFitToViewBox,
                                  public SVGTests,
                                  public SVGURIReference
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGPatternElement);

  private:
    Ref<SVGAnimatedLength> _x;
    Ref<SVGAnimatedLength> _y;
    Ref<SVGAnimatedLength> _width;
    Ref<SVGAnimatedLength> _height;
    Ref<SVGAnimatedEnumeration> _patternUnits;
    Ref<SVGAnimatedEnumeration> _patternContentUnits;
    Ref<SVGAnimatedTransformList> _patternTransform;

  public:
    explicit SVGPatternElement(Document &document) noexcept;

    void CollectPatternAttributes(svg::PatternAttributes &) const noexcept;

    KRYS_NODISCARD AffineTransform LocalCoordinateSpaceTransform(svg::CTMScope) const noexcept final;

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

    KRYS_NODISCARD SVGUnitTypes::SVGUnitType PatternUnits() const noexcept
    {
      return _patternUnits->CurrentValue<SVGUnitTypes::SVGUnitType>();
    }

    KRYS_NODISCARD SVGUnitTypes::SVGUnitType PatternContentUnits() const noexcept
    {
      return _patternContentUnits->CurrentValue<SVGUnitTypes::SVGUnitType>();
    }

    KRYS_NODISCARD const SVGTransformList &PatternTransform() const noexcept
    {
      return _patternTransform->CurrentValue();
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

    KRYS_NODISCARD SVGAnimatedEnumeration &PatternUnitsAnimated() noexcept
    {
      return *_patternUnits;
    }

    KRYS_NODISCARD SVGAnimatedEnumeration &PatternContentUnitsAnimated() noexcept
    {
      return *_patternContentUnits;
    }

    KRYS_NODISCARD SVGAnimatedTransformList &PatternTransformAnimated() noexcept
    {
      return *_patternTransform;
    }

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGPatternElement, SVGElement, SVGFitToViewBox, SVGTests, SVGURIReference>;

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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::SVGPatternElement)
  KRYS_NODISCARD static bool IsType(const krys::web::SVGElement &target) noexcept
  {
    return target.IsSVGPatternElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
