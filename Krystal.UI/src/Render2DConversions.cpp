#include "Krystal.UI/Render2DConversions.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <cmath>

namespace krys::ui
{
  SRGBA<uint8> ToColour(const boo::Color &color, float alpha) noexcept
  {
    if (!color.IsValid())
    {
      return {};
    }
    const auto colour = color.ToColorTypeLossy<SRGBA<uint8>>();
    if (alpha >= 1.f)
    {
      return colour;
    }
    const auto [red, green, blue, opacity] = colour.Resolved();
    return {red, green, blue,
            static_cast<uint8>(std::lround(static_cast<float>(opacity) * std::max(alpha, 0.f)))};
  }

  Maybe<gfx::RoundedRect2D> ToCircularRoundedRect(const boo::FloatRoundedRect &rect) noexcept
  {
    // A corner this close to circular is drawn as the circle between its radii: no point of its curve moves
    // by more than half the difference, a thirty-second of a pixel.
    constexpr float circularCornerTolerance = 1.f / 16.f;

    const auto &radii = rect.GetRadii();
    auto circular = [](const boo::FloatSize &radius)
    {
      return std::abs(radius.Width() - radius.Height()) <= circularCornerTolerance;
    };
    if (!circular(radii.TopLeft()) || !circular(radii.TopRight()) || !circular(radii.BottomRight())
        || !circular(radii.BottomLeft()))
    {
      return std::nullopt;
    }
    auto radius = [](const boo::FloatSize &corner)
    {
      return (corner.Width() + corner.Height()) / 2.f;
    };
    return gfx::RoundedRect2D {.Rect = ToRect2D(rect.Rect()),
                               .Radii = {.TopLeft = radius(radii.TopLeft()),
                                         .TopRight = radius(radii.TopRight()),
                                         .BottomRight = radius(radii.BottomRight()),
                                         .BottomLeft = radius(radii.BottomLeft())}};
  }

  void AddRoundedRect(gfx::Path2D &path, const boo::FloatRoundedRect &rect) noexcept
  {
    const auto &radii = rect.GetRadii();
    auto toVec = [](const boo::FloatSize &size)
    {
      return Vec2f {size.Width(), size.Height()};
    };
    path.AddRoundedRect(ToRect2D(rect.Rect()), {toVec(radii.TopLeft()), toVec(radii.TopRight()),
                                                toVec(radii.BottomRight()), toVec(radii.BottomLeft())});
  }

  namespace
  {
    /// @brief Builds a `Path2D` from a path's segments, as upstream's Skia path does.
    class PathBuilder
    {
    private:
      gfx::Path2D _path;
      boo::FloatPoint _currentPoint;
      boo::FloatPoint _lastMoveToPoint;
      bool _hasCurrentPoint {false};

    public:
      void Add(const boo::PathSegment &segment) noexcept
      {
        std::visit([&](const auto &data) { AddData(data); }, segment.GetData());
        _currentPoint = segment.CalculateEndPoint(_currentPoint, _lastMoveToPoint);
        _hasCurrentPoint = true;
      }

      KRYS_NODISCARD gfx::Path2D Take() noexcept
      {
        return krys::move(_path);
      }

    private:
      void AddData(const boo::PathMoveTo &data) noexcept
      {
        _path.MoveTo(ToVec2f(data.Point));
      }

      void AddData(const boo::PathLineTo &data) noexcept
      {
        _path.LineTo(ToVec2f(data.Point));
      }

      void AddData(const boo::PathQuadCurveTo &data) noexcept
      {
        _path.QuadTo(ToVec2f(data.ControlPoint), ToVec2f(data.EndPoint));
      }

      void AddData(const boo::PathBezierCurveTo &data) noexcept
      {
        _path.CubicTo(ToVec2f(data.ControlPoint1), ToVec2f(data.ControlPoint2), ToVec2f(data.EndPoint));
      }

      void AddData(const boo::PathArcTo &data) noexcept
      {
        AddArcTo(data.ControlPoint1, data.ControlPoint2, data.Radius);
      }

