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

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"

namespace krys::boo::svg
{
  /// @brief The `<stop>` element: a gradient's colour at an offset.
  /// @see https://svgwg.org/svg2-draft/pservers.html#InterfaceSVGStopElement
  class SVGStopElement final : public SVGElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGStopElement);

  private:
    Ref<SVGAnimatedNumber> _offset;

  public:
    explicit SVGStopElement(dom::Document &document) noexcept;

    KRYS_NODISCARD Color StopColorIncludingOpacity() const noexcept;

    KRYS_NODISCARD float Offset() const noexcept
    {
      return _offset->CurrentValue();
    }
    KRYS_NODISCARD SVGAnimatedNumber &OffsetAnimated() noexcept
    {
      return *_offset;
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGStopElement, SVGElement>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;
    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGStopElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGStopElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
