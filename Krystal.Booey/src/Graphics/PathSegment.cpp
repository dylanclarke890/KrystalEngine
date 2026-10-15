#include "Krystal.Booey/Graphics/PathSegment.hpp"

#include "Krystal.Booey/Graphics/PathElement.hpp"
#include "Krystal.Booey/Graphics/PathImpl.hpp"

namespace krys::boo
{
  FloatPoint PathSegment::CalculateEndPoint(const FloatPoint &currentPoint,
                                            FloatPoint &lastMoveToPoint) const noexcept
  {
    return krys::SwitchOn(_data,
                          [&](auto &data) { return data.CalculateEndPoint(currentPoint, lastMoveToPoint); });
  }

  Maybe<FloatPoint> PathSegment::TryGetEndPointWithoutContext() const noexcept
  {
    return krys::SwitchOn(_data, [&](auto &data) { return data.TryGetEndPointWithoutContext(); });
  }

  FloatRect PathSegment::FastBoundingRect() const noexcept
  {
    FloatPoint currentPoint;
    FloatPoint lastMoveToPoint;

    auto boundingRect = FloatRect::SmallestRect();
    ExtendFastBoundingRect(currentPoint, lastMoveToPoint, boundingRect);

    if (boundingRect.IsSmallest())
    {
      currentPoint = CalculateEndPoint(currentPoint, lastMoveToPoint);
      boundingRect.Extend(currentPoint);
    }

    return boundingRect;
  }

  void PathSegment::ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                           FloatRect &boundingRect) const noexcept
  {
    krys::SwitchOn(_data, [&](auto &data)
                   { data.ExtendFastBoundingRect(currentPoint, lastMoveToPoint, boundingRect); });
  }

  void PathSegment::ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                       FloatRect &boundingRect) const noexcept
  {
    krys::SwitchOn(_data,
                   [&](auto &data) { data.ExtendBoundingRect(currentPoint, lastMoveToPoint, boundingRect); });
  }

  bool PathSegment::CanApplyElements() const noexcept
  {
    return krys::SwitchOn(_data, [&](auto &data) { return data.CanApplyElements; });
  }

  bool PathSegment::ApplyElements(const PathElementApplier &applier) const noexcept
  {
    return krys::SwitchOn(_data,
                          [&]<typename DataType>(DataType &data) -> bool
                          {
                            if constexpr (DataType::CanApplyElements)
                            {
                              data.ApplyElements(applier);
                              return true;
                            }
                            else
                            {
                              return false;
                            }
                          });
  }

  bool PathSegment::CanTransform() const noexcept
  {
    return krys::SwitchOn(_data, [&](auto &data) { return data.CanTransform; });
  }

  bool PathSegment::Transform(const AffineTransform &transform) noexcept
  {
    return krys::SwitchOn(_data,
                          [&]<typename DataType>(DataType &data)
                          {
                            if constexpr (DataType::CanTransform)
                            {
                              data.Transform(transform);
                              return true;
                            }
                            else
                            {
                              return false;
                            }
                          });
  }
}
