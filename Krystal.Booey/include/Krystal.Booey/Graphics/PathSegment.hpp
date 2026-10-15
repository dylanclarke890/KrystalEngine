#pragma once

#include "Krystal.Booey/Graphics/PathSegmentData.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <span>

namespace krys::boo
{
  class PathSegment
  {
  public:
    using Data = Variant<PathMoveTo,

                         PathLineTo, PathQuadCurveTo, PathBezierCurveTo, PathArcTo,

                         PathArc, PathClosedArc, PathEllipse, PathEllipseInRect, PathRect, PathRoundedRect,
                         PathContinuousRoundedRect,

                         PathDataLine, PathDataQuadCurve, PathDataBezierCurve, PathDataArc,

                         PathCloseSubpath>;

    PathSegment(Data &&) noexcept;

    KRYS_NODISCARD bool operator==(const PathSegment &) const noexcept = default;

    KRYS_NODISCARD const Data &GetData() const & noexcept
    {
      return _data;
    }
    KRYS_NODISCARD Data &&GetData() && noexcept
    {
      return krys::move(_data);
    }
    KRYS_NODISCARD bool ClosesSubpath() const noexcept
    {
      return std::holds_alternative<PathCloseSubpath>(_data) || std::holds_alternative<PathClosedArc>(_data);
    }

    KRYS_NODISCARD FloatPoint CalculateEndPoint(const FloatPoint &currentPoint,
                                                FloatPoint &lastMoveToPoint) const noexcept;
    KRYS_NODISCARD Maybe<FloatPoint> TryGetEndPointWithoutContext() const noexcept;

    KRYS_NODISCARD FloatRect FastBoundingRect() const noexcept;
    void ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                FloatRect &boundingRect) const noexcept;
    void ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                            FloatRect &boundingRect) const noexcept;

    KRYS_NODISCARD bool CanApplyElements() const noexcept;
    KRYS_NODISCARD bool ApplyElements(const PathElementApplier &) const noexcept;

    KRYS_NODISCARD bool CanTransform() const noexcept;
    KRYS_NODISCARD bool Transform(const AffineTransform &) noexcept;

  private:
    Data _data;
  };

  using PathSegmentApplier = Func<void(const PathSegment &)>;

  inline PathSegment::PathSegment(Data &&data) noexcept : _data(krys::move(data))
  {
  }
}
