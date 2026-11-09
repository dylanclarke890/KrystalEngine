/*
 * Copyright (C) 2004, 2005 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
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

#include "Krystal.Booey/SVG/SVGAnimateElementBase.hpp"

namespace krys::boo::svg
{
  /// @brief The `<set>` element: sets an attribute or property to a value for its interval.
  /// @note `SVGAnimateElement` implements a superset of the functionality.
  /// @see https://svgwg.org/specs/animations/#SetElement
  class SVGSetElement final : public SVGAnimateElementBase
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGSetElement);

  public:
    explicit SVGSetElement(dom::Document &document) noexcept
        : SVGAnimateElementBase(document, SVGElementInterface::Set)
    {
      SetAnimationMode(AnimationMode::To);
    }

  private:
    void UpdateAnimationMode() noexcept override
    {
      // No-op, as <set> has a constant animation mode of AnimationMode::To.
      // See: http://www.w3.org/TR/SVG/single-page.html#animate-SetElement
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGSetElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &element) noexcept
  {
    return element.Interface() == krys::boo::svg::SVGElementInterface::Set;
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *svgElement = krys::DynamicDowncast<krys::boo::svg::SVGElement>(node);
    return svgElement && IsType(*svgElement);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
