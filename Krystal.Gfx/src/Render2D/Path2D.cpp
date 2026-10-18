#include "Krystal.Gfx/Render2D/Path2D.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace krys::gfx
{
  namespace
  {
    /// @brief How far along the tangents a cubic's control points sit to approximate a quarter of an ellipse.
    constexpr float QuarterArcControl = 0.5522847498f;
  }

  void Path2D::MoveTo(Vec2f point) noexcept
  {
    _verbs.push_back(PathVerb::MoveTo);
    _points.push_back(point);
    _contourStart = point;
    _hasCurrentPoint = true;
  }

  void Path2D::LineTo(Vec2f point) noexcept
  {
    EnsureContour(point);
    _verbs.push_back(PathVerb::LineTo);
    _points.push_back(point);
  }

  void Path2D::QuadTo(Vec2f control, Vec2f end) noexcept
  {
    EnsureContour(control);
    _verbs.push_back(PathVerb::QuadTo);
    _points.push_back(control);
    _points.push_back(end);
  }

  void Path2D::CubicTo(Vec2f control1, Vec2f control2, Vec2f end) noexcept
  {
    EnsureContour(control1);
    _verbs.push_back(PathVerb::CubicTo);
    _points.push_back(control1);
    _points.push_back(control2);
    _points.push_back(end);
  }

  void Path2D::Close() noexcept
  {
    if (!_hasCurrentPoint || _verbs.back() == PathVerb::Close)
    {
      return;
    }
    _verbs.push_back(PathVerb::Close);
    // The next segment continues from the contour's start, in a new contour.
    _hasCurrentPoint = false;
  }

  void Path2D::AddRect(const Rect2D &rect) noexcept
  {
    MoveTo({rect.X, rect.Y});
    LineTo({rect.X + rect.Width, rect.Y});
    LineTo({rect.X + rect.Width, rect.Y + rect.Height});
    LineTo({rect.X, rect.Y + rect.Height});
    Close();
  }

  void Path2D::AddRoundedRect(const Rect2D &rect, const Array<Vec2f, 4> &radii) noexcept
  {
    Array<Vec2f, 4> r {};
    for (size_t i = 0u; i < 4u; ++i)
    {
      r[i] = {std::max(radii[i].x, 0.f), std::max(radii[i].y, 0.f)};
    }

    float scale = 1.f;
    auto fit = [&](float length, float a, float b)
    {
      if (a + b > length)
      {
        scale = std::min(scale, length / (a + b));
      }
    };
    fit(rect.Width, r[0].x, r[1].x);
    fit(rect.Width, r[3].x, r[2].x);
    fit(rect.Height, r[0].y, r[3].y);
    fit(rect.Height, r[1].y, r[2].y);
    for (auto &radius : r)
    {
      radius = {radius.x * scale, radius.y * scale};
    }

    const float left = rect.X;
    const float top = rect.Y;
    const float right = rect.X + rect.Width;
    const float bottom = rect.Y + rect.Height;
    const float k = QuarterArcControl;

    MoveTo({left + r[0].x, top});
    LineTo({right - r[1].x, top});
    if (r[1].x > 0.f && r[1].y > 0.f)
    {
      CubicTo({right - r[1].x * (1.f - k), top}, {right, top + r[1].y * (1.f - k)}, {right, top + r[1].y});
    }
    LineTo({right, bottom - r[2].y});
    if (r[2].x > 0.f && r[2].y > 0.f)
    {
      CubicTo({right, bottom - r[2].y * (1.f - k)}, {right - r[2].x * (1.f - k), bottom},
              {right - r[2].x, bottom});
    }
    LineTo({left + r[3].x, bottom});
    if (r[3].x > 0.f && r[3].y > 0.f)
    {
      CubicTo({left + r[3].x * (1.f - k), bottom}, {left, bottom - r[3].y * (1.f - k)},
              {left, bottom - r[3].y});
    }
    LineTo({left, top + r[0].y});
    if (r[0].x > 0.f && r[0].y > 0.f)
    {
      CubicTo({left, top + r[0].y * (1.f - k)}, {left + r[0].x * (1.f - k), top}, {left + r[0].x, top});
    }
    Close();
  }

  void Path2D::AddEllipse(Vec2f centre, Vec2f radii) noexcept
  {
    const float k = QuarterArcControl;
    const float x = centre.x;
    const float y = centre.y;
    const float rx = radii.x;
    const float ry = radii.y;

    MoveTo({x + rx, y});
    CubicTo({x + rx, y + ry * k}, {x + rx * k, y + ry}, {x, y + ry});
    CubicTo({x - rx * k, y + ry}, {x - rx, y + ry * k}, {x - rx, y});
    CubicTo({x - rx, y - ry * k}, {x - rx * k, y - ry}, {x, y - ry});
    CubicTo({x + rx * k, y - ry}, {x + rx, y - ry * k}, {x + rx, y});
    Close();
  }

  void Path2D::AddArc(Vec2f centre, Vec2f radii, float startAngle, float endAngle,
                      bool anticlockwise) noexcept
  {
    constexpr float fullTurn = 2.f * std::numbers::pi_v<float>;

    // The sweep runs from the start angle in the arc's direction; a full turn or more is the whole ellipse.
    float sweep = 0.f;
    if (!anticlockwise)
    {
      const float delta = endAngle - startAngle;
      sweep = delta >= fullTurn ? fullTurn : std::fmod(delta, fullTurn);
      if (sweep < 0.f)
      {
        sweep += fullTurn;
      }
    }
    else
    {
      const float delta = startAngle - endAngle;
      sweep = delta >= fullTurn ? -fullTurn : -std::fmod(delta, fullTurn);
      if (sweep > 0.f)
      {
        sweep -= fullTurn;
      }
    }

    auto pointAt = [&](float angle) -> Vec2f
    {
      return {centre.x + radii.x * std::cos(angle), centre.y + radii.y * std::sin(angle)};
    };

    const Vec2f start = pointAt(startAngle);
    if (_hasCurrentPoint)
    {
      LineTo(start);
    }
    else
    {
      MoveTo(start);
    }

    // One cubic per quarter turn or less.
    const int segments = std::max(1, static_cast<int>(std::ceil(std::abs(sweep) / (fullTurn / 4.f) - 1e-4f)));
    const float step = sweep / static_cast<float>(segments);
    const float k = 4.f / 3.f * std::tan(step / 4.f);
    float angle = startAngle;
    for (int i = 0; i < segments; ++i)
    {
      const float next = angle + step;
      const Vec2f from = pointAt(angle);
      const Vec2f to = pointAt(next);
      const Vec2f control1 {from.x - k * radii.x * std::sin(angle), from.y + k * radii.y * std::cos(angle)};
      const Vec2f control2 {to.x + k * radii.x * std::sin(next), to.y - k * radii.y * std::cos(next)};
      CubicTo(control1, control2, to);
      angle = next;
    }
  }

  void Path2D::Clear() noexcept
  {
    _verbs.clear();
    _points.clear();
    _contourStart = {};
    _hasCurrentPoint = false;
  }

  bool Path2D::operator==(const Path2D &other) const noexcept
  {
    if (_verbs != other._verbs || _points.size() != other._points.size())
    {
      return false;
    }
    for (size_t i = 0u; i < _points.size(); ++i)
    {
      if (_points[i].x != other._points[i].x || _points[i].y != other._points[i].y)
      {
        return false;
      }
    }
    return true;
  }

  void Path2D::EnsureContour(Vec2f point) noexcept
  {
    if (!_hasCurrentPoint)
    {
      // After a Close, a segment without a MoveTo continues from the closed contour's start.
      MoveTo(_verbs.empty() ? point : _contourStart);
    }
  }
}
