#include "Krystal.UI/PathStrokerStrategy.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.UI/Render2DConversions.hpp"
#include <algorithm>
#include <limits>

namespace krys::ui
{
  namespace
  {
    // Calls @p function with each line of @p outline's contours, closing each one; curves are split into
    // lines.
    template <typename TFunction>
    void ForEachEdge(const gfx::Path2D &outline, TFunction &&function) noexcept
    {
      constexpr int32 curveSteps = 16;
      auto points = outline.Points();
      size_t index = 0;
      Vec2f current {};
      Vec2f contourStart {};
      bool inContour = false;

      auto closeContour = [&]
      {
        if (inContour && current != contourStart)
        {
          function(current, contourStart);
        }
        current = contourStart;
      };

      for (auto verb : outline.Verbs())
      {
        switch (verb)
        {
          case gfx::PathVerb::MoveTo:
            closeContour();
            current = contourStart = points[index++];
            inContour = true;
            break;
          case gfx::PathVerb::LineTo:
            function(current, points[index]);
            current = points[index++];
            break;
          case gfx::PathVerb::QuadTo:
          {
            auto control = points[index];
            auto end = points[index + 1];
            index += 2;
            auto previous = current;
            for (int32 step = 1; step <= curveSteps; ++step)
            {
              float t = static_cast<float>(step) / curveSteps;
              float u = 1 - t;
              auto next = current * (u * u) + control * (2 * u * t) + end * (t * t);
              function(previous, next);
              previous = next;
            }
            current = end;
            break;
          }
          case gfx::PathVerb::CubicTo:
          {
            auto control1 = points[index];
            auto control2 = points[index + 1];
            auto end = points[index + 2];
            index += 3;
            auto previous = current;
            for (int32 step = 1; step <= curveSteps; ++step)
            {
              float t = static_cast<float>(step) / curveSteps;
              float u = 1 - t;
              auto next = current * (u * u * u) + control1 * (3 * u * u * t) + control2 * (3 * u * t * t)
                          + end * (t * t * t);
              function(previous, next);
              previous = next;
            }
            current = end;
            break;
          }
          case gfx::PathVerb::Close:
            closeContour();
            inContour = false;
            break;
        }
      }
      closeContour();
    }
  }

  gfx::Path2D PathStrokerStrategy::Outline(const boo::Path &path, const boo::PathStrokeStyle &style) noexcept
  {
    gfx::StrokeStyle strokeStyle {.Width = style.Thickness,
                                  .Cap = ToLineCap(style.Cap),
                                  .Join = ToLineJoin(style.Join),
                                  .MiterLimit = style.MiterLimit,
                                  .Dashes = List<float>(style.Dashes.begin(), style.Dashes.end()),
                                  .DashOffset = style.DashOffset};
    return _stroker.Stroke(ToPath2D(path), strokeStyle, gfx::Transform2D::Identity());
  }

  bool PathStrokerStrategy::StrokeContains(const boo::Path &path, const boo::FloatPoint &point,
                                           const boo::PathStrokeStyle &style) noexcept
  {
    // The outline is filled with the nonzero rule: the point is inside when the edges wind around it.
    auto target = ToVec2f(point);
    int32 winding = 0;
    ForEachEdge(Outline(path, style),
                [&](Vec2f from, Vec2f to)
                {
                  auto side = (to.x - from.x) * (target.y - from.y) - (target.x - from.x) * (to.y - from.y);
                  if (from.y <= target.y)
                  {
                    if (to.y > target.y && side > 0)
                    {
                      ++winding;
                    }
                  }
                  else if (to.y <= target.y && side < 0)
                  {
                    --winding;
                  }
                });
    return winding != 0;
  }

  boo::FloatRect PathStrokerStrategy::StrokeBoundingRect(const boo::Path &path,
                                                         const boo::PathStrokeStyle &style) noexcept
  {
    auto outline = Outline(path, style);
    if (outline.IsEmpty())
    {
      return {};
    }

    // The outline is made of polygons, so its points bound it.
    Vec2f minimum {std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    Vec2f maximum {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
    for (auto outlinePoint : outline.Points())
    {
      minimum = {std::min(minimum.x, outlinePoint.x), std::min(minimum.y, outlinePoint.y)};
      maximum = {std::max(maximum.x, outlinePoint.x), std::max(maximum.y, outlinePoint.y)};
    }
    return {minimum.x, minimum.y, maximum.x - minimum.x, maximum.y - minimum.y};
  }
}
