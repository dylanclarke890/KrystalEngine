#include "Krystal.Gfx/Render2D/PathRasteriser.hpp"
#include "Krystal.Gfx/Render2D/Flattening.hpp"
#include <algorithm>
#include <cmath>

namespace krys::gfx
{
  namespace
  {
    KRYS_NODISCARD bool IsFinite(Vec2f point) noexcept
    {
      return std::isfinite(point.x) && std::isfinite(point.y);
    }
  }

  CoverageMask PathRasteriser::Rasterise(const Path2D &path, const Transform2D &transform, FillRule rule,
                                         const PixelRect2D &clip) noexcept
  {
    _edges.clear();
    if (clip.IsEmpty() || !BuildEdges(path, transform) || _edges.empty())
    {
      return {};
    }

    // The mask covers the pixels the edges touch, within the clip.
    float minX = _edges[0].X0;
    float maxX = _edges[0].X0;
    float minY = _edges[0].Y0;
    float maxY = _edges[0].Y1;
    for (const auto &edge : _edges)
    {
      minX = std::min({minX, edge.X0, edge.X1});
      maxX = std::max({maxX, edge.X0, edge.X1});
      minY = std::min(minY, edge.Y0);
      maxY = std::max(maxY, edge.Y1);
    }
    const auto left = std::max(static_cast<int32>(std::floor(minX)), clip.X);
    const auto top = std::max(static_cast<int32>(std::floor(minY)), clip.Y);
    const auto right = std::min(static_cast<int32>(std::ceil(maxX)), clip.X + static_cast<int32>(clip.Width));
    const auto bottom =
      std::min(static_cast<int32>(std::ceil(maxY)), clip.Y + static_cast<int32>(clip.Height));
    if (right <= left || bottom <= top)
    {
      return {};
    }

    CoverageMask mask {.Bounds = {.X = left,
                                  .Y = top,
                                  .Width = static_cast<uint32>(right - left),
                                  .Height = static_cast<uint32>(bottom - top)},
                       .Coverage = {}};
    mask.Coverage.assign(static_cast<size_t>(mask.Bounds.Width) * mask.Bounds.Height, uint8 {0});
    _row.assign(mask.Bounds.Width, 0.f);

    std::sort(_edges.begin(), _edges.end(), [](const Edge &a, const Edge &b) { return a.Y0 < b.Y0; });
    _active.clear();
    size_t nextEdge = 0u;

    constexpr float weight = 1.f / static_cast<float>(SubScanlines);
    for (int32 y = top; y < bottom; ++y)
    {
      const auto rowTop = static_cast<float>(y);
      const auto rowBottom = rowTop + 1.f;

      // Edges join the active list as the rows reach them and leave once the rows have passed them.
      while (nextEdge < _edges.size() && _edges[nextEdge].Y0 < rowBottom)
      {
        _active.push_back(static_cast<uint32>(nextEdge++));
      }
      std::erase_if(_active, [&](uint32 index) { return _edges[index].Y1 <= rowTop; });
      if (_active.empty())
      {
        continue;
      }

      for (uint32 sub = 0u; sub < SubScanlines; ++sub)
      {
        const float sampleY = rowTop + (static_cast<float>(sub) + 0.5f) * weight;

        _crossings.clear();
        for (const auto index : _active)
        {
          const auto &edge = _edges[index];
          // Half-open, so a vertex shared by two edges counts once.
          if (sampleY >= edge.Y0 && sampleY < edge.Y1)
          {
            const float t = (sampleY - edge.Y0) / (edge.Y1 - edge.Y0);
            _crossings.push_back({.X = edge.X0 + (edge.X1 - edge.X0) * t, .Winding = edge.Winding});
          }
        }
        if (_crossings.size() < 2u)
        {
          continue;
        }
        std::sort(_crossings.begin(), _crossings.end(),
                  [](const Crossing &a, const Crossing &b) { return a.X < b.X; });

        int32 winding = 0;
        for (size_t i = 0u; i + 1u < _crossings.size(); ++i)
        {
          winding += _crossings[i].Winding;
          const bool inside = rule == FillRule::NonZero ? winding != 0 : (winding & 1) != 0;
          if (inside)
          {
            AccumulateSpan(_crossings[i].X, _crossings[i + 1u].X, weight, left, right);
          }
        }
      }

      auto *coverage = mask.Coverage.data() + static_cast<size_t>(y - top) * mask.Bounds.Width;
      for (uint32 x = 0u; x < mask.Bounds.Width; ++x)
      {
        coverage[x] = static_cast<uint8>(std::clamp(_row[x], 0.f, 1.f) * 255.f + 0.5f);
        _row[x] = 0.f;
      }
    }
    return mask;
  }

