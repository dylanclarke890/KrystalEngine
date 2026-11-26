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

#include "Krystal.Web/Rendering/SVG/Legacy/LegacyRenderSVGResourceGradient.hpp"
#include "Krystal.Web/Svg/LinearGradientAttributes.hpp"

namespace krys::web
{
  class SVGLinearGradientElement;

  /// @brief The renderer of a `<linearGradient>`.
  class LegacyRenderSVGResourceLinearGradient final : public LegacyRenderSVGResourceGradient
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGResourceLinearGradient);

  private:
    svg::LinearGradientAttributes _attributes;

  public:
    LegacyRenderSVGResourceLinearGradient(SVGLinearGradientElement &, RenderStyle &&) noexcept;

    ~LegacyRenderSVGResourceLinearGradient() noexcept override;

    KRYS_NODISCARD SVGLinearGradientElement &LinearGradientElement() const noexcept;

    KRYS_NODISCARD PointF StartPoint(const svg::LinearGradientAttributes &) const noexcept;

    KRYS_NODISCARD PointF EndPoint(const svg::LinearGradientAttributes &) const noexcept;

    void GradientElement() const = delete;

  private:
    KRYS_NODISCARD RenderSVGResourceType ResourceType() const noexcept final
    {
      return RenderSVGResourceType::LinearGradientResourceType;
    }

    KRYS_NODISCARD SVGUnitTypes::SVGUnitType GradientUnits() const noexcept final
    {
      return _attributes.GradientUnits();
    }

    KRYS_NODISCARD AffineTransform GradientTransform() const noexcept final
    {
      return _attributes.GradientTransform();
    }

    bool CollectGradientAttributes() noexcept final;

    KRYS_NODISCARD Ref<Gradient> BuildGradient() const noexcept final;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderSVGResourceLinearGradient";
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::LegacyRenderSVGResourceLinearGradient)
  KRYS_NODISCARD static bool IsType(const krys::web::RenderObject &renderer) noexcept
  {
    return renderer.GetType() == krys::web::RenderObject::Type::LegacySVGResourceLinearGradient;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
