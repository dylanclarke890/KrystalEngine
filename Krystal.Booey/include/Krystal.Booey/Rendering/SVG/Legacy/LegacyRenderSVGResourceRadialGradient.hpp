/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceGradient.hpp"
#include "Krystal.Booey/SVG/RadialGradientAttributes.hpp"

namespace krys::boo::svg
{
  class SVGRadialGradientElement;
}

namespace krys::boo
{
  /// @brief The renderer of a `<radialGradient>`.
  class LegacyRenderSVGResourceRadialGradient final : public LegacyRenderSVGResourceGradient
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGResourceRadialGradient);

  private:
    svg::RadialGradientAttributes _attributes;

  public:
    LegacyRenderSVGResourceRadialGradient(svg::SVGRadialGradientElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGResourceRadialGradient() noexcept override;

    KRYS_NODISCARD svg::SVGRadialGradientElement &RadialGradientElement() const noexcept;

    KRYS_NODISCARD FloatPoint CenterPoint(const svg::RadialGradientAttributes &) const noexcept;
    KRYS_NODISCARD FloatPoint FocalPoint(const svg::RadialGradientAttributes &) const noexcept;
    KRYS_NODISCARD float Radius(const svg::RadialGradientAttributes &) const noexcept;
    KRYS_NODISCARD float FocalRadius(const svg::RadialGradientAttributes &) const noexcept;

  private:
    KRYS_NODISCARD RenderSVGResourceType ResourceType() const noexcept final
    {
      return RenderSVGResourceType::RadialGradientResourceType;
    }

    KRYS_NODISCARD svg::SVGUnitTypes::SVGUnitType GradientUnits() const noexcept final
    {
      return _attributes.GradientUnits();
    }
    KRYS_NODISCARD AffineTransform GradientTransform() const noexcept final
    {
      return _attributes.GradientTransform();
    }
    KRYS_NODISCARD Ref<Gradient> BuildGradient() const noexcept final;

    void GradientElement() const = delete;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderSVGResourceRadialGradient";
    }
    bool CollectGradientAttributes() noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGResourceRadialGradient)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.GetType() == krys::boo::RenderObject::Type::LegacySVGResourceRadialGradient;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
