/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2006 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Google, Inc.
 * Copyright (C) 2011 Renata Hodovan <reni@webkit.org>
 * Copyright (C) 2011 University of Szeged
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

#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGShape.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGMarkerData.hpp"

namespace krys::boo
{
  /// @brief The renderer of `<path>`, `<line>`, `<polygon>` and `<polyline>`: it also draws the caps of zero
  /// length subpaths.
  class LegacyRenderSVGPath final : public LegacyRenderSVGShape
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGPath);

  private:
    List<FloatPoint> _zeroLengthLinecapLocations;
    List<MarkerPosition> _markerPositions;

  public:
    LegacyRenderSVGPath(svg::SVGGraphicsElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGPath() noexcept override;

    void DrawMarkers(PaintInfo &) noexcept final;
    KRYS_NODISCARD FloatRect AdjustStrokeBoundingBoxForMarkersAndZeroLengthLinecaps(
      RepaintRectCalculation, FloatRect strokeBoundingBox) const noexcept override;

  private:
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGPath";
    }

    void UpdateShapeFromElement() noexcept override;

    void StrokeShape(GraphicsContext &) const noexcept override;
    KRYS_NODISCARD bool
      ShapeDependentStrokeContains(const FloatPoint &,
                                   PointCoordinateSpace = GlobalCoordinateSpace) noexcept override;

    void StyleDidChange(style::Difference, const RenderStyle *) noexcept final;

    KRYS_NODISCARD bool ShouldStrokeZeroLengthSubpath() const noexcept;
    KRYS_NODISCARD Path *ZeroLengthLinecapPath(const FloatPoint &) const noexcept;
    KRYS_NODISCARD FloatRect ZeroLengthSubpathRect(const FloatPoint &, float) const noexcept;
    void UpdateZeroLengthSubpaths() noexcept;
    void StrokeZeroLengthSubpaths(GraphicsContext &) const noexcept;

    KRYS_NODISCARD bool ShouldGenerateMarkerPositions() const noexcept;
    void ProcessMarkerPositions() noexcept;
    KRYS_NODISCARD FloatRect MarkerRect(RepaintRectCalculation, float strokeWidth) const noexcept;

    KRYS_NODISCARD bool IsRenderingDisabled() const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGPath)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGPath();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
