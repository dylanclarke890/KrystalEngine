#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/PathElement.hpp"
#include "Krystal.Booey/Graphics/RotationDirection.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  struct PathMoveTo
  {
    FloatPoint Point;

    static constexpr bool CanApplyElements = true;
    static constexpr bool CanTransform = true;

    KRYS_NODISCARD bool operator==(const PathMoveTo &) const noexcept = default;

    KRYS_NODISCARD FloatPoint CalculateEndPoint(const FloatPoint &currentPoint,
                                                FloatPoint &lastMoveToPoint) const noexcept;
    KRYS_NODISCARD Maybe<FloatPoint> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                FloatRect &boundingRect) const noexcept;
    void ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                            FloatRect &boundingRect) const noexcept;

    void ApplyElements(const PathElementApplier &) const noexcept;

    void Transform(const AffineTransform &) noexcept;
  };

  struct PathLineTo
  {
    FloatPoint Point;

    static constexpr bool CanApplyElements = true;
    static constexpr bool CanTransform = true;

    KRYS_NODISCARD bool operator==(const PathLineTo &) const noexcept = default;

    KRYS_NODISCARD FloatPoint CalculateEndPoint(const FloatPoint &currentPoint,
                                                FloatPoint &lastMoveToPoint) const noexcept;
    KRYS_NODISCARD Maybe<FloatPoint> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                FloatRect &boundingRect) const noexcept;
    void ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                            FloatRect &boundingRect) const noexcept;

    void ApplyElements(const PathElementApplier &) const noexcept;

    void Transform(const AffineTransform &) noexcept;
  };

  struct PathQuadCurveTo
  {
    FloatPoint ControlPoint;
    FloatPoint EndPoint;

    static constexpr bool CanApplyElements = true;
    static constexpr bool CanTransform = true;

    KRYS_NODISCARD bool operator==(const PathQuadCurveTo &) const noexcept = default;

    KRYS_NODISCARD FloatPoint CalculateEndPoint(const FloatPoint &currentPoint,
                                                FloatPoint &lastMoveToPoint) const noexcept;
    KRYS_NODISCARD Maybe<FloatPoint> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                FloatRect &boundingRect) const noexcept;
    void ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                            FloatRect &boundingRect) const noexcept;

    void ApplyElements(const PathElementApplier &) const noexcept;

    void Transform(const AffineTransform &) noexcept;
  };

  struct PathBezierCurveTo
  {
    FloatPoint ControlPoint1;
    FloatPoint ControlPoint2;
    FloatPoint EndPoint;

    static constexpr bool CanApplyElements = true;
    static constexpr bool CanTransform = true;

    KRYS_NODISCARD bool operator==(const PathBezierCurveTo &) const noexcept = default;

    KRYS_NODISCARD FloatPoint CalculateEndPoint(const FloatPoint &currentPoint,
                                                FloatPoint &lastMoveToPoint) const noexcept;
    KRYS_NODISCARD Maybe<FloatPoint> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                FloatRect &boundingRect) const noexcept;
    void ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                            FloatRect &boundingRect) const noexcept;

    void ApplyElements(const PathElementApplier &) const noexcept;

    void Transform(const AffineTransform &) noexcept;
  };

  struct PathArcTo
  {
    FloatPoint ControlPoint1;
    FloatPoint ControlPoint2;
    float Radius;

    static constexpr bool CanApplyElements = false;
    static constexpr bool CanTransform = false;

    KRYS_NODISCARD bool operator==(const PathArcTo &) const noexcept = default;

    KRYS_NODISCARD FloatPoint CalculateEndPoint(const FloatPoint &currentPoint,
                                                FloatPoint &lastMoveToPoint) const noexcept;
    KRYS_NODISCARD Maybe<FloatPoint> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                FloatRect &boundingRect) const noexcept;
    void ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                            FloatRect &boundingRect) const noexcept;
  };

  struct PathArc
  {
    FloatPoint Center;
    float Radius;
    float StartAngle;
    float EndAngle;
    RotationDirection Direction;

    static constexpr bool CanApplyElements = false;
    static constexpr bool CanTransform = false;

    KRYS_NODISCARD bool operator==(const PathArc &) const noexcept = default;

    KRYS_NODISCARD FloatPoint CalculateEndPoint(const FloatPoint &currentPoint,
                                                FloatPoint &lastMoveToPoint) const noexcept;
    KRYS_NODISCARD Maybe<FloatPoint> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                FloatRect &boundingRect) const noexcept;
    void ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                            FloatRect &boundingRect) const noexcept;
  };

  struct PathClosedArc
  {
    PathArc Arc;

    static constexpr bool CanApplyElements = false;
    static constexpr bool CanTransform = false;

    KRYS_NODISCARD bool operator==(const PathClosedArc &) const noexcept = default;

    KRYS_NODISCARD FloatPoint CalculateEndPoint(const FloatPoint &currentPoint,
                                                FloatPoint &lastMoveToPoint) const noexcept;
    KRYS_NODISCARD Maybe<FloatPoint> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                FloatRect &boundingRect) const noexcept;
    void ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                            FloatRect &boundingRect) const noexcept;
  };

  struct PathEllipse
  {
    FloatPoint Center;
    float RadiusX;
    float RadiusY;
    float Rotation;
    float StartAngle;
    float EndAngle;
    RotationDirection Direction;

    static constexpr bool CanApplyElements = false;
    static constexpr bool CanTransform = false;

    KRYS_NODISCARD bool operator==(const PathEllipse &) const noexcept = default;

    KRYS_NODISCARD FloatPoint CalculateEndPoint(const FloatPoint &currentPoint,
                                                FloatPoint &lastMoveToPoint) const noexcept;
    KRYS_NODISCARD Maybe<FloatPoint> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                FloatRect &boundingRect) const noexcept;
    void ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                            FloatRect &boundingRect) const noexcept;
  };

  struct PathEllipseInRect
  {
    FloatRect Rect;

    static constexpr bool CanApplyElements = false;
    static constexpr bool CanTransform = false;

    KRYS_NODISCARD bool operator==(const PathEllipseInRect &) const noexcept = default;

    KRYS_NODISCARD FloatPoint CalculateEndPoint(const FloatPoint &currentPoint,
                                                FloatPoint &lastMoveToPoint) const noexcept;
    KRYS_NODISCARD Maybe<FloatPoint> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                FloatRect &boundingRect) const noexcept;
    void ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                            FloatRect &boundingRect) const noexcept;
  };

  struct PathRect
  {
    FloatRect Rect;

    static constexpr bool CanApplyElements = false;
    static constexpr bool CanTransform = false;

    KRYS_NODISCARD bool operator==(const PathRect &) const noexcept = default;

    KRYS_NODISCARD FloatPoint CalculateEndPoint(const FloatPoint &currentPoint,
                                                FloatPoint &lastMoveToPoint) const noexcept;
    KRYS_NODISCARD Maybe<FloatPoint> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                FloatRect &boundingRect) const noexcept;
    void ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                            FloatRect &boundingRect) const noexcept;
  };

  struct PathRoundedRect
  {
    enum class Strategy : uint8
    {
      PreferNative,
      PreferBezier
    };

    FloatRoundedRect RoundedRect;
    Strategy RoundingStrategy;

    static constexpr bool CanApplyElements = false;
    static constexpr bool CanTransform = false;

    KRYS_NODISCARD bool operator==(const PathRoundedRect &) const noexcept = default;

    KRYS_NODISCARD FloatPoint CalculateEndPoint(const FloatPoint &currentPoint,
                                                FloatPoint &lastMoveToPoint) const noexcept;
    KRYS_NODISCARD Maybe<FloatPoint> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                FloatRect &boundingRect) const noexcept;
    void ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                            FloatRect &boundingRect) const noexcept;
  };

  struct PathContinuousRoundedRect
  {
    FloatRect Rect;
    float CornerWidth;
    float CornerHeight;

    static constexpr bool CanApplyElements = false;
    static constexpr bool CanTransform = false;

    KRYS_NODISCARD bool operator==(const PathContinuousRoundedRect &) const noexcept = default;

    KRYS_NODISCARD FloatPoint CalculateEndPoint(const FloatPoint &currentPoint,
                                                FloatPoint &lastMoveToPoint) const noexcept;
    KRYS_NODISCARD Maybe<FloatPoint> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                FloatRect &boundingRect) const noexcept;
    void ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                            FloatRect &boundingRect) const noexcept;
  };

  struct PathDataLine
  {
    PathDataLine(FloatPoint start, FloatPoint end) noexcept
        : _values {{start.X(), start.Y(), end.X(), end.Y()}}
    {
    }
    PathDataLine(std::span<const float, 4> values) noexcept
        : _values {values[0], values[1], values[2], values[3]}
    {
    }

    KRYS_NODISCARD FloatPoint Start() const noexcept
    {
      return {_values[0], _values[1]};
    }
    void SetStart(FloatPoint p) noexcept
    {
      _values[0] = p.X();
      _values[1] = p.Y();
    }
    KRYS_NODISCARD FloatPoint End() const noexcept
    {
      return {_values[2], _values[3]};
    }
    void SetEnd(FloatPoint p) noexcept
    {
      _values[2] = p.X();
      _values[3] = p.Y();
    }
    std::span<const float, 4> span() const KRYS_LIFETIME_BOUND
    {
      return _values;
    }
    static constexpr bool CanApplyElements = true;
    static constexpr bool CanTransform = true;

    KRYS_NODISCARD bool operator==(const PathDataLine &) const noexcept = default;

    KRYS_NODISCARD FloatPoint CalculateEndPoint(const FloatPoint &currentPoint,
                                                FloatPoint &lastMoveToPoint) const noexcept;
    KRYS_NODISCARD Maybe<FloatPoint> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                FloatRect &boundingRect) const noexcept;
    void ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                            FloatRect &boundingRect) const noexcept;

    void ApplyElements(const PathElementApplier &) const noexcept;

    void Transform(const AffineTransform &) noexcept;

  private:
    std::array<float, 4> _values {};
  };

  struct PathDataQuadCurve
  {
    FloatPoint Start;
    FloatPoint ControlPoint;
    FloatPoint EndPoint;

    static constexpr bool CanApplyElements = true;
    static constexpr bool CanTransform = true;

    KRYS_NODISCARD bool operator==(const PathDataQuadCurve &) const noexcept = default;

    KRYS_NODISCARD FloatPoint CalculateEndPoint(const FloatPoint &currentPoint,
                                                FloatPoint &lastMoveToPoint) const noexcept;
    KRYS_NODISCARD Maybe<FloatPoint> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                FloatRect &boundingRect) const noexcept;
    void ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                            FloatRect &boundingRect) const noexcept;

    void ApplyElements(const PathElementApplier &) const noexcept;

    void Transform(const AffineTransform &) noexcept;
  };

  struct PathDataBezierCurve
  {
    FloatPoint Start;
    FloatPoint ControlPoint1;
    FloatPoint ControlPoint2;
    FloatPoint EndPoint;

    static constexpr bool CanApplyElements = true;
    static constexpr bool CanTransform = true;

    KRYS_NODISCARD bool operator==(const PathDataBezierCurve &) const noexcept = default;

    KRYS_NODISCARD FloatPoint CalculateEndPoint(const FloatPoint &currentPoint,
                                                FloatPoint &lastMoveToPoint) const noexcept;
    KRYS_NODISCARD Maybe<FloatPoint> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                FloatRect &boundingRect) const noexcept;
    void ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                            FloatRect &boundingRect) const noexcept;

    void ApplyElements(const PathElementApplier &) const noexcept;

    void Transform(const AffineTransform &) noexcept;
  };

  struct PathDataArc
  {
    FloatPoint Start;
    FloatPoint ControlPoint1;
    FloatPoint ControlPoint2;
    float Radius;

    static constexpr bool CanApplyElements = false;
    static constexpr bool CanTransform = false;

    KRYS_NODISCARD bool operator==(const PathDataArc &) const noexcept = default;

    KRYS_NODISCARD FloatPoint CalculateEndPoint(const FloatPoint &currentPoint,
                                                FloatPoint &lastMoveToPoint) const noexcept;
    KRYS_NODISCARD Maybe<FloatPoint> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                FloatRect &boundingRect) const noexcept;
    void ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                            FloatRect &boundingRect) const noexcept;
  };

  struct PathCloseSubpath
  {
    static constexpr bool CanApplyElements = true;
    static constexpr bool CanTransform = true;

    KRYS_NODISCARD bool operator==(const PathCloseSubpath &) const noexcept = default;

    KRYS_NODISCARD FloatPoint CalculateEndPoint(const FloatPoint &currentPoint,
                                                FloatPoint &lastMoveToPoint) const noexcept;
    KRYS_NODISCARD Maybe<FloatPoint> TryGetEndPointWithoutContext() const noexcept;

    void ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                FloatRect &boundingRect) const noexcept;
    void ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                            FloatRect &boundingRect) const noexcept;

    void ApplyElements(const PathElementApplier &) const noexcept;

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
