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
#include "Krystal.Booey/SVG/LinearGradientAttributes.hpp"

namespace krys::boo::svg
{
  class SVGLinearGradientElement;
}

namespace krys::boo
{
  /// @brief The renderer of a `<linearGradient>`.
  class LegacyRenderSVGResourceLinearGradient final : public LegacyRenderSVGResourceGradient
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGResourceLinearGradient);

  private:
    svg::LinearGradientAttributes _attributes;

  public:
    LegacyRenderSVGResourceLinearGradient(svg::SVGLinearGradientElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGResourceLinearGradient() noexcept override;

    KRYS_NODISCARD svg::SVGLinearGradientElement &LinearGradientElement() const noexcept;

    KRYS_NODISCARD FloatPoint StartPoint(const svg::LinearGradientAttributes &) const noexcept;
    KRYS_NODISCARD FloatPoint EndPoint(const svg::LinearGradientAttributes &) const noexcept;

  private:
    KRYS_NODISCARD RenderSVGResourceType ResourceType() const noexcept final
    {
      return RenderSVGResourceType::LinearGradientResourceType;
    }

    KRYS_NODISCARD svg::SVGUnitTypes::SVGUnitType GradientUnits() const noexcept final
    {
      return _attributes.GradientUnits();
    }
    KRYS_NODISCARD AffineTransform GradientTransform() const noexcept final
    {
      return _attributes.GradientTransform();
    }
    bool CollectGradientAttributes() noexcept final;
    KRYS_NODISCARD Ref<Gradient> BuildGradient() const noexcept final;

    void GradientElement() const = delete;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderSVGResourceLinearGradient";
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGResourceLinearGradient)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.GetType() == krys::boo::RenderObject::Type::LegacySVGResourceLinearGradient;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
