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

#include "Krystal.Booey/Graphics/Paths/Path.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGModelObject.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Core/Geometry/AffineTransform.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Types/Markable.hpp"

namespace krys::boo
{
  class GraphicsContextStateSaver;
  class SVGGraphicsElement;

  enum class LegacyRenderSVGShapeType : uint8
  {
    Empty,
    Path,
    Line,
    Rectangle,
    RoundedRectangle,
    Ellipse,
    Circle,
  };

  /// @brief The renderer of an SVG shape: it fills, strokes and hit tests the element's path.
  class LegacyRenderSVGShape : public LegacyRenderSVGModelObject
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGShape);

    friend RectF SVGRenderSupport::CalculateApproximateStrokeBoundingBox(const RenderElement &) noexcept;

  public:
    using ShapeType = LegacyRenderSVGShapeType;

    enum PointCoordinateSpace
    {
      GlobalCoordinateSpace,
      LocalCoordinateSpace,
    };

  protected:
    RectF _fillBoundingBox;
    mutable Markable<RectF> _strokeBoundingBox;
    mutable Markable<RectF> _approximateStrokeBoundingBox;

  private:
    RectF _repaintBoundingBox;

    bool _needsBoundariesUpdate {false}; // Default is false, the cached rects are empty from the beginning.
    bool _needsShapeUpdate {true}; // Default is true, so we grab a Path object once from SVGGraphicsElement.
    bool _needsTransformUpdate {
      true}; // Default is true, so we grab a AffineTransform object once from SVGGraphicsElement.
    bool _fillRequiresClip {true};

  protected:
    ShapeType _shapeType {ShapeType::Empty};

  private:
    AffineTransform _localTransform;
    UniquePtr<Path> _path;

  public:
    ~LegacyRenderSVGShape() noexcept override;

    KRYS_NODISCARD SVGGraphicsElement &GraphicsElement() const noexcept;

    void SetNeedsShapeUpdate() noexcept
    {
      _needsShapeUpdate = true;
    }

    void SetNeedsBoundariesUpdate() noexcept final
    {
      _needsBoundariesUpdate = true;
    }

    void SetNeedsTransformUpdate() noexcept final
    {
      _needsTransformUpdate = true;
    }

    virtual void FillShape(GraphicsContext &) const noexcept;

    virtual void StrokeShape(GraphicsContext &) const noexcept;

    KRYS_NODISCARD virtual bool IsRenderingDisabled() const noexcept = 0;

    KRYS_NODISCARD bool IsPointInFill(const PointF &) noexcept;

    KRYS_NODISCARD bool IsPointInStroke(const PointF &) noexcept;

    KRYS_NODISCARD float GetTotalLength() const noexcept;

    KRYS_NODISCARD PointF GetPointAtLength(float distance) const noexcept;

    KRYS_NODISCARD bool HasPath() const noexcept
    {
      return static_cast<bool>(_path);
    }

    KRYS_NODISCARD Path &GetPath() const noexcept
    {
      krys_debug_assert(_path);
      return *_path;
    }

    void ClearPath() noexcept
    {
      _path = nullptr;
    }

    KRYS_NODISCARD ShapeType GetShapeType() const noexcept
    {
      return _shapeType;
    }

  protected:
    LegacyRenderSVGShape(Type, SVGGraphicsElement &, RenderStyle &&) noexcept;

    Path &EnsurePath() noexcept;

    virtual void UpdateShapeFromElement() noexcept = 0;

    KRYS_NODISCARD virtual bool IsEmpty() const noexcept;

    KRYS_NODISCARD virtual bool
      ShapeDependentStrokeContains(const PointF &, PointCoordinateSpace = GlobalCoordinateSpace) noexcept;

    KRYS_NODISCARD virtual bool ShapeDependentFillContains(const PointF &, const WindRule) const noexcept;

    KRYS_NODISCARD float StrokeWidth() const noexcept;

    KRYS_NODISCARD bool HasNonScalingStroke() const noexcept;

    KRYS_NODISCARD AffineTransform NonScalingStrokeTransform() const noexcept;

    KRYS_NODISCARD static Path *NonScalingStrokePath(const Path *, const AffineTransform &) noexcept;

    KRYS_NODISCARD virtual RectF
      AdjustStrokeBoundingBoxForMarkersAndZeroLengthLinecaps(RepaintRectCalculation,
                                                             RectF strokeBoundingBox) const noexcept
    {
      return strokeBoundingBox;
    }

    KRYS_NODISCARD RectF StrokeBoundingBox() const noexcept final;

    KRYS_NODISCARD RectF ApproximateStrokeBoundingBox() const noexcept;

    KRYS_NODISCARD bool FillRequiresClip() const noexcept
    {
      return _fillRequiresClip;
    }

  private:
    // Hit-detection separated for the fill and the stroke
    KRYS_NODISCARD bool FillContains(const PointF &, bool requiresFill = true,
                                     const WindRule fillRule = WindRule::NonZero) noexcept;

    KRYS_NODISCARD bool StrokeContains(const PointF &, bool requiresStroke = true) noexcept;

    KRYS_NODISCARD RectF RepaintRectInLocalCoordinates(
      RepaintRectCalculation = RepaintRectCalculation::Fast) const noexcept final;

    KRYS_NODISCARD RectF DecoratedBoundingBox() const noexcept final;

    KRYS_NODISCARD const AffineTransform &LocalToParentTransform() const noexcept final
    {
      return _localTransform;
    }

    KRYS_NODISCARD AffineTransform LocalTransform() const noexcept final
    {
      return _localTransform;
    }

    KRYS_NODISCARD bool CanHaveChildren() const noexcept final
    {
      return false;
    }

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGShape";
    }

    void Layout() noexcept final;

    void Paint(PaintInfo &, const LayoutPoint &) noexcept final;

    void AddFocusRingRects(List<LayoutRect> &, const LayoutPoint &additionalOffset,
                           const RenderLayerModelObject *paintContainer = nullptr) const noexcept final;

    bool NodeAtPointF(const HitTestRequest &, HitTestResult &, const PointF &pointInParent,
                      HitTestAction) noexcept final;

    KRYS_NODISCARD RectF ObjectBoundingBox() const noexcept final
    {
      return _fillBoundingBox;
    }

    KRYS_NODISCARD RectF CalculateStrokeBoundingBox() const noexcept;

    void UpdateRepaintBoundingBox() noexcept;

    KRYS_NODISCARD static bool SetupNonScalingStrokeContext(AffineTransform &,
                                                            GraphicsContextStateSaver &) noexcept;

    KRYS_NODISCARD RectF CalculateApproximateStrokeBoundingBox() const noexcept;

    KRYS_NODISCARD UniquePtr<Path> CreatePath() const noexcept;

    void FillShape(const RenderStyle &, GraphicsContext &) noexcept;

    void StrokeShapeInternal(const RenderStyle &, GraphicsContext &) noexcept;

    void StrokeShape(const RenderStyle &, GraphicsContext &) noexcept;

    void FillStrokeMarkers(PaintInfo &) noexcept;

    virtual void DrawMarkers(PaintInfo &) noexcept
    {
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGShape)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGShape();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
