/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
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

#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Rendering/SVG/Legacy/LegacyRenderSVGResourceContainer.hpp"

namespace krys::web
{
  class SVGMarkerElement;
  enum class SVGMarkerUnitsType : uint8;

  /// @brief The renderer of a `<marker>`: draws its children at the vertices of the shapes that reference it.
  class LegacyRenderSVGResourceMarker final : public LegacyRenderSVGResourceContainer
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGResourceMarker);

  private:
    mutable AffineTransform _localToParentTransform;
    RectF _viewport;

  public:
    LegacyRenderSVGResourceMarker(SVGMarkerElement &, RenderStyle &&) noexcept;

    ~LegacyRenderSVGResourceMarker() noexcept override;

    KRYS_NODISCARD SVGMarkerElement &MarkerElement() const noexcept;

    void RemoveAllClientsFromCache() noexcept override
    {
    }

    void RemoveClientFromCache(RenderElement &) noexcept override
    {
    }

    void Draw(PaintInfo &, const AffineTransform &) noexcept;

    // Calculates marker boundaries, mapped to the target element's coordinate space
    KRYS_NODISCARD RectF MarkerBoundaries(RepaintRectCalculation,
                                          const AffineTransform &markerTransformation) const noexcept;

    void ApplyViewportClip(PaintInfo &) noexcept override;

    void Layout() noexcept override;

    void CalcViewport() noexcept override;

    KRYS_NODISCARD const AffineTransform &LocalToParentTransform() const noexcept override;

    KRYS_NODISCARD AffineTransform MarkerTransformation(const PointF &origin, float angle,
                                                        float strokeWidth) const noexcept;

    EnumSet<ApplyResult> ApplyResource(RenderElement &, const RenderStyle &, GraphicsContext *&,
                                       EnumSet<RenderSVGResourceMode>) noexcept override
    {
      return {};
    }

    KRYS_NODISCARD RectF ResourceBoundingBox(const RenderObject &, RepaintRectCalculation) noexcept override
    {
      return RectF();
    }

    KRYS_NODISCARD PointF ReferencePoint() const noexcept;

    KRYS_NODISCARD Maybe<float> Angle() const noexcept;

    KRYS_NODISCARD SVGMarkerUnitsType MarkerUnits() const noexcept;

    KRYS_NODISCARD RenderSVGResourceType ResourceType() const noexcept override
    {
      return RenderSVGResourceType::MarkerResourceType;
    }

    void GetElement() const = delete;

  private:
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGResourceMarker";
    }

    // Generates a transformation matrix usable to render marker content. Handles scaling the marker content
    // acording to SVGs markerUnits="strokeWidth" concept, when a strokeWidth value != -1 is passed in.
    KRYS_NODISCARD AffineTransform MarkerContentTransformation(const AffineTransform &contentTransformation,
                                                               const PointF &origin,
                                                               float strokeWidth = -1) const noexcept;

    KRYS_NODISCARD AffineTransform ViewportTransform() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::LegacyRenderSVGResourceMarker)
  KRYS_NODISCARD static bool IsType(const krys::web::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGResourceMarker();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
