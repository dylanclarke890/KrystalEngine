#include "Krystal.Booey/HTML/Canvas/CanvasPath.hpp"
#include "Krystal.Booey/Core/MathExtras.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace krys::boo::html
{
  // https://html.spec.whatwg.org/#dom-context-2d-closepath
  void CanvasPath::ClosePath() noexcept
  {
    // The closePath() method, when invoked, must do nothing if the object's path has no subpaths.
    // Otherwise, it must mark the last subpath as closed, create a new subpath whose first point is the same
    // as the previous subpath's first point, and finally add this new subpath to the path.
    if (_path.IsEmpty())
    {
      return;
    }
    if (_path.HasSubpaths())
    {
      _path.CloseSubpath();
    }
  }

  // https://html.spec.whatwg.org/#dom-context-2d-moveto
  void CanvasPath::MoveTo(float x, float y) noexcept
  {
    // 1. If either of the arguments are infinite or NaN, then return.
    if (!std::isfinite(x) || !std::isfinite(y))
    {
      return;
    }
    // (Nor while the transform can't be inverted.)
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return;
    }

    // 2. Create a new subpath with the specified point as its first (and only) point.
    _path.MoveTo(FloatPoint(x, y));
  }

  void CanvasPath::LineTo(FloatPoint point) noexcept
  {
    LineTo(point.X(), point.Y());
  }

  // https://html.spec.whatwg.org/#dom-context-2d-lineto
  void CanvasPath::LineTo(float x, float y) noexcept
  {
    // 1. If either of the arguments are infinite or NaN, then return.
    if (!std::isfinite(x) || !std::isfinite(y))
    {
      return;
    }
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return;
    }

    // 2. If the object's path has no subpaths, then ensure there is a subpath for (x, y).
    FloatPoint p1 = FloatPoint(x, y);
    if (_path.IsEmpty())
    {
      _path.MoveTo(p1);
    }
    // 3. Otherwise, connect the last point in the subpath to the given point (x, y) using a straight line,
    // and
    //    then add the given point (x, y) to the subpath.
    // (A line to the last point is left out, as stroking prunes it.)
    else if (p1 != _path.CurrentPoint())
    {
      _path.AddLineTo(p1);
    }
  }

  // https://html.spec.whatwg.org/#dom-context-2d-quadraticcurveto
  void CanvasPath::QuadraticCurveTo(float cpx, float cpy, float x, float y) noexcept
  {
    // 1. If any of the arguments are infinite or NaN, then return.
    if (!std::isfinite(cpx) || !std::isfinite(cpy) || !std::isfinite(x) || !std::isfinite(y))
    {
      return;
    }
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return;
    }

    // 2. Ensure there is a subpath for (cpx, cpy).
    if (_path.IsEmpty())
    {
      _path.MoveTo(FloatPoint(cpx, cpy));
    }

    // 3. Connect the last point in the subpath to the given point (x, y) using a quadratic Bézier curve with
    //    control point (cpx, cpy).
    // 4. Add the given point (x, y) to the subpath.

    FloatPoint p1 = FloatPoint(x, y);
    FloatPoint cp = FloatPoint(cpx, cpy);
    if (p1 != _path.CurrentPoint() || p1 != cp)
    {
      _path.AddQuadCurveTo(cp, p1);
    }
  }

  // https://html.spec.whatwg.org/#dom-context-2d-beziercurveto
  void CanvasPath::BezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y) noexcept
  {
    // 1. If any of the arguments are infinite or NaN, then return.
    if (!std::isfinite(cp1x) || !std::isfinite(cp1y) || !std::isfinite(cp2x) || !std::isfinite(cp2y)
        || !std::isfinite(x) || !std::isfinite(y))
    {
      return;
    }
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return;
    }

    // 2. Ensure there is a subpath for (cp1x, cp1y).
    if (_path.IsEmpty())
    {
      _path.MoveTo(FloatPoint(cp1x, cp1y));
    }

    // 3. Connect the last point in the subpath to the given point (x, y) using a cubic Bézier curve with
    //    control points (cp1x, cp1y) and (cp2x, cp2y).
    // 4. Add the point (x, y) to the subpath.

    FloatPoint p1 = FloatPoint(x, y);
    FloatPoint cp1 = FloatPoint(cp1x, cp1y);
    FloatPoint cp2 = FloatPoint(cp2x, cp2y);
    if (p1 != _path.CurrentPoint() || p1 != cp1 || p1 != cp2)
    {
      _path.AddBezierCurveTo(cp1, cp2, p1);
    }
  }

  // https://html.spec.whatwg.org/#dom-context-2d-arcto
  dom::ExceptionOr<void> CanvasPath::ArcTo(float x1, float y1, float x2, float y2, float r) noexcept
  {
    // 1. If any of the arguments are infinite or NaN, then return.
    if (!std::isfinite(x1) || !std::isfinite(y1) || !std::isfinite(x2) || !std::isfinite(y2)
        || !std::isfinite(r))
    {
      return {};
    }

    // 3. If radius is negative, then throw an "IndexSizeError" DOMException.
    // (Before step 2, so a negative radius leaves the path as it was.)
    if (r < 0)
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }

    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return {};
    }

    FloatPoint p1 = FloatPoint(x1, y1);
    FloatPoint p2 = FloatPoint(x2, y2);

    // 2. Ensure there is a subpath for (x1, y1).
    // 4. Let the point (x0, y0) be the last point in the subpath, transformed by the inverse of the current
    //    transformation matrix.
    // (The path is kept in the current transform's space.)
    if (_path.IsEmpty())
    {
      _path.MoveTo(p1);
    }
    // 5. If the point (x0, y0) is equal to the point (x1, y1), or if the point (x1, y1) is equal to the point
    //    (x2, y2), or if radius is zero, then add the point (x1, y1) to the subpath, and connect that point
    //    to the previous point (x0, y0) by a straight line.
    else if (p1 == _path.CurrentPoint() || p1 == p2 || !r)
    {
      LineTo(x1, y1);
    }
    // 6. Otherwise, if the points (x0, y0), (x1, y1), and (x2, y2) all lie on a single straight line, then
    // add
    //    the point (x1, y1) to the subpath, and connect that point to the previous point (x0, y0) by a
    //    straight line.
    // 7. Otherwise, let The Arc be the shortest arc [...]
    // (Both are Path::AddArcTo's.)
    else
    {
      _path.AddArcTo(p1, p2, r);
    }

    return {};
  }

  static void NormaliseAngles(float &startAngle, float &endAngle, bool anticlockwise) noexcept
  {
    constexpr auto twoPiFloat = 2 * std::numbers::pi_v<float>;
    float newStartAngle = std::fmod(startAngle, twoPiFloat);
    if (newStartAngle < 0)
    {
      newStartAngle += twoPiFloat;
    }

    float delta = newStartAngle - startAngle;
    startAngle = newStartAngle;
    endAngle = endAngle + delta;
    krys_debug_assert(
      newStartAngle >= 0
      && (newStartAngle < twoPiFloat || AreEssentiallyEqual<float>(newStartAngle, twoPiFloat)));

    if (anticlockwise && startAngle - endAngle >= twoPiFloat)
    {
      endAngle = startAngle - twoPiFloat;
    }
    else if (!anticlockwise && endAngle - startAngle >= twoPiFloat)
    {
      endAngle = startAngle + twoPiFloat;
    }
  }

  // https://html.spec.whatwg.org/#dom-context-2d-arc
  // (The ellipse method steps with radius as both radii and no rotation.)
  dom::ExceptionOr<void> CanvasPath::Arc(float x, float y, float radius, float startAngle, float endAngle,
                                         bool anticlockwise) noexcept
  {
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(radius) || !std::isfinite(startAngle)
        || !std::isfinite(endAngle))
    {
      return {};
    }

    if (radius < 0)
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }

    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return {};
    }

    NormaliseAngles(startAngle, endAngle, anticlockwise);

    if (!radius || startAngle == endAngle)
    {
      // The arc is empty but we still need to draw the connecting line.
      LineTo(x + radius * std::cos(startAngle), y + radius * std::sin(startAngle));
      return {};
    }

    _path.AddArc(FloatPoint(x, y), radius, startAngle, endAngle,
                 anticlockwise ? RotationDirection::Counterclockwise : RotationDirection::Clockwise);
    return {};
  }

  // https://html.spec.whatwg.org/#ellipse-method-steps
  dom::ExceptionOr<void> CanvasPath::Ellipse(float x, float y, float radiusX, float radiusY, float rotation,
                                             float startAngle, float endAngle, bool anticlockwise) noexcept
  {
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(radiusX) || !std::isfinite(radiusY)
        || !std::isfinite(rotation) || !std::isfinite(startAngle) || !std::isfinite(endAngle))
    {
      return {};
    }

    if (radiusX < 0 || radiusY < 0)
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }

    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return {};
    }

    NormaliseAngles(startAngle, endAngle, anticlockwise);

    if ((!radiusX && !radiusY) || startAngle == endAngle)
    {
      AffineTransform transform;
      transform.Translate(x, y).RotateRadians(rotation);

      LineTo(transform.MapPoint(FloatPoint(radiusX * std::cos(startAngle), radiusY * std::sin(startAngle))));
      return {};
    }

    if (!radiusX || !radiusY)
    {
      AffineTransform transform;
      transform.Translate(x, y).RotateRadians(rotation);

      LineTo(transform.MapPoint(FloatPoint(radiusX * std::cos(startAngle), radiusY * std::sin(startAngle))));

      if (!anticlockwise)
      {
        for (float angle = startAngle - std::fmod(startAngle, piOverTwoFloat) + piOverTwoFloat;
             angle < endAngle; angle += piOverTwoFloat)
        {
          LineTo(transform.MapPoint(FloatPoint(radiusX * std::cos(angle), radiusY * std::sin(angle))));
        }
      }
      else
      {
        for (float angle = startAngle - std::fmod(startAngle, piOverTwoFloat); angle > endAngle;
             angle -= piOverTwoFloat)
        {
          LineTo(transform.MapPoint(FloatPoint(radiusX * std::cos(angle), radiusY * std::sin(angle))));
        }
      }

      LineTo(transform.MapPoint(FloatPoint(radiusX * std::cos(endAngle), radiusY * std::sin(endAngle))));
      return {};
    }

    _path.AddEllipse(FloatPoint(x, y), radiusX, radiusY, rotation, startAngle, endAngle,
                     anticlockwise ? RotationDirection::Counterclockwise : RotationDirection::Clockwise);
    return {};
  }

  // https://html.spec.whatwg.org/#dom-context-2d-rect
  void CanvasPath::Rect(float x, float y, float width, float height) noexcept
  {
    if (!HasInvertibleTransform()) KRYS_UNLIKELY
    {
      return;
    }

    // 1. If any of the arguments are infinite or NaN, then return.
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(width) || !std::isfinite(height))
    {
      return;
    }

    // (A rectangle with no area is its new subpath, as stroking prunes the rest.)
    if (!width && !height)
    {
      _path.MoveTo(FloatPoint(x, y));
      return;
    }

    // 2. Create a new subpath containing just the four points (x, y), (x+w, y), (x+w, y+h), (x, y+h), in that
    //    order, with those four points connected by straight lines.
    // 3. Mark the subpath as closed.
    // 4. Create a new subpath with the point (x, y) as the only point in the subpath.
    _path.AddRect(FloatRect(x, y, width, height));
  }

  dom::ExceptionOr<void> CanvasPath::RoundRect(float x, float y, float width, float height,
                                               const RadiusVariant &radii) noexcept
  {
    return RoundRect(x, y, width, height, Span<const RadiusVariant>(&radii, 1));
  }

  dom::ExceptionOr<void> CanvasPath::RoundRect(float x, float y, float width, float height,
                                               Span<const RadiusVariant> radii) noexcept
  {
    // https://html.spec.whatwg.org/#dom-context-2d-roundrect
    // 1. If any of x, y, w, or h are infinite or NaN, then return.

    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(width) || !std::isfinite(height))
    {
      return {};
    }

    // 2. If radii is an unrestricted double or DOMPointInit, then set radii to « radii ».
    // (A single radius is a span of one.)
    // 3. If radii is not a list of size one, two, three, or four, then throw a RangeError.
    if (radii.size() > 4 || radii.empty())
    {
      StringBuilder message;
      message.Append(u8"radii must contain at least 1 element, up to 4. It contained ", radii.size(),
                     u8" elements.");
      return dom::Exception {dom::ExceptionCode::RangeError, krys::move(message).ToString()};
    }

    // 4. Let normalizedRadii be an empty list.
    SmallList<FloatPoint, 4> normalizedRadii;

    // 5. For each radius of radii:
    for (const auto &radius : radii)
    {
      auto shouldReturnSilently = false;
      auto exception = SwitchOn(
        radius,
        // 5a. If radius is a DOMPointInit:
        [&normalizedRadii, &shouldReturnSilently](const dom::DOMPointInit &point) -> dom::ExceptionOr<void>
        {
          // 5a.i. If radius["x"] or radius["y"] is infinite or NaN, then return.
          if (!std::isfinite(point.X) || !std::isfinite(point.Y))
          {
            shouldReturnSilently = true;
            return {};
          }

          // 5a.ii. If radius["x"] or radius["y"] is negative, then throw a RangeError.
          if (point.X < 0 || point.Y < 0)
          {
            return dom::Exception {dom::ExceptionCode::RangeError,
                                   u8"radius point coordinates must be positive"};
          }

          // 5a.iii. Otherwise, append radius to normalizedRadii.
          normalizedRadii.push_back({static_cast<float>(point.X), static_cast<float>(point.Y)});
          return {};
        },
        // 5b. If radius is a unrestricted double:
        [&normalizedRadii, &shouldReturnSilently](double radiusValue) -> dom::ExceptionOr<void>
        {
          // 5b.i. If radius is infinite or NaN, then return.
          if (!std::isfinite(radiusValue))
          {
            shouldReturnSilently = true;
            return {};
          }

          // 5b.ii. If radius is negative, then throw a RangeError.
          if (radiusValue < 0)
          {
            return dom::Exception {dom::ExceptionCode::RangeError, u8"radius value must be positive"};
          }

          // 5b.iii. Otherwise, append «[ "x" → radius, "y" → radius ]» to normalizedRadii.
          normalizedRadii.push_back({static_cast<float>(radiusValue), static_cast<float>(radiusValue)});
          return {};
        });
      if (exception.HasException() || shouldReturnSilently)
      {
        return exception;
      }
    }

    // Degenerate case, fall back to regular rect.
    // We do not do this before parsing the radii in order to make sure the Exceptions can be raised.
    if (!width || !height)
    {
      Rect(x, y, width, height);
      return {};
    }

    // 6. Let upperLeft, upperRight, lowerRight, and lowerLeft be null.
    FloatPoint upperLeft, upperRight, lowerRight, lowerLeft;

    switch (normalizedRadii.size())
    {
      case 4:
        // 7. If normalizedRadii's size is 4, then set upperLeft to normalizedRadii[0], set upperRight to
        // normalizedRadii[1], set lowerRight to normalizedRadii[2], and set lowerLeft to normalizedRadii[3].
        upperLeft = normalizedRadii[0];
        upperRight = normalizedRadii[1];
        lowerRight = normalizedRadii[2];
        lowerLeft = normalizedRadii[3];
        break;
      case 3:
        // 8. If normalizedRadii's size is 3, then set upperLeft to normalizedRadii[0], set upperRight and
        // lowerLeft to normalizedRadii[1], and set lowerRight to normalizedRadii[2].
        upperLeft = normalizedRadii[0];
        upperRight = normalizedRadii[1];
        lowerRight = normalizedRadii[2];
        lowerLeft = normalizedRadii[1];
        break;
      case 2:
        // 9. If normalizedRadii's size is 2, then set upperLeft and lowerRight to normalizedRadii[0] and set
        // upperRight and lowerLeft to normalizedRadii[1].
        upperLeft = normalizedRadii[0];
        upperRight = normalizedRadii[1];
        lowerRight = normalizedRadii[0];
        lowerLeft = normalizedRadii[1];
        break;
      case 1:
        // 10. If normalizedRadii's size is 1, then set upperLeft, upperRight, lowerRight, and lowerLeft to
        // normalizedRadii[0].
        upperLeft = normalizedRadii[0];
        upperRight = normalizedRadii[0];
        lowerRight = normalizedRadii[0];
        lowerLeft = normalizedRadii[0];
        break;
      default: krys_unreachable(); break;
    }

    // Must handle clockwise and counter-clockwise directions properly so path winding works correctly.
    bool clockwise = true;
    if (width < 0)
    {
      clockwise = !clockwise;
      width = std::abs(width);
      x -= width;
      std::swap(upperLeft, upperRight);
      std::swap(lowerLeft, lowerRight);
    }

    if (height < 0)
    {
      clockwise = !clockwise;
      height = std::abs(height);
      y -= height;
      std::swap(upperLeft, lowerLeft);
      std::swap(upperRight, lowerRight);
    }

    // 11. Corner curves must not overlap. Scale all radii to prevent this:
    // 11a. Let top be upperLeft["x"] + upperRight["x"].
    auto top = upperLeft.X() + upperRight.X();

    // 11b. Let right be upperRight["y"] + lowerRight["y"].
    auto right = upperRight.Y() + lowerRight.Y();

    // 11c. Let bottom be lowerRight["x"] + lowerLeft["x"].
    auto bottom = lowerRight.X() + lowerLeft.X();

    // 11d. Let left be upperLeft["y"] + lowerLeft["y"].
    auto left = upperLeft.Y() + lowerLeft.Y();

    // 11e. Let scale be the minimum value of the ratios w / top, h / right, w / bottom, h / left.
    auto scale = std::min({width / top, height / right, width / bottom, height / left});

    // 11f. If scale is less than 1, then set the x and y members of upperLeft, upperRight, lowerLeft, and
    //      lowerRight to their current values multiplied by scale.
    if (scale < 1)
    {
      upperLeft.Scale(scale);
      upperRight.Scale(scale);
      lowerLeft.Scale(scale);
      lowerRight.Scale(scale);
    }

    // 12. Create a new subpath:
    _path.MoveTo({x + upperLeft.X(), y});

    // (Its clockwise substeps are Path::AddRoundedRect's.)
    if (clockwise)
    {
      _path.AddRoundedRect(FloatRoundedRect {
        FloatRect(x, y, width, height),
        {upperLeft.X(), upperLeft.Y()},
        {upperRight.X(), upperRight.Y()},
        {lowerLeft.X(), lowerLeft.Y()},
        {lowerRight.X(), lowerRight.Y()},
      });
    }
    else
    {
      const auto circleControlPoint = Path::CircleControlPoint();
      // Top Left corner
      if (upperLeft.X() > 0 || upperLeft.Y() > 0)
      {
        _path.AddBezierCurveTo({x + upperLeft.X() * circleControlPoint, y},
                               {x, y + upperLeft.Y() * circleControlPoint}, {x, y + upperLeft.Y()});
      }
      // Left edge
      _path.AddLineTo({x, y + height - lowerLeft.Y()});
      // Bottom left corner
      if (lowerLeft.X() > 0 || lowerLeft.Y() > 0)
      {
        _path.AddBezierCurveTo({x, y + height - lowerLeft.Y() * circleControlPoint},
                               {x + lowerLeft.X() * circleControlPoint, y + height},
                               {x + lowerLeft.X(), y + height});
      }
      // Bottom edge
      _path.AddLineTo({x + width - lowerRight.X(), y + height});
      // Bottom right corner
      if (lowerRight.X() > 0 || lowerRight.Y() > 0)
      {
        _path.AddBezierCurveTo({x + width - lowerRight.X() * circleControlPoint, y + height},
                               {x + width, y + height - lowerRight.Y() * circleControlPoint},
                               {x + width, y + height - lowerRight.Y()});
      }
      // Right edge
      _path.AddLineTo({x + width, y + upperRight.Y()});
      // Top right corner
      if (upperRight.X() > 0 || upperRight.Y() > 0)
      {
        _path.AddBezierCurveTo({x + width, y + upperRight.Y() * circleControlPoint},
                               {x + width - upperRight.X() * circleControlPoint, y},
                               {x + width - upperRight.X(), y});
      }
      // Top edge
      _path.AddLineTo({x + upperLeft.X(), y});
    }

    // 13. Mark the subpath as closed.
    _path.CloseSubpath();

    // 14. Create a new subpath with the point (x, y) as the only point in the subpath.
    _path.MoveTo({x, y});

    return {};
  }
}