      void AddData(const boo::PathArc &data) noexcept
      {
        AddEllipse(data.Center, data.Radius, data.Radius, 0.f, data.StartAngle, data.EndAngle,
                   data.Direction);
      }

      void AddData(const boo::PathClosedArc &data) noexcept
      {
        AddData(data.Arc);
        _path.Close();
      }

      void AddData(const boo::PathEllipse &data) noexcept
      {
        AddEllipse(data.Center, data.RadiusX, data.RadiusY, data.Rotation, data.StartAngle, data.EndAngle,
                   data.Direction);
      }

      void AddData(const boo::PathEllipseInRect &data) noexcept
      {
        const auto &rect = data.Rect;
        _path.AddEllipse(ToVec2f(rect.Center()), {rect.Width() / 2.f, rect.Height() / 2.f});
      }

      void AddData(const boo::PathRect &data) noexcept
      {
        _path.AddRect(ToRect2D(data.Rect));
      }

      void AddData(const boo::PathRoundedRect &data) noexcept
      {
        AddRoundedRect(_path, data.RoundedRect);
      }

      void AddData(const boo::PathContinuousRoundedRect &data) noexcept
      {
        // Continuous rounded rects are unavailable. Paint a normal rounded rect instead.
        const Vec2f radius {data.CornerWidth, data.CornerHeight};
        _path.AddRoundedRect(ToRect2D(data.Rect), {radius, radius, radius, radius});
      }

      void AddData(const boo::PathDataLine &data) noexcept
      {
        _path.MoveTo(ToVec2f(data.Start()));
        _path.LineTo(ToVec2f(data.End()));
      }

      void AddData(const boo::PathDataQuadCurve &data) noexcept
      {
        _path.MoveTo(ToVec2f(data.Start));
        _path.QuadTo(ToVec2f(data.ControlPoint), ToVec2f(data.EndPoint));
      }

      void AddData(const boo::PathDataBezierCurve &data) noexcept
      {
        _path.MoveTo(ToVec2f(data.Start));
        _path.CubicTo(ToVec2f(data.ControlPoint1), ToVec2f(data.ControlPoint2), ToVec2f(data.EndPoint));
      }

      void AddData(const boo::PathDataArc &data) noexcept
      {
        _path.MoveTo(ToVec2f(data.Start));
        _currentPoint = data.Start;
        _hasCurrentPoint = true;
        AddArcTo(data.ControlPoint1, data.ControlPoint2, data.Radius);
      }

      void AddData(const boo::PathCloseSubpath &) noexcept
      {
        _path.Close();
      }

      /// @brief An arc of @p radius tangent to the line from the current point to @p point1 and to the line
      /// from @p point1 to @p point2, joined to the current point by a line, as the canvas `arcTo` draws.
      void AddArcTo(const boo::FloatPoint &point1, const boo::FloatPoint &point2, float radius) noexcept
      {
        if (!_hasCurrentPoint)
        {
          _path.MoveTo(ToVec2f(point1));
          return;
        }

        const Vec2f start = ToVec2f(_currentPoint);
        const Vec2f corner = ToVec2f(point1);
        const Vec2f end = ToVec2f(point2);
        Vec2f before = corner - start;
        Vec2f after = end - corner;
        const float beforeLength = std::hypot(before.x, before.y);
        const float afterLength = std::hypot(after.x, after.y);
        if (radius <= 0.f || beforeLength == 0.f || afterLength == 0.f)
        {
          _path.LineTo(corner);
          return;
        }
        before /= beforeLength;
        after /= afterLength;

        // The turn's sine and cosine; a straight line has no arc.
        const float cosine = before.x * after.x + before.y * after.y;
        const float sine = before.x * after.y - before.y * after.x;
        if (std::abs(sine) <= 1e-6f)
        {
          _path.LineTo(corner);
          return;
        }

        // The arc's ends are where it touches each line, its centre a radius from both, on the turn's side.
        const float distance = std::abs(radius * (1.f - cosine) / sine);
        const Vec2f tangent0 = corner - before * distance;
        const Vec2f tangent1 = corner + after * distance;
        const Vec2f normal = sine > 0.f ? Vec2f {-before.y, before.x} : Vec2f {before.y, -before.x};
        const Vec2f centre = tangent0 + normal * radius;
        const float startAngle = std::atan2(tangent0.y - centre.y, tangent0.x - centre.x);
        const float endAngle = std::atan2(tangent1.y - centre.y, tangent1.x - centre.x);
        _path.AddArc(centre, {radius, radius}, startAngle, endAngle, sine < 0.f);
      }

