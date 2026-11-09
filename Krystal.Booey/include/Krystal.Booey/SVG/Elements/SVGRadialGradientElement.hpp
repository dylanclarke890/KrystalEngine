/*
 * Copyright (C) 2004, 2005, 2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>
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

#include "Krystal.Booey/SVG/Elements/SVGGradientElement.hpp"

namespace krys::boo::svg
{
  struct RadialGradientAttributes;

  /// @brief The `<radialGradient>` element: a gradient between a focal circle and an end circle.
  /// @see https://svgwg.org/svg2-draft/pservers.html#InterfaceSVGRadialGradientElement
  class SVGRadialGradientElement final : public SVGGradientElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGRadialGradientElement);

  private:
    Ref<SVGAnimatedLength> _cx;
    Ref<SVGAnimatedLength> _cy;
    Ref<SVGAnimatedLength> _r;
    Ref<SVGAnimatedLength> _fx;
    Ref<SVGAnimatedLength> _fy;
    Ref<SVGAnimatedLength> _fr;

  public:
    explicit SVGRadialGradientElement(dom::Document &document) noexcept;

    bool CollectGradientAttributes(RadialGradientAttributes &) noexcept;

    KRYS_NODISCARD const SVGLengthValue &Cx() const noexcept
    {
      return _cx->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Cy() const noexcept
    {
      return _cy->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &R() const noexcept
    {
      return _r->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Fx() const noexcept
    {
      return _fx->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Fy() const noexcept
    {
      return _fy->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Fr() const noexcept
    {
      return _fr->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedLength &CxAnimated() noexcept
    {
      return *_cx;
    }
    KRYS_NODISCARD SVGAnimatedLength &CyAnimated() noexcept
    {
      return *_cy;
    }
    KRYS_NODISCARD SVGAnimatedLength &RAnimated() noexcept
    {
      return *_r;
    }
    KRYS_NODISCARD SVGAnimatedLength &FxAnimated() noexcept
    {
      return *_fx;
    }
    KRYS_NODISCARD SVGAnimatedLength &FyAnimated() noexcept
    {
      return *_fy;
    }
    KRYS_NODISCARD SVGAnimatedLength &FrAnimated() noexcept
    {
      return *_fr;
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGRadialGradientElement, SVGGradientElement>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;

    KRYS_NODISCARD bool SelfHasRelativeLengths() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGRadialGradientElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGRadialGradientElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
