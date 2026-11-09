/*
 * Copyright (C) 2004, 2005, 2007, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
 * Copyright (C) 2018-2019 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Elements/SVGGraphicsElement.hpp"
#include "Krystal.Booey/SVG/SVGUnitTypes.hpp"

namespace krys::boo::svg
{
  /// @brief The `<clipPath>` element: the shapes that clip the elements that reference it.
  /// @see https://drafts.fxtf.org/css-masking/#InterfaceSVGClipPathElement
  class SVGClipPathElement final : public SVGGraphicsElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGClipPathElement);

  private:
    Ref<SVGAnimatedEnumeration> _clipPathUnits;

  public:
    explicit SVGClipPathElement(dom::Document &document) noexcept;

    KRYS_NODISCARD SVGUnitTypes::SVGUnitType ClipPathUnits() const noexcept
    {
      return _clipPathUnits->CurrentValue<SVGUnitTypes::SVGUnitType>();
    }
    KRYS_NODISCARD SVGAnimatedEnumeration &ClipPathUnitsAnimated() noexcept
    {
      return *_clipPathUnits;
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGClipPathElement, SVGGraphicsElement>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;
    void OnChildrenChanged() noexcept final;

    KRYS_NODISCARD bool IsValid() const noexcept final
    {
      return SVGTests::IsValid();
    }
    KRYS_NODISCARD bool NeedsPendingResourceHandling() const noexcept final
    {
      return false;
    }

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGClipPathElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGClipPathElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
