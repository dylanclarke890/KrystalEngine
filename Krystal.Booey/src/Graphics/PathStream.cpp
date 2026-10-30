#include "Krystal.Booey/Graphics/PathStream.hpp"

#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"

namespace krys::boo
{
  Ref<PathStream> PathStream::Create() noexcept
  {
    return AdoptRef(*new PathStream);
  }

  Ref<PathStream> PathStream::Create(PathSegment &&segment) noexcept
  {
    return AdoptRef(*new PathStream(krys::move(segment)));
  }

  Ref<PathStream> PathStream::Create(List<PathSegment> &&segments) noexcept
  {
    return AdoptRef(*new PathStream(krys::move(segments)));
  }

  Ref<PathStream> PathStream::Create(const List<PathSegment> &segments) noexcept
  {
    return AdoptRef(*new PathStream(segments));
  }

  Ref<PathStream> PathStream::Create(const List<FloatPoint> &points) noexcept
  {
    auto stream = PathStream::Create();
    if (points.size() < 2)
    {
      return stream;
    }

    stream->Add(PathMoveTo {points[0]});
    for (size_t i = 1; i < points.size(); ++i)
    {
      stream->Add(PathLineTo {points[i]});
    }

    stream->Add(PathCloseSubpath {});
    return stream;
  }

  PathStream::PathStream(List<PathSegment> &&segments) noexcept : _segments(krys::move(segments))
  {
  }

  PathStream::PathStream(const List<PathSegment> &segments) noexcept : _segments(segments)
  {
  }

  PathStream::PathStream(PathSegment &&segment) noexcept
  {
    _segments.reserve(16); // We know we're going to append a second segment.
    _segments.emplace_back(krys::move(segment));
  }

  bool PathStream::DefinitelyEqual(const PathImpl &other) const noexcept
  {
    auto *otherAsPathStream = DynamicDowncast<PathStream>(other);
    if (!otherAsPathStream)
    {
      return false;
    }

    if (otherAsPathStream == this)
    {
      return true;
    }

    return Segments() == otherAsPathStream->Segments();
  }

  Ref<PathImpl> PathStream::Copy() const noexcept
  {
    return Create(_segments);
  }

  const PathMoveTo *PathStream::LastIfMoveTo() const noexcept
  {
    if (_segments.empty())
    {
      return nullptr;
    }

    return std::get_if<PathMoveTo>(&_segments.back().GetData());
  }

  void PathStream::Add(PathMoveTo moveTo) noexcept
  {
    Segments().emplace_back(moveTo);
  }

  void PathStream::Add(PathLineTo lineTo) noexcept
  {
    if (const auto *moveTo = LastIfMoveTo())
    {
      Segments().back() = PathSegment {PathDataLine {moveTo->Point, lineTo.Point}};
    }
    else
    {
      Segments().emplace_back(lineTo);
    }
  }

  void PathStream::Add(PathQuadCurveTo quadTo) noexcept
  {
    if (const auto *moveTo = LastIfMoveTo())
    {
      Segments().back() = PathSegment {PathDataQuadCurve {moveTo->Point, quadTo.ControlPoint, quadTo.EndPoint}};
    }
    else
    {
      Segments().emplace_back(quadTo);
    }
  }

  void PathStream::Add(PathBezierCurveTo bezierTo) noexcept
  {
    if (const auto *moveTo = LastIfMoveTo())
    {
      Segments().back() = PathSegment {PathDataBezierCurve {moveTo->Point, bezierTo.ControlPoint1, bezierTo.ControlPoint2,
                                                bezierTo.EndPoint}};
    }
    else
    {
      Segments().emplace_back(bezierTo);
    }
  }

  void PathStream::Add(PathArcTo arcTo) noexcept
  {
    if (const auto *moveTo = LastIfMoveTo())
    {
      Segments().back() = {
        PathDataArc {moveTo->Point, arcTo.ControlPoint1, arcTo.ControlPoint2, arcTo.Radius}};
    }
    else
    {
      Segments().emplace_back(arcTo);
    }
  }

  void PathStream::Add(PathArc arc) noexcept
  {
    Segments().emplace_back(arc);
  }

  void PathStream::Add(PathClosedArc closedArc) noexcept
  {
    Segments().emplace_back(closedArc);
  }

  void PathStream::Add(PathRect rect) noexcept
  {
    Segments().emplace_back(rect);
  }

  void PathStream::Add(PathEllipse ellipse) noexcept
  {
    Segments().emplace_back(ellipse);
  }

  void PathStream::Add(PathEllipseInRect ellipseInRect) noexcept
  {
    Segments().emplace_back(ellipseInRect);
  }