  bool PathRasteriser::BuildEdges(const Path2D &path, const Transform2D &transform) noexcept
  {
    const auto points = path.Points();
    size_t next = 0u;
    Vec2f contourStart {};
    Vec2f current {};
    bool inContour = false;

    auto closeContour = [&]
    {
      if (inContour)
      {
        AddLine(current, contourStart);
      }
      inContour = false;
    };

    for (const auto verb : path.Verbs())
    {
      switch (verb)
      {
        case PathVerb::MoveTo:
        {
          closeContour();
          const auto point = transform.Apply(points[next++]);
          if (!IsFinite(point))
          {
            return false;
          }
          contourStart = point;
          current = point;
          inContour = true;
          break;
        }
        case PathVerb::LineTo:
        {
          const auto point = transform.Apply(points[next++]);
          if (!IsFinite(point))
          {
            return false;
          }
          AddLine(current, point);
          current = point;
          break;
        }
        case PathVerb::QuadTo:
        {
          const auto control = transform.Apply(points[next++]);
          const auto end = transform.Apply(points[next++]);
          if (!IsFinite(control) || !IsFinite(end))
          {
            return false;
          }
          AddQuad(current, control, end);
          current = end;
          break;
        }
        case PathVerb::CubicTo:
        {
          const auto control1 = transform.Apply(points[next++]);
          const auto control2 = transform.Apply(points[next++]);
          const auto end = transform.Apply(points[next++]);
          if (!IsFinite(control1) || !IsFinite(control2) || !IsFinite(end))
          {
            return false;
          }
          AddCubic(current, control1, control2, end);
          current = end;
          break;
        }
        case PathVerb::Close:
          closeContour();
          current = contourStart;
          break;
      }
    }
    closeContour();
    return true;
  }

  void PathRasteriser::AddLine(Vec2f from, Vec2f to) noexcept
  {
    if (from.y == to.y)
    {
      // Horizontal lines never cross a sub-scanline.
      return;
    }
    if (from.y < to.y)
    {
      _edges.push_back({.X0 = from.x, .Y0 = from.y, .X1 = to.x, .Y1 = to.y, .Winding = 1});
    }
    else
    {
      _edges.push_back({.X0 = to.x, .Y0 = to.y, .X1 = from.x, .Y1 = from.y, .Winding = -1});
    }
  }

  void PathRasteriser::AddQuad(Vec2f from, Vec2f control, Vec2f to) noexcept
  {
    const int32 segments = detail::QuadSegments(from, control, to, FlatteningTolerance);
    Vec2f previous = from;
    for (int32 i = 1; i <= segments; ++i)
    {
      const Vec2f point =
        detail::QuadPoint(from, control, to, static_cast<float>(i) / static_cast<float>(segments));
      AddLine(previous, point);
      previous = point;
    }
  }

  void PathRasteriser::AddCubic(Vec2f from, Vec2f control1, Vec2f control2, Vec2f to) noexcept
  {
    const int32 segments = detail::CubicSegments(from, control1, control2, to, FlatteningTolerance);
    Vec2f previous = from;
    for (int32 i = 1; i <= segments; ++i)
    {
      const Vec2f point = detail::CubicPoint(from, control1, control2, to,
                                             static_cast<float>(i) / static_cast<float>(segments));
      AddLine(previous, point);
      previous = point;
    }
  }

  void PathRasteriser::AccumulateSpan(float left, float right, float weight, int32 rowLeft,
                                      int32 rowRight) noexcept
  {
    left = std::max(left, static_cast<float>(rowLeft));
    right = std::min(right, static_cast<float>(rowRight));
    if (right <= left)
    {
      return;
    }

    const auto first = static_cast<int32>(std::floor(left));
    const auto last = static_cast<int32>(std::floor(right));
    if (first == last)
    {
      _row[static_cast<size_t>(first - rowLeft)] += (right - left) * weight;
      return;
    }

    _row[static_cast<size_t>(first - rowLeft)] += (static_cast<float>(first + 1) - left) * weight;
    for (int32 x = first + 1; x < last; ++x)
    {
      _row[static_cast<size_t>(x - rowLeft)] += weight;
    }
    // The span can end exactly on the row's right edge, past the last pixel.
    if (last < rowRight)
    {
      _row[static_cast<size_t>(last - rowLeft)] += (right - static_cast<float>(last)) * weight;
    }
  }
}
