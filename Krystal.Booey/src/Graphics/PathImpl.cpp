#include "Krystal.Booey/Graphics/PathImpl.hpp"

namespace krys::boo
{
  void PathImpl::AddLinesForRect(const FloatRect &rect) noexcept
  {
    Add(PathMoveTo {rect.MinXMinYCorner()});
    Add(PathLineTo {rect.MaxXMinYCorner()});
    Add(PathLineTo {rect.MaxXMaxYCorner()});
    Add(PathLineTo {rect.MinXMaxYCorner()});
    Add(PathCloseSubpath {});
  }

  SmallList<PathSegment, 10> PathImpl::BeziersForRoundedRect(const FloatRoundedRect &roundedRect) noexcept
  {
    const auto &radii = roundedRect.GetRadii();
    const auto &rect = roundedRect.Rect();

    const auto &topLeftRadius = radii.TopLeft();
    const auto &topRightRadius = radii.TopRight();
    const auto &bottomLeftRadius = radii.BottomLeft();
    const auto &bottomRightRadius = radii.BottomRight();

    SmallList<PathSegment, 10> segments;
    segments.push_back(PathSegment(PathMoveTo {FloatPoint(rect.X() + topLeftRadius.Width(), rect.Y())}));

    segments.push_back(PathSegment(PathLineTo {FloatPoint(rect.MaxX() - topRightRadius.Width(), rect.Y())}));
    if (topRightRadius.Width() > 0 || topRightRadius.Height() > 0)
    {
      segments.push_back(PathSegment(
        PathBezierCurveTo {FloatPoint(rect.MaxX() - topRightRadius.Width() * CircleControlPoint(), rect.Y()),
                           FloatPoint(rect.MaxX(), rect.Y() + topRightRadius.Height() * CircleControlPoint()),
                           FloatPoint(rect.MaxX(), rect.Y() + topRightRadius.Height())}));
    }

    segments.push_back(
      PathSegment(PathLineTo {FloatPoint(rect.MaxX(), rect.MaxY() - bottomRightRadius.Height())}));

    if (bottomRightRadius.Width() > 0 || bottomRightRadius.Height() > 0)
    {
      segments.push_back(PathSegment(PathBezierCurveTo {
        FloatPoint(rect.MaxX(), rect.MaxY() - bottomRightRadius.Height() * CircleControlPoint()),
        FloatPoint(rect.MaxX() - bottomRightRadius.Width() * CircleControlPoint(), rect.MaxY()),
        FloatPoint(rect.MaxX() - bottomRightRadius.Width(), rect.MaxY())}));
    }

    segments.push_back(
      PathSegment(PathLineTo {FloatPoint(rect.X() + bottomLeftRadius.Width(), rect.MaxY())}));
    if (bottomLeftRadius.Width() > 0 || bottomLeftRadius.Height() > 0)
    {
      segments.push_back(PathSegment(PathBezierCurveTo {
        FloatPoint(rect.X() + bottomLeftRadius.Width() * CircleControlPoint(), rect.MaxY()),
        FloatPoint(rect.X(), rect.MaxY() - bottomLeftRadius.Height() * CircleControlPoint()),
        FloatPoint(rect.X(), rect.MaxY() - bottomLeftRadius.Height())}));
    }

    segments.push_back(PathSegment(PathLineTo {FloatPoint(rect.X(), rect.Y() + topLeftRadius.Height())}));

    if (topLeftRadius.Width() > 0 || topLeftRadius.Height() > 0)
    {
      segments.push_back(PathSegment(
        PathBezierCurveTo {FloatPoint(rect.X(), rect.Y() + topLeftRadius.Height() * CircleControlPoint()),
                           FloatPoint(rect.X() + topLeftRadius.Width() * CircleControlPoint(), rect.Y()),
                           FloatPoint(rect.X() + topLeftRadius.Width(), rect.Y())}));
    }

    segments.push_back(PathSegment(PathCloseSubpath {}));
    krys_debug_assert(segments.size()
                      <= 10); // Update the preallocated vector size and call sites if the amount changes.
    return segments;
  }

  void PathImpl::ApplySegments(const PathSegmentApplier &applier) const noexcept
  {
    (void)ApplyElements(
      [&](const PathElement &pathElement)
      {
        switch (pathElement.ElementType)
        {
          case PathElement::Type::MoveToPoint:    applier({PathMoveTo {pathElement.Points[0]}}); break;

          case PathElement::Type::AddLineToPoint: applier({PathLineTo {pathElement.Points[0]}}); break;

          case PathElement::Type::AddQuadCurveToPoint:
            applier({PathQuadCurveTo {pathElement.Points[0], pathElement.Points[1]}});
            break;

          case PathElement::Type::AddCurveToPoint:
            applier(
              {PathBezierCurveTo {pathElement.Points[0], pathElement.Points[1], pathElement.Points[2]}});
            break;

          case PathElement::Type::CloseSubpath: applier({PathCloseSubpath {}}); break;
        }
      });
  }

  bool PathImpl::IsClosed() const noexcept
  {
    bool lastElementIsClosed = false;

    // The path is closed if the type of the last PathElement is CloseSubpath. Unfortunately,
    // the only way to access PathElements is sequentially through apply(), there's no random
    // access as if they're in a vector.
    // The lambda below sets lastElementIsClosed if the last PathElement is CloseSubpath.
    // Because lastElementIsClosed is overridden if there are any remaining PathElements
    // to be iterated, its final value is the value of the last iteration.
    // (i.e the last PathElement).
    // FIXME: find a more efficient way to implement this, that does not require iterating
    // through all PathElements.
    (void)ApplyElements([&lastElementIsClosed](const PathElement &element)
                  { lastElementIsClosed = (element.ElementType == PathElement::Type::CloseSubpath); });

    return lastElementIsClosed;
  }

  bool PathImpl::HasSubpaths() const noexcept
  {
    auto rect = FastBoundingRect();
    return rect.Height() || rect.Width();
  }
}