  void PathStream::Add(PathRoundedRect roundedRect) noexcept
  {
    Segments().emplace_back(roundedRect);
  }

  void PathStream::Add(PathContinuousRoundedRect continuousRoundedRect) noexcept
  {
    Segments().emplace_back(continuousRoundedRect);
  }

  void PathStream::Add(PathCloseSubpath) noexcept
  {
    Segments().emplace_back(PathCloseSubpath {});
  }

  void PathStream::ApplySegments(const PathSegmentApplier &applier) const noexcept
  {
    for (auto &segment : _segments)
    {
      applier(segment);
    }
  }

  bool PathStream::ApplyElements(const PathElementApplier &applier) const noexcept
  {
    for (auto &segment : _segments)
    {
      if (!segment.CanApplyElements())
      {
        return false;
      }
    }

    for (auto &segment : _segments)
    {
      (void)segment.ApplyElements(applier);
    }

    return true;
  }

  bool PathStream::Transform(const AffineTransform &transform) noexcept
  {
    for (auto &segment : _segments)
    {
      if (!segment.CanTransform())
      {
        return false;
      }
    }

    for (auto &segment : _segments)
    {
      (void)segment.Transform(transform);
    }

    return true;
  }

  Maybe<PathSegment> PathStream::SingleSegment() const noexcept
  {
    if (_segments.size() != 1)
    {
      return std::nullopt;
    }

    return _segments.front();
  }

  bool PathStream::IsClosed() const noexcept
  {
    if (_segments.empty())
    {
      return false;
    }

    return _segments.back().ClosesSubpath();
  }

  FloatPoint PathStream::CurrentPoint() const noexcept
  {
    if (_segments.empty())
    {
      return {};
    }

    if (auto result = _segments.back().TryGetEndPointWithoutContext())
    {
      return result.value();
    }

    FloatPoint lastMoveToPoint;
    FloatPoint currentPoint;

    for (auto &segment : _segments)
    {
      currentPoint = segment.CalculateEndPoint(currentPoint, lastMoveToPoint);
    }

    return currentPoint;
  }

  FloatRect PathStream::ComputeFastBoundingRect(std::span<const PathSegment> segments) noexcept
  {
    FloatPoint lastMoveToPoint;
    FloatPoint currentPoint;
    FloatRect boundingRect = FloatRect::SmallestRect();

    for (auto &segment : segments)
    {
      segment.ExtendFastBoundingRect(currentPoint, lastMoveToPoint, boundingRect);
      currentPoint = segment.CalculateEndPoint(currentPoint, lastMoveToPoint);
    }

    if (boundingRect.IsSmallest())
    {
      boundingRect.Extend(currentPoint);
    }

    return boundingRect;
  }

  bool PathStream::ComputeHasSubpaths(std::span<const PathSegment> segments) noexcept
  {
    FloatPoint lastMoveToPoint;
    FloatPoint currentPoint;
    FloatRect boundingRect = FloatRect::SmallestRect();

    for (auto &segment : segments)
    {
      segment.ExtendFastBoundingRect(currentPoint, lastMoveToPoint, boundingRect);
      if (!boundingRect.IsSmallest() && (boundingRect.Height() || boundingRect.Width()))
      {
        return true;
      }
      currentPoint = segment.CalculateEndPoint(currentPoint, lastMoveToPoint);
    }

    if (boundingRect.IsSmallest())
    {
      boundingRect.Extend(currentPoint);
    }

    return boundingRect.Height() || boundingRect.Width();
  }

  bool PathStream::HasSubpaths() const noexcept
  {
    return ComputeHasSubpaths(std::span<const PathSegment> {_segments});
  }

  FloatRect PathStream::FastBoundingRect() const noexcept
  {
    return ComputeFastBoundingRect(std::span<const PathSegment> {_segments});
  }

  FloatRect PathStream::ComputeBoundingRect(std::span<const PathSegment> segments) noexcept
  {
    FloatPoint lastMoveToPoint;
    FloatPoint currentPoint;
    FloatRect boundingRect = FloatRect::SmallestRect();

    for (auto &segment : segments)
    {
      segment.ExtendBoundingRect(currentPoint, lastMoveToPoint, boundingRect);
      currentPoint = segment.CalculateEndPoint(currentPoint, lastMoveToPoint);
    }

    if (boundingRect.IsSmallest())
    {
      boundingRect.Extend(currentPoint);
    }

    return boundingRect;
  }

  FloatRect PathStream::BoundingRect() const noexcept
  {
    return ComputeBoundingRect(std::span<const PathSegment> {_segments});
  }
}
