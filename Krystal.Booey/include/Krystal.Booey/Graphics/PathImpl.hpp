#pragma once

#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/PathElement.hpp"
#include "Krystal.Booey/Graphics/PathSegment.hpp"

#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class PathImpl : public ThreadSafeRefCounted<PathImpl>
  {
  public:
    virtual ~PathImpl() noexcept = default;

    KRYS_NODISCARD constexpr static float CircleControlPoint() noexcept
    {
      // Approximation of control point positions on a bezier to simulate a quarter of a circle.
      // This is 1-kappa, where kappa = 4 * (sqrt(2) - 1) / 3
      return 0.447715f;
    }

    KRYS_NODISCARD virtual bool IsPathStream() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool DefinitelyEqual(const PathImpl &) const noexcept = 0;

    KRYS_NODISCARD virtual Ref<PathImpl> Copy() const noexcept = 0;

    void AddSegment(PathSegment) noexcept;
    virtual void Add(PathMoveTo) noexcept = 0;
    virtual void Add(PathLineTo) noexcept = 0;
    virtual void Add(PathQuadCurveTo) noexcept = 0;
    virtual void Add(PathBezierCurveTo) noexcept = 0;
    virtual void Add(PathArcTo) noexcept = 0;
    virtual void Add(PathArc) noexcept = 0;
    virtual void Add(PathClosedArc) noexcept = 0;
    virtual void Add(PathEllipse) noexcept = 0;
    virtual void Add(PathEllipseInRect) noexcept = 0;
    virtual void Add(PathRect) noexcept = 0;
    virtual void Add(PathRoundedRect) noexcept = 0;
    virtual void Add(PathContinuousRoundedRect) noexcept = 0;
    virtual void Add(PathCloseSubpath) noexcept = 0;

    void AddLinesForRect(const FloatRect &) noexcept;
    KRYS_NODISCARD static SmallList<PathSegment, 10> BeziersForRoundedRect(const FloatRoundedRect &) noexcept;

    virtual void ApplySegments(const PathSegmentApplier &) const noexcept;
    KRYS_NODISCARD virtual bool ApplyElements(const PathElementApplier &) const noexcept = 0;

    KRYS_NODISCARD virtual bool Transform(const AffineTransform &) noexcept = 0;

    KRYS_NODISCARD virtual Maybe<PathSegment> SingleSegment() const noexcept
    {
      return std::nullopt;
    }

    KRYS_NODISCARD virtual bool IsClosed() const noexcept;

    KRYS_NODISCARD virtual bool HasSubpaths() const noexcept;

    KRYS_NODISCARD virtual FloatPoint CurrentPoint() const noexcept = 0;

    KRYS_NODISCARD virtual FloatRect FastBoundingRect() const noexcept = 0;
    KRYS_NODISCARD virtual FloatRect BoundingRect() const noexcept = 0;

  protected:
    PathImpl() noexcept = default;
  };

  inline void PathImpl::AddSegment(PathSegment segment) noexcept
  {
    krys::SwitchOn(
      krys::move(segment).GetData(), [&](auto &&segment) { Add(krys::move(segment)); },
      [&](PathDataLine segment)
      {
        Add(PathMoveTo {segment.Start()});
        Add(PathLineTo {segment.End()});
      },
      [&](PathDataQuadCurve segment)
      {
        Add(PathMoveTo {segment.Start});
        Add(PathQuadCurveTo {segment.ControlPoint, segment.EndPoint});
      },
      [&](PathDataBezierCurve segment)
      {
        Add(PathMoveTo {segment.Start});
        Add(PathBezierCurveTo {segment.ControlPoint1, segment.ControlPoint2, segment.EndPoint});
      },
      [&](PathDataArc segment)
      {
        Add(PathMoveTo {segment.Start});
        Add(PathArcTo {segment.ControlPoint1, segment.ControlPoint2, segment.Radius});
      });
  }
}