      /// @brief An elliptical arc, rotated by @p rotation about its centre, joined to the current point by a
      /// line.
      void AddEllipse(const boo::FloatPoint &centre, float radiusX, float radiusY, float rotation,
                      float startAngle, float endAngle, boo::RotationDirection direction) noexcept
      {
        const bool anticlockwise = direction == boo::RotationDirection::Counterclockwise;
        if (!rotation)
        {
          _path.AddArc(ToVec2f(centre), {radiusX, radiusY}, startAngle, endAngle, anticlockwise);
          return;
        }

        // Made about the origin, then rotated and moved into place.
        gfx::Path2D arc;
        arc.AddArc({}, {radiusX, radiusY}, startAngle, endAngle, anticlockwise);
        const auto transform =
          gfx::Transform2D::Translation(centre.X(), centre.Y()) * gfx::Transform2D::Rotation(rotation);
        auto points = arc.Points();
        size_t index = 0;
        for (const auto verb : arc.Verbs())
        {
          switch (verb)
          {
            case gfx::PathVerb::MoveTo:
              if (_hasCurrentPoint)
              {
                _path.LineTo(transform.Apply(points[index]));
              }
              else
              {
                _path.MoveTo(transform.Apply(points[index]));
              }
              index += 1;
              break;
            case gfx::PathVerb::LineTo:
              _path.LineTo(transform.Apply(points[index]));
              index += 1;
              break;
            case gfx::PathVerb::QuadTo:
              _path.QuadTo(transform.Apply(points[index]), transform.Apply(points[index + 1]));
              index += 2;
              break;
            case gfx::PathVerb::CubicTo:
              _path.CubicTo(transform.Apply(points[index]), transform.Apply(points[index + 1]),
                            transform.Apply(points[index + 2]));
              index += 3;
              break;
            case gfx::PathVerb::Close: _path.Close(); break;
          }
        }
      }
    };
  }

  gfx::Path2D ToPath2D(const boo::Path &path) noexcept
  {
    PathBuilder builder;
    path.ApplySegments([&](const boo::PathSegment &segment) { builder.Add(segment); });
    return builder.Take();
  }

  gfx::Gradient2D ToGradient2D(const boo::Gradient &gradient, float alpha) noexcept
  {
    gfx::Gradient2D result;
    std::visit(
      CreateVisitor([&](const boo::Gradient::LinearData &data)
                    { result.Shape = gfx::LinearGradient2D {ToVec2f(data.Point0), ToVec2f(data.Point1)}; },
                    [&](const boo::Gradient::RadialData &data)
                    {
                      result.Shape = gfx::RadialGradient2D {.Point0 = ToVec2f(data.Point0),
                                                            .Point1 = ToVec2f(data.Point1),
                                                            .StartRadius = data.StartRadius,
                                                            .EndRadius = data.EndRadius,
                                                            .AspectRatio = data.AspectRatio};
                    },
                    [&](const boo::Gradient::ConicData &data)
                    { result.Shape = gfx::ConicGradient2D {ToVec2f(data.Point0), data.AngleRadians}; }),
      gradient.GetData());

    for (const auto &stop : gradient.Stops().Stops())
    {
      const auto [red, green, blue, opacity] = stop.Color.ToColorTypeLossy<ExtendedSRGBA<float>>().Resolved();
      result.Stops.push_back({.Offset = stop.Offset, .Colour = {red, green, blue, opacity * alpha}});
    }
    result.Spread = gradient.SpreadMethod();
    result.Interpolation = gradient.GetColorInterpolationMethod();
    return result;
  }
}
