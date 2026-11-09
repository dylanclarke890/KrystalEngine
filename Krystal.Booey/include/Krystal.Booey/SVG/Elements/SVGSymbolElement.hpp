/*
 * Copyright (C) 2004, 2005 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
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

#include "Krystal.Booey/SVG/Elements/SVGGraphicsElement.hpp"
#include "Krystal.Booey/SVG/SVGFitToViewBox.hpp"

namespace krys::boo::svg
{
  /// @brief The `<symbol>` element: graphics drawn only where `<use>` instantiates them.
  /// @see https://svgwg.org/svg2-draft/struct.html#InterfaceSVGSymbolElement
  class SVGSymbolElement final : public SVGGraphicsElement, public SVGFitToViewBox
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGSymbolElement);

  public:
    explicit SVGSymbolElement(dom::Document &document) noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGSymbolElement, SVGGraphicsElement, SVGFitToViewBox>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept override;
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept override;
    KRYS_NODISCARD bool SelfHasRelativeLengths() const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGSymbolElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGSymbolElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
