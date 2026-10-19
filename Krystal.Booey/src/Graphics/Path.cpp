#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Graphics/PathImpl.hpp"

#include "Krystal.Booey/Graphics/PathStream.hpp"
#include "Krystal.Booey/Graphics/PathTraversalState.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Path::Path(const List<FloatPoint> &points) noexcept
      : _data(DataRef<PathImpl> {Ref<PathImpl> {PathStream::Create(points)}})
  {
  }

  Path::Path(List<PathSegment> &&segments) noexcept
  {
    if (segments.empty())
    {
      return;
    }

    if (segments.size() == 1)
    {
      _data = krys::move(segments[0]);
    }
    else
    {
      _data = DataRef<PathImpl> {Ref<PathImpl> {PathStream::Create(krys::move(segments))}};
    }
  }

  Path::Path(Ref<PathImpl> &&impl) noexcept : _data(DataRef<PathImpl> {krys::move(impl)})
  {
  }

  bool Path::DefinitelyEqual(const Path &other) const noexcept
  {
    if (&other == this)
    {
      return true;
    }

    return krys::SwitchOn(
      _data, [&](std::monostate) { return other.IsEmpty(); },
      [&](const PathSegment &segment)
      {
        auto otherSegment = other.SingleSegment();
        return otherSegment && segment == otherSegment.value();
      },
      [&](const DataRef<PathImpl> &impl)
      {
        if (auto singleSegment = impl->SingleSegment())
        {
          auto otherSegment = other.SingleSegment();
          return otherSegment && singleSegment == otherSegment.value();
        }

        return impl.Ptr() && other.AsImpl() && impl->DefinitelyEqual(*other.AsImpl());
      });
  }

  PathImpl &Path::SetImpl(Ref<PathImpl> &&impl) noexcept
  {
    auto &pathImpl = *impl;
    _data = DataRef<PathImpl> {krys::move(impl)};
    return pathImpl;
  }

  PathImpl &Path::EnsureImpl() noexcept
  {
    if (auto segment = AsSingle())
    {
      return SetImpl(Ref<PathImpl> {PathStream::Create(krys::move(*segment))});
    }

    if (auto *impl = AsImpl())
    {
      return *impl;
    }
    krys_debug_assert_msg(false, "Unreachable"); // Impl is never empty.
    return SetImpl(Ref<PathImpl> {PathStream::Create()});
  }

  PathImpl *Path::AsImpl() noexcept
  {
    if (auto ref = std::get_if<DataRef<PathImpl>>(&_data))
    {
      return &ref->Access();
    }
    return nullptr;
  }

  const PathImpl *Path::AsImpl() const noexcept
  {
    if (auto ref = std::get_if<DataRef<PathImpl>>(&_data))
    {
      return ref->Ptr();
    }
    return nullptr;
  }

  KRYS_NODISCARD static FloatRoundedRect CalculateEvenRoundedRect(const FloatRect &rect,
                                                                  const FloatSize &roundingRadii) noexcept
  {
    FloatSize radius(roundingRadii);
    FloatSize halfSize = rect.Size() / 2;

    // Apply the SVG corner radius constraints, per the rect section of the SVG shapes spec: if
    // one of rx,ry is negative, then the other corner radius value is used. If both values are
    // negative then rx = ry = 0. If rx is greater than half of the width of the rectangle
    // then set rx to half of the width; ry is handled similarly.

    if (radius.Width() < 0)
    {
      radius.SetWidth(std::max<float>(radius.Height(), 0));
    }

    if (radius.Height() < 0)
    {
      radius.SetHeight(radius.Width());
    }

    if (radius.Width() > halfSize.Width())
    {
      radius.SetWidth(halfSize.Width());
    }

    if (radius.Height() > halfSize.Height())
    {
      radius.SetHeight(halfSize.Height());
    }

    return FloatRoundedRect(rect, radius, radius, radius, radius);
  }

  void Path::AddRoundedRect(const FloatRoundedRect &roundedRect, PathRoundedRect::Strategy strategy) noexcept
  {
    if (roundedRect.IsEmpty())
    {
      return;
    }

    if (!roundedRect.IsRenderable())
    {
      // If all the radii cannot be accommodated, return a rect.
      AddRect(roundedRect.Rect());
      return;
    }

    if (IsEmpty())
    {
      _data = PathSegment(PathRoundedRect {roundedRect, strategy});
    }
    else
    {
      EnsureImpl().Add(PathRoundedRect {roundedRect, strategy});
    }
  }

  void Path::AddRoundedRect(const FloatRect &rect, const FloatSize &roundingRadii,
                            PathRoundedRect::Strategy strategy) noexcept
  {
    if (rect.IsEmpty())
    {
      return;
    }

    if (IsEmpty())
    {
      _data = PathSegment(PathRoundedRect {CalculateEvenRoundedRect(rect, roundingRadii), strategy});
    }
    else
    {
      EnsureImpl().Add(PathRoundedRect {CalculateEvenRoundedRect(rect, roundingRadii), strategy});
    }
  }

  void Path::AddRoundedRect(const LayoutRoundedRect &rect) noexcept
  {
    AddRoundedRect(FloatRoundedRect(rect));
  }

  void Path::AddContinuousRoundedRect(const FloatRect &rect, const float cornerRadius) noexcept
  {
    if (rect.IsEmpty())
    {
      return;
    }

    PathContinuousRoundedRect continuousRoundedRect {rect, cornerRadius, cornerRadius};
    if (IsEmpty())
    {
      _data = PathSegment(continuousRoundedRect);
    }
    else
    {
      EnsureImpl().Add(continuousRoundedRect);
    }
  }

  void Path::AddContinuousRoundedRect(const FloatRect &rect, const float cornerWidth,
                                      const float cornerHeight) noexcept
  {
    if (rect.IsEmpty())
    {
      return;
    }

    PathContinuousRoundedRect continuousRoundedRect {rect, cornerWidth, cornerHeight};
    if (IsEmpty())
    {
      _data = PathSegment(continuousRoundedRect);
    }
    else
    {
      EnsureImpl().Add(continuousRoundedRect);
    }
  }

  void Path::AddPath(const Path &path, const AffineTransform &transform) noexcept
  {
    if (path.IsEmpty() || !transform.IsInvertible())
    {
      return;
    }

    // FIXME: This should inspect the incoming path and add just the segments if possible.
    // FIXME: Without a platform path, only segments that can be transformed are added.
    for (auto segment : path.Segments())
    {
      if (segment.Transform(transform))
      {
        EnsureImpl().AddSegment(krys::move(segment));
      }
    }
  }

  void Path::ApplySegments(const PathSegmentApplier &applier) const noexcept
  {
    if (auto segment = AsSingle())
    {
      applier(*segment);
    }
    else if (auto *impl = AsImpl())
    {
      impl->ApplySegments(applier);
    }
  }

  bool Path::Contains(const FloatPoint &point, WindRule rule) const noexcept
  {
    if (IsEmpty() || !FastBoundingRect().Contains(point))
    {
      return false;
    }

    // The winding number of the point: each edge crossing the horizontal line through it to its right counts
    // one way going up and the other going down. Curves are flattened into lines.
    constexpr int32 curveSteps = 16;
    int32 winding = 0;
    FloatPoint current;
    FloatPoint subpathStart;
    auto addEdge = [&](FloatPoint from, FloatPoint to)
    {
      if (from.Y() <= point.Y())
      {
        if (to.Y() > point.Y()
            && (to.X() - from.X()) * (point.Y() - from.Y()) - (point.X() - from.X()) * (to.Y() - from.Y())
                 > 0)
        {
          ++winding;
        }
      }
      else if (to.Y() <= point.Y()
               && (to.X() - from.X()) * (point.Y() - from.Y()) - (point.X() - from.X()) * (to.Y() - from.Y())
                    < 0)
      {
        --winding;
      }
    };
    ApplyElements(
      [&](const PathElement &element)
      {
        switch (element.ElementType)
        {
          case PathElement::Type::MoveToPoint:
            addEdge(current, subpathStart);
            current = subpathStart = element.Points[0];
            break;
          case PathElement::Type::AddLineToPoint:
            addEdge(current, element.Points[0]);
            current = element.Points[0];
            break;
          case PathElement::Type::AddQuadCurveToPoint:
          {
            FloatPoint start = current;
            for (int32 step = 1; step <= curveSteps; ++step)
            {
              float t = static_cast<float>(step) / curveSteps;
              float u = 1 - t;
              FloatPoint next(
                u * u * start.X() + 2 * u * t * element.Points[0].X() + t * t * element.Points[1].X(),
                u * u * start.Y() + 2 * u * t * element.Points[0].Y() + t * t * element.Points[1].Y());
              addEdge(current, next);
              current = next;
            }
            break;
          }
          case PathElement::Type::AddCurveToPoint:
          {
            FloatPoint start = current;
            for (int32 step = 1; step <= curveSteps; ++step)
            {
              float t = static_cast<float>(step) / curveSteps;
              float u = 1 - t;
              FloatPoint next(u * u * u * start.X() + 3 * u * u * t * element.Points[0].X()
                                + 3 * u * t * t * element.Points[1].X() + t * t * t * element.Points[2].X(),
                              u * u * u * start.Y() + 3 * u * u * t * element.Points[0].Y()
                                + 3 * u * t * t * element.Points[1].Y() + t * t * t * element.Points[2].Y());
              addEdge(current, next);
              current = next;
            }
            break;
          }
          case PathElement::Type::CloseSubpath:
            addEdge(current, subpathStart);
            current = subpathStart;
            break;
        }
      });
    // Subpaths are filled closed.
    addEdge(current, subpathStart);

    return rule == WindRule::EvenOdd ? (winding % 2) != 0 : winding != 0;
  }

  void Path::ApplyElements(const PathElementApplier &applier) const noexcept
  {
    if (IsEmpty())
    {
      return;
    }

    auto segment = AsSingle();
    if (segment && segment->ApplyElements(applier))
    {
      return;
    }

    auto *impl = AsImpl();
    if (impl && impl->ApplyElements(applier))
    {
      return;
    }

    // There's no platform path to flatten the segments that can't produce elements, so rects become lines and
    // rounded rects and ellipses become beziers, as a platform path preferring them adds them
    // (PathImpl::BeziersForRoundedRect).
    // FIXME: Arcs, closed arcs, rotated ellipses and continuous rounded rects need a platform path; they
    // apply nothing.
    ApplySegments(
      [&applier](const PathSegment &segment)
      {
        if (segment.ApplyElements(applier))
        {
          return;
        }

        auto applyBeziers = [&applier](const FloatRoundedRect &roundedRect)
        {
          for (const auto &bezierSegment : PathImpl::BeziersForRoundedRect(roundedRect))
          {
            (void)bezierSegment.ApplyElements(applier);
          }
        };

        if (const auto *rect = std::get_if<PathRect>(&segment.GetData()))
        {
          // As a platform path adds a rect: a closed subpath clockwise from its top left corner.
          const auto &r = rect->Rect;
          applier({PathElement::Type::MoveToPoint, {r.MinXMinYCorner()}});
          applier({PathElement::Type::AddLineToPoint, {r.MaxXMinYCorner()}});
          applier({PathElement::Type::AddLineToPoint, {r.MaxXMaxYCorner()}});
          applier({PathElement::Type::AddLineToPoint, {r.MinXMaxYCorner()}});
          applier({PathElement::Type::CloseSubpath, {}});
        }
        else if (const auto *roundedRect = std::get_if<PathRoundedRect>(&segment.GetData()))
        {
          applyBeziers(roundedRect->RoundedRect);
        }
        else if (const auto *ellipse = std::get_if<PathEllipseInRect>(&segment.GetData()))
        {
          applyBeziers(FloatRoundedRect(
            ellipse->Rect, FloatRoundedRect::Radii(ellipse->Rect.Width() / 2, ellipse->Rect.Height() / 2)));
        }
      });
  }

  void Path::Clear() noexcept
  {
    _data = std::monostate {};
  }

  void Path::Translate(const FloatSize &delta) noexcept
  {
    Transform(AffineTransform::MakeTranslation(delta));
  }

  void Path::Transform(const AffineTransform &transform) noexcept
  {
    if (transform.IsIdentity() || IsEmpty())
    {
      return;
    }

    auto segment = AsSingle();
    if (segment && segment->Transform(transform))
    {
      return;
    }

    auto *impl = AsImpl();
    if (impl && impl->Transform(transform))
    {
      return;
    }

    // FIXME: Segments that can't be transformed (arcs and ellipses under some transforms) need a platform
    // path.
  }

  Maybe<PathSegment> Path::SingleSegment() const noexcept
  {
    if (auto segment = AsSingle())
    {
      return *segment;
    }

    if (auto *impl = AsImpl())
    {
      return impl->SingleSegment();
    }

    return std::nullopt;
  }

  Maybe<PathDataLine> Path::SingleDataLine() const noexcept
  {
    if (auto segment = SingleSegment())
    {
      if (auto *line = std::get_if<PathDataLine>(&segment->GetData()))
      {
        return *line;
      }
    }
    return std::nullopt;
  }

  bool Path::DefinitelySingleLine() const noexcept
  {
    return !!SingleDataLine();
  }

  const List<PathSegment> *Path::SegmentsIfExists() const noexcept
  {
    if (auto *impl = AsImpl())
    {
      if (auto *stream = DynamicDowncast<PathStream>((*impl)))
      {
        return &stream->Segments();
      }
    }

    return nullptr;
  }

  List<PathSegment> Path::Segments() const noexcept
  {
    if (const auto *segments = SegmentsIfExists())
    {
      return *segments;
    }

    List<PathSegment> segments;
    ApplySegments([&](const PathSegment &segment) { segments.push_back(segment); });

    return segments;
  }

  float Path::Length() const noexcept
  {
    PathTraversalState traversalState(PathTraversalState::Action::TotalLength);

    ApplyElements([&traversalState](const PathElement &element)
                  { (void)traversalState.ProcessPathElement(element); });

    return traversalState.TotalLength();
  }

  bool Path::IsClosed() const noexcept
  {
    if (auto segment = AsSingle())
    {
      return segment->ClosesSubpath();
    }

    if (auto *impl = AsImpl())
    {
      return impl->IsClosed();
    }

    return false;
  }

  PathTraversalState Path::TraversalStateAtLength(float length) const noexcept
  {
    PathTraversalState traversalState(PathTraversalState::Action::VectorAtLength, length);

    ApplyElements([&traversalState](const PathElement &element)
                  { (void)traversalState.ProcessPathElement(element); });

    return traversalState;
  }

  FloatPoint Path::PointAtLength(float length) const noexcept
  {
    return TraversalStateAtLength(length).Current();
  }

  bool Path::HasSubpaths() const noexcept
  {
    if (auto *segment = AsSingle())
    {
      return PathStream::ComputeHasSubpaths(std::span<const PathSegment> {segment, 1});
    }

    if (auto *impl = AsImpl())
    {
      return impl->HasSubpaths();
    }

    return false;
  }

  FloatRect Path::FastBoundingRect() const noexcept
  {
    if (auto *segment = AsSingle())
    {
      return segment->FastBoundingRect();
    }

    if (auto *impl = AsImpl())
    {
      return impl->FastBoundingRect();
    }

    return {};
  }

  FloatRect Path::BoundingRect() const noexcept
  {
    if (auto *segment = AsSingle())
    {
      return PathStream::ComputeBoundingRect(std::span<const PathSegment> {segment, 1});
    }

    if (auto *impl = AsImpl())
    {
      return impl->BoundingRect();
    }

    return {};
  }

  TextStream &operator<<(TextStream &ts, const Path &path) noexcept
  {
    bool isFirst = true;
    path.ApplySegments(
      [&ts, &isFirst](const PathSegment &segment)
      {
        if (!isFirst)
        {
          ts << ", ";
        }
        else
        {
          isFirst = false;
        }
        ts << segment;
      });
    return ts;
  }
}
