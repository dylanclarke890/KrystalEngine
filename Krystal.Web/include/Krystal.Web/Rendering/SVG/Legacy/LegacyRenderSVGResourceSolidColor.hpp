/*
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

#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Web/Graphics/Color/Color.hpp"
#include "Krystal.Web/Rendering/SVG/Legacy/LegacyRenderSVGResource.hpp"

namespace krys::web
{
  /// @brief The paint server of a plain colour `fill` or `stroke`.
  class LegacyRenderSVGResourceSolidColor final : public LegacyRenderSVGResource
  {
  private:
    Color _color;

  public:
    LegacyRenderSVGResourceSolidColor() noexcept = default;

    void RemoveAllClientsFromCache() noexcept override
    {
    }

    void RemoveAllClientsFromCacheAndMarkForInvalidationIfNeeded(bool, VisitedRenderers *) noexcept override
    {
    }

    void RemoveClientFromCache(RenderElement &) noexcept override
    {
    }

    void RemoveClientFromCacheAndMarkForInvalidation(RenderElement &, bool = true) noexcept override
    {
    }

    EnumSet<ApplyResult> ApplyResource(RenderElement &, const RenderStyle &, GraphicsContext *&,
                                       EnumSet<RenderSVGResourceMode>) noexcept override;

    void PostApplyResource(RenderElement &, GraphicsContext *&, EnumSet<RenderSVGResourceMode>, const Path *,
                           const RenderElement *) noexcept override;

    KRYS_NODISCARD RectF ResourceBoundingBox(const RenderObject &, RepaintRectCalculation) noexcept override
    {
      return RectF();
    }

    KRYS_NODISCARD RenderSVGResourceType ResourceType() const noexcept override
    {
      return RenderSVGResourceType::SolidColorResourceType;
    }

    KRYS_NODISCARD const Color &GetColor() const noexcept
    {
      return _color;
    }

    void SetColor(const Color &color) noexcept
    {
      _color = color;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::LegacyRenderSVGResourceSolidColor)
  KRYS_NODISCARD static bool IsType(const krys::web::LegacyRenderSVGResource &resource) noexcept
  {
    return resource.ResourceType() == krys::web::RenderSVGResourceType::SolidColorResourceType;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
