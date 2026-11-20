/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/Paths/PathElement.hpp"
#include "Krystal.Booey/Graphics/Paths/RotationDirection.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"

namespace krys
{
  class AffineTransform;
  class TextStream;
}

namespace krys::boo
{
  struct PathMoveTo
  {
    PointF Point;

    constexpr static bool CanApplyElements = true;
    constexpr static bool CanTransform = true;

    KRYS_NODISCARD bool operator==(const PathMoveTo &) const noexcept = default;

    KRYS_NODISCARD PointF CalculateEndPoint(const PointF &currentPoint,
                                            PointF &lastMoveToPoint) const noexcept;

    KRYS_NODISCARD Maybe<PointF> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                                RectF &boundingRect) const noexcept;

    void ExtendBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                            RectF &boundingRect) const noexcept;

    void ApplyElements(const PathElementApplier &) const noexcept;

    void Transform(const AffineTransform &) noexcept;
  };

  struct PathLineTo
  {
    PointF Point;

    constexpr static bool CanApplyElements = true;
    constexpr static bool CanTransform = true;

    KRYS_NODISCARD bool operator==(const PathLineTo &) const noexcept = default;

    KRYS_NODISCARD PointF CalculateEndPoint(const PointF &currentPoint,
                                            PointF &lastMoveToPoint) const noexcept;

    KRYS_NODISCARD Maybe<PointF> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                                RectF &boundingRect) const noexcept;

    void ExtendBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                            RectF &boundingRect) const noexcept;

    void ApplyElements(const PathElementApplier &) const noexcept;

    void Transform(const AffineTransform &) noexcept;
  };

  struct PathQuadCurveTo
  {
    PointF ControlPoint;
    PointF EndPoint;

    constexpr static bool CanApplyElements = true;
    constexpr static bool CanTransform = true;

    KRYS_NODISCARD bool operator==(const PathQuadCurveTo &) const noexcept = default;

    KRYS_NODISCARD PointF CalculateEndPoint(const PointF &currentPoint,
                                            PointF &lastMoveToPoint) const noexcept;

    KRYS_NODISCARD Maybe<PointF> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                                RectF &boundingRect) const noexcept;

    void ExtendBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                            RectF &boundingRect) const noexcept;

    void ApplyElements(const PathElementApplier &) const noexcept;

    void Transform(const AffineTransform &) noexcept;
  };

  struct PathBezierCurveTo
  {
    PointF ControlPoint1;
    PointF ControlPoint2;
    PointF EndPoint;

    constexpr static bool CanApplyElements = true;
    constexpr static bool CanTransform = true;

    KRYS_NODISCARD bool operator==(const PathBezierCurveTo &) const noexcept = default;

    KRYS_NODISCARD PointF CalculateEndPoint(const PointF &currentPoint,
                                            PointF &lastMoveToPoint) const noexcept;

    KRYS_NODISCARD Maybe<PointF> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                                RectF &boundingRect) const noexcept;

    void ExtendBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                            RectF &boundingRect) const noexcept;

    void ApplyElements(const PathElementApplier &) const noexcept;

    void Transform(const AffineTransform &) noexcept;
  };

  struct PathArcTo
  {
    PointF ControlPoint1;
    PointF ControlPoint2;
    float Radius;

    constexpr static bool CanApplyElements = false;
    constexpr static bool CanTransform = false;

    KRYS_NODISCARD bool operator==(const PathArcTo &) const noexcept = default;

    KRYS_NODISCARD PointF CalculateEndPoint(const PointF &currentPoint,
                                            PointF &lastMoveToPoint) const noexcept;

    KRYS_NODISCARD static Maybe<PointF> TryGetEndPointWithoutContext() noexcept;

    void ExtendFastBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                                RectF &boundingRect) const noexcept;

    void ExtendBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                            RectF &boundingRect) const noexcept;

    /// @brief Applies the segment as elements, its curves as cubic béziers, after the path's @p currentPoint
    /// (none at the start of a path), as a platform path adds it.
    void ApplyFlattenedElements(const Maybe<PointF> &currentPoint,
                                const PathElementApplier &applier) const noexcept;
  };

  struct PathArc
  {
    PointF Center;
    float Radius;
    float StartAngle;
    float EndAngle;
    RotationDirection Direction;

    constexpr static bool CanApplyElements = false;
    constexpr static bool CanTransform = false;

    KRYS_NODISCARD bool operator==(const PathArc &) const noexcept = default;

    KRYS_NODISCARD PointF CalculateEndPoint(const PointF &currentPoint,
                                            PointF &lastMoveToPoint) const noexcept;

    KRYS_NODISCARD Maybe<PointF> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                                RectF &boundingRect) const noexcept;

    void ExtendBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                            RectF &boundingRect) const noexcept;

    /// @brief Applies the segment as elements, its curves as cubic béziers, after the path's @p currentPoint
    /// (none at the start of a path), as a platform path adds it.
    void ApplyFlattenedElements(const Maybe<PointF> &currentPoint,
                                const PathElementApplier &applier) const noexcept;
  };

  struct PathClosedArc
  {
    PathArc Arc;

    constexpr static bool CanApplyElements = false;
    constexpr static bool CanTransform = false;

    KRYS_NODISCARD bool operator==(const PathClosedArc &) const noexcept = default;

    KRYS_NODISCARD PointF CalculateEndPoint(const PointF &currentPoint,
                                            PointF &lastMoveToPoint) const noexcept;

    KRYS_NODISCARD Maybe<PointF> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                                RectF &boundingRect) const noexcept;

    void ExtendBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                            RectF &boundingRect) const noexcept;

    /// @brief Applies the segment as elements, its curves as cubic béziers, after the path's @p currentPoint
    /// (none at the start of a path), as a platform path adds it.
    void ApplyFlattenedElements(const Maybe<PointF> &currentPoint,
                                const PathElementApplier &applier) const noexcept;
  };

  struct PathEllipse
  {
    PointF Center;
    float RadiusX;
    float RadiusY;
    float Rotation;
    float StartAngle;
    float EndAngle;
    RotationDirection Direction;

    constexpr static bool CanApplyElements = false;
    constexpr static bool CanTransform = false;

    KRYS_NODISCARD bool operator==(const PathEllipse &) const noexcept = default;

    KRYS_NODISCARD PointF CalculateEndPoint(const PointF &currentPoint,
                                            PointF &lastMoveToPoint) const noexcept;

    KRYS_NODISCARD Maybe<PointF> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                                RectF &boundingRect) const noexcept;

    static void ExtendBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                                   RectF &boundingRect) noexcept;

    /// @brief Applies the segment as elements, its curves as cubic béziers, after the path's @p currentPoint
    /// (none at the start of a path), as a platform path adds it.
    void ApplyFlattenedElements(const Maybe<PointF> &currentPoint,
                                const PathElementApplier &applier) const noexcept;
  };

  struct PathEllipseInRect
  {
    RectF Rect;

    constexpr static bool CanApplyElements = false;
    constexpr static bool CanTransform = false;

    KRYS_NODISCARD bool operator==(const PathEllipseInRect &) const noexcept = default;

    KRYS_NODISCARD PointF CalculateEndPoint(const PointF &currentPoint,
                                            PointF &lastMoveToPoint) const noexcept;

    KRYS_NODISCARD Maybe<PointF> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                                RectF &boundingRect) const noexcept;

    void ExtendBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                            RectF &boundingRect) const noexcept;
  };

  struct PathRect
  {
    RectF Rect;

    constexpr static bool CanApplyElements = false;
    constexpr static bool CanTransform = false;

    KRYS_NODISCARD bool operator==(const PathRect &) const noexcept = default;

    KRYS_NODISCARD PointF CalculateEndPoint(const PointF &currentPoint,
                                            PointF &lastMoveToPoint) const noexcept;

    KRYS_NODISCARD Maybe<PointF> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                                RectF &boundingRect) const noexcept;

    void ExtendBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                            RectF &boundingRect) const noexcept;
  };

  struct PathRoundedRect
  {
    enum class Strategy : uint8
    {
      PreferNative,
      PreferBezier,
    };

    FloatRoundedRect RoundedRect;
    Strategy RoundingStrategy;

    constexpr static bool CanApplyElements = false;
    constexpr static bool CanTransform = false;

    KRYS_NODISCARD bool operator==(const PathRoundedRect &) const noexcept = default;

    KRYS_NODISCARD PointF CalculateEndPoint(const PointF &currentPoint,
                                            PointF &lastMoveToPoint) const noexcept;

    KRYS_NODISCARD Maybe<PointF> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                                RectF &boundingRect) const noexcept;

    void ExtendBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                            RectF &boundingRect) const noexcept;
  };

  struct PathContinuousRoundedRect
  {
    RectF Rect;
    float CornerWidth;
    float CornerHeight;

    constexpr static bool CanApplyElements = false;
    constexpr static bool CanTransform = false;

    KRYS_NODISCARD bool operator==(const PathContinuousRoundedRect &) const noexcept = default;

    KRYS_NODISCARD PointF CalculateEndPoint(const PointF &currentPoint,
                                            PointF &lastMoveToPoint) const noexcept;

    KRYS_NODISCARD Maybe<PointF> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                                RectF &boundingRect) const noexcept;

    void ExtendBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                            RectF &boundingRect) const noexcept;
  };

  struct PathDataLine
  {
    PathDataLine(PointF start, PointF end) noexcept : _values {{start.X(), start.Y(), end.X(), end.Y()}}
    {
    }

    PathDataLine(std::span<const float, 4> values) noexcept
        : _values {values[0], values[1], values[2], values[3]}
    {
    }

    KRYS_NODISCARD PointF Start() const noexcept
    {
      return {_values[0], _values[1]};
    }

    void SetStart(PointF p) noexcept
    {
      _values[0] = p.X();
      _values[1] = p.Y();
    }

    KRYS_NODISCARD PointF End() const noexcept
    {
      return {_values[2], _values[3]};
    }

    void SetEnd(PointF p) noexcept
    {
      _values[2] = p.X();
      _values[3] = p.Y();
    }

    std::span<const float, 4> span() const KRYS_LIFETIME_BOUND
    {
      return _values;
    }

    constexpr static bool CanApplyElements = true;
    constexpr static bool CanTransform = true;

    KRYS_NODISCARD bool operator==(const PathDataLine &) const noexcept = default;

    KRYS_NODISCARD PointF CalculateEndPoint(const PointF &currentPoint,
                                            PointF &lastMoveToPoint) const noexcept;

    KRYS_NODISCARD Maybe<PointF> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                                RectF &boundingRect) const noexcept;

    void ExtendBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                            RectF &boundingRect) const noexcept;

    void ApplyElements(const PathElementApplier &) const noexcept;

    void Transform(const AffineTransform &) noexcept;

  private:
    std::array<float, 4> _values {};
  };

  struct PathDataQuadCurve
  {
    PointF Start;
    PointF ControlPoint;
    PointF EndPoint;

    constexpr static bool CanApplyElements = true;
    constexpr static bool CanTransform = true;

    KRYS_NODISCARD bool operator==(const PathDataQuadCurve &) const noexcept = default;

    KRYS_NODISCARD PointF CalculateEndPoint(const PointF &currentPoint,
                                            PointF &lastMoveToPoint) const noexcept;

    KRYS_NODISCARD Maybe<PointF> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                                RectF &boundingRect) const noexcept;

    void ExtendBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                            RectF &boundingRect) const noexcept;

    void ApplyElements(const PathElementApplier &) const noexcept;

    void Transform(const AffineTransform &) noexcept;
  };

  struct PathDataBezierCurve
  {
    PointF Start;
    PointF ControlPoint1;
    PointF ControlPoint2;
    PointF EndPoint;

    constexpr static bool CanApplyElements = true;
    constexpr static bool CanTransform = true;

    KRYS_NODISCARD bool operator==(const PathDataBezierCurve &) const noexcept = default;

    KRYS_NODISCARD PointF CalculateEndPoint(const PointF &currentPoint,
                                            PointF &lastMoveToPoint) const noexcept;

    KRYS_NODISCARD Maybe<PointF> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                                RectF &boundingRect) const noexcept;

    void ExtendBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                            RectF &boundingRect) const noexcept;

    void ApplyElements(const PathElementApplier &) const noexcept;

    void Transform(const AffineTransform &) noexcept;
  };

  struct PathDataArc
  {
    PointF Start;
    PointF ControlPoint1;
    PointF ControlPoint2;
    float Radius;

    constexpr static bool CanApplyElements = false;
    constexpr static bool CanTransform = false;

    KRYS_NODISCARD bool operator==(const PathDataArc &) const noexcept = default;

    KRYS_NODISCARD PointF CalculateEndPoint(const PointF &currentPoint,
                                            PointF &lastMoveToPoint) const noexcept;

    KRYS_NODISCARD Maybe<PointF> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                                RectF &boundingRect) const noexcept;

    void ExtendBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                            RectF &boundingRect) const noexcept;

    /// @brief Applies the segment as elements, its curves as cubic béziers, after the path's @p currentPoint
    /// (none at the start of a path), as a platform path adds it.
    void ApplyFlattenedElements(const Maybe<PointF> &currentPoint,
                                const PathElementApplier &applier) const noexcept;
  };

  struct PathCloseSubpath
  {
    constexpr static bool CanApplyElements = true;
    constexpr static bool CanTransform = true;

    KRYS_NODISCARD bool operator==(const PathCloseSubpath &) const noexcept = default;

    KRYS_NODISCARD static PointF CalculateEndPoint(const PointF &currentPoint,
                                                   PointF &lastMoveToPoint) noexcept;

    KRYS_NODISCARD static Maybe<PointF> TryGetEndPointWithoutContext() noexcept;

    static void ExtendFastBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                                       RectF &boundingRect) noexcept;

    static void ExtendBoundingRect(const PointF &currentPoint, const PointF &lastMoveToPoint,
                                   RectF &boundingRect) noexcept;

    static void ApplyElements(const PathElementApplier &) noexcept;

    void Transform(const AffineTransform &) noexcept;
  };
}

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, const PathMoveTo &data) noexcept;

  TextStream &operator<<(TextStream &ts, const PathLineTo &data) noexcept;

  TextStream &operator<<(TextStream &ts, const PathQuadCurveTo &data) noexcept;

  TextStream &operator<<(TextStream &ts, const PathBezierCurveTo &data) noexcept;

  TextStream &operator<<(TextStream &ts, const PathArcTo &data) noexcept;

  TextStream &operator<<(TextStream &ts, const PathArc &data) noexcept;

  TextStream &operator<<(TextStream &ts, const PathClosedArc &data) noexcept;

  TextStream &operator<<(TextStream &ts, const PathEllipse &data) noexcept;

  TextStream &operator<<(TextStream &ts, const PathEllipseInRect &data) noexcept;

  TextStream &operator<<(TextStream &ts, const PathRect &data) noexcept;

  TextStream &operator<<(TextStream &ts, const PathRoundedRect &data) noexcept;

  TextStream &operator<<(TextStream &ts, const PathContinuousRoundedRect &data) noexcept;

  TextStream &operator<<(TextStream &ts, const PathDataLine &data) noexcept;

  TextStream &operator<<(TextStream &ts, const PathDataQuadCurve &data) noexcept;

  TextStream &operator<<(TextStream &ts, const PathDataBezierCurve &data) noexcept;

  TextStream &operator<<(TextStream &ts, const PathDataArc &data) noexcept;

  TextStream &operator<<(TextStream &ts, const PathCloseSubpath &data) noexcept;
}
