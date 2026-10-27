#pragma once

#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Platform/PathStrokeStrategy.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include <cmath>

namespace krys::boo::tests
{
  /// @brief A stand-in for the host's stroker: a point is inside a stroke when it's within half the stroke's
  /// thickness of the path (curves flattened, caps, joins and dashes ignored), and a stroke's bounds are the
  /// path's grown by half its thickness. It remembers the last style it was given.
  class TestPathStrokeStrategy final : public PathStrokeStrategy
  {
  public:
    PathStrokeStyle LastStyle;

    KRYS_NODISCARD bool StrokeContains(const Path &path, const FloatPoint &point,
                                       const PathStrokeStyle &style) noexcept override
    {
      LastStyle = style;
      float halfThickness = style.Thickness / 2;
      bool contains = false;
      ForEachLine(path,
                  [&](FloatPoint from, FloatPoint to)
                  {
                    if (DistanceToSegment(point, from, to) <= halfThickness)
                    {
                      contains = true;
                    }
                  });
      return contains;
    }

    KRYS_NODISCARD FloatRect StrokeBoundingRect(const Path &path,
                                                const PathStrokeStyle &style) noexcept override
    {
      LastStyle = style;
      auto rect = path.BoundingRect();
      rect.Inflate(style.Thickness / 2);
      return rect;
    }

  private:
    template <typename TFunction>
    static void ForEachLine(const Path &path, TFunction &&function) noexcept
    {
      constexpr int32 curveSteps = 16;
      FloatPoint current;
      FloatPoint subpathStart;
      path.ApplyElements(
        [&](const PathElement &element)
        {
          const auto &points = element.Points;
          switch (element.ElementType)
          {
            case PathElement::Type::MoveToPoint: current = subpathStart = points[0]; break;
            case PathElement::Type::AddLineToPoint:
              function(current, points[0]);
              current = points[0];
              break;
            case PathElement::Type::AddQuadCurveToPoint:
            {
              auto previous = current;
              for (int32 step = 1; step <= curveSteps; ++step)
              {
                float t = static_cast<float>(step) / curveSteps;
                float u = 1 - t;
                FloatPoint next(u * u * current.X() + 2 * u * t * points[0].X() + t * t * points[1].X(),
                                u * u * current.Y() + 2 * u * t * points[0].Y() + t * t * points[1].Y());
                function(previous, next);
                previous = next;
              }
              current = points[1];
              break;
            }
            case PathElement::Type::AddCurveToPoint:
            {
              auto previous = current;
              for (int32 step = 1; step <= curveSteps; ++step)
              {
                float t = static_cast<float>(step) / curveSteps;
                float u = 1 - t;
                FloatPoint next(u * u * u * current.X() + 3 * u * u * t * points[0].X()
                                  + 3 * u * t * t * points[1].X() + t * t * t * points[2].X(),
                                u * u * u * current.Y() + 3 * u * u * t * points[0].Y()
                                  + 3 * u * t * t * points[1].Y() + t * t * t * points[2].Y());
                function(previous, next);
                previous = next;
              }
              current = points[2];
              break;
            }
            case PathElement::Type::CloseSubpath:
              function(current, subpathStart);
              current = subpathStart;
              break;
          }
        });
    }

    KRYS_NODISCARD static float DistanceToSegment(FloatPoint point, FloatPoint from, FloatPoint to) noexcept
    {
      float dx = to.X() - from.X();
      float dy = to.Y() - from.Y();
      float lengthSquared = dx * dx + dy * dy;
      float t =
        lengthSquared ? ((point.X() - from.X()) * dx + (point.Y() - from.Y()) * dy) / lengthSquared : 0;
      t = std::clamp(t, 0.f, 1.f);
      return std::hypot(point.X() - (from.X() + t * dx), point.Y() - (from.Y() + t * dy));
    }
  };

  /// @brief Installs a @ref TestPathStrokeStrategy as the host's stroker while in scope.
  struct PathStrokeStrategyScope
  {
    TestPathStrokeStrategy Strategy;
    PathStrokeStrategy *Previous {PlatformStrategies::Singleton().GetPathStrokeStrategy()};

    PathStrokeStrategyScope() noexcept
    {
      PlatformStrategies::Singleton().SetPathStrokeStrategy(&Strategy);
    }
    ~PathStrokeStrategyScope() noexcept
    {
      PlatformStrategies::Singleton().SetPathStrokeStrategy(Previous);
    }
    PathStrokeStrategyScope(const PathStrokeStrategyScope &) = delete;
    PathStrokeStrategyScope &operator=(const PathStrokeStrategyScope &) = delete;
  };
}
