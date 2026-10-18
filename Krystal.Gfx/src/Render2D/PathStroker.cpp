#include "Krystal.Gfx/Render2D/PathStroker.hpp"
#include "Krystal.Gfx/Render2D/Dashing.hpp"
#include "Krystal.Gfx/Render2D/Flattening.hpp"
#include "Krystal.Gfx/Render2D/PathRasteriser.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace krys::gfx
{
  namespace
  {
    KRYS_NODISCARD Vec2f Add(Vec2f a, Vec2f b) noexcept
    {
      return {a.x + b.x, a.y + b.y};
    }

    KRYS_NODISCARD Vec2f Subtract(Vec2f a, Vec2f b) noexcept
    {
      return {a.x - b.x, a.y - b.y};
    }

    KRYS_NODISCARD Vec2f Scaled(Vec2f v, float factor) noexcept
    {
      return {v.x * factor, v.y * factor};
    }

    KRYS_NODISCARD float Dot(Vec2f a, Vec2f b) noexcept
    {
      return a.x * b.x + a.y * b.y;
    }

    KRYS_NODISCARD float Cross(Vec2f a, Vec2f b) noexcept
    {
      return a.x * b.y - a.y * b.x;
    }

    /// @brief A quarter turn of `v`.
    KRYS_NODISCARD Vec2f Perpendicular(Vec2f v) noexcept
    {
      return {-v.y, v.x};
    }

    /// @brief The unit vector from `from` to `to`, or (1, 0) when they're the same point.
    KRYS_NODISCARD Vec2f Direction(Vec2f from, Vec2f to) noexcept
    {
      const auto delta = Subtract(to, from);
      const float length = detail::Length(delta);
      return length > 0.f ? Scaled(delta, 1.f / length) : Vec2f {1.f, 0.f};
    }

    KRYS_NODISCARD bool SamePoint(Vec2f a, Vec2f b) noexcept
    {
      return a.x == b.x && a.y == b.y;
    }

    /// @brief The most the transform stretches any length: its largest singular value.
    KRYS_NODISCARD float MaxStretch(const Transform2D &transform) noexcept
    {
      const float sum = transform.A * transform.A + transform.B * transform.B + transform.C * transform.C
                        + transform.D * transform.D;
      const float determinant = transform.Determinant();
      const float root = std::sqrt(std::max(sum * sum - 4.f * determinant * determinant, 0.f));
      return std::sqrt((sum + root) * 0.5f);
    }

    /// @brief How far a curve's control polygon turns, which bounds how far the curve does.
    KRYS_NODISCARD float ControlTurn(Span<const Vec2f> points) noexcept
    {
      float turn = 0.f;
      Vec2f previous {};
      bool hasPrevious = false;
      for (size_t i = 1u; i < points.size(); ++i)
      {
        const auto edge = Subtract(points[i], points[i - 1u]);
        const float length = detail::Length(edge);
        if (length == 0.f)
        {
          continue;
        }
        const auto direction = Scaled(edge, 1.f / length);
        if (hasPrevious)
        {
          turn += std::acos(std::clamp(Dot(previous, direction), -1.f, 1.f));
        }
        previous = direction;
        hasPrevious = true;
      }
      return turn;
    }
  }

  Path2D PathStroker::Stroke(const Path2D &path, const StrokeStyle &style,
                             const Transform2D &transform) noexcept
  {
    Path2D outline;
    const float stretch = MaxStretch(transform);
    if (path.IsEmpty() || !(stretch > 0.f) || !std::isfinite(stretch))
    {
      return outline;
    }

    // A hairline is one target pixel wide, whatever the transform's scale.
    _halfWidth = style.Width > 0.f ? style.Width * 0.5f : 0.5f / std::sqrt(std::abs(transform.Determinant()));
    if (!(_halfWidth > 0.f) || !std::isfinite(_halfWidth))
    {
      return outline;
    }
    _tolerance = PathRasteriser::FlatteningTolerance / stretch;
    // A turn of θ per line puts the outer edge up to half the width times θ² / 8 off the arc it stands for.
    _maxTurn = std::min(std::sqrt(8.f * _tolerance / _halfWidth), std::numbers::pi_v<float> / 4.f);

    detail::PrepareDashes(style, _dashes);

    _outline = &outline;
    _style = &style;
    _points.clear();
    _corners.clear();

    const auto points = path.Points();
    size_t next = 0u;
    Vec2f current {};
    bool drewSegment = false;
    for (const auto verb : path.Verbs())
    {
      switch (verb)
      {
        case PathVerb::MoveTo:
          FinishContour(false, drewSegment);
          current = points[next++];
          AddPoint(current, true);
          drewSegment = false;
          break;
        case PathVerb::LineTo:
          current = points[next++];
          AddPoint(current, true);
          drewSegment = true;
          break;
        case PathVerb::QuadTo:
        {
          const auto control = points[next++];
          const auto end = points[next++];
          const Vec2f controls[3] = {current, control, end};
          const int32 segments =
            std::clamp(std::max(detail::QuadSegments(current, control, end, _tolerance),
                                static_cast<int32>(std::ceil(ControlTurn(controls) / _maxTurn))),
                       1, detail::MaxCurveSegments);
          for (int32 i = 1; i < segments; ++i)
          {
            AddPoint(
              detail::QuadPoint(current, control, end, static_cast<float>(i) / static_cast<float>(segments)),
              false);
          }
          current = end;
          AddPoint(current, true);
          drewSegment = true;
          break;
        }
        case PathVerb::CubicTo:
        {
          const auto control1 = points[next++];
          const auto control2 = points[next++];
          const auto end = points[next++];
          const Vec2f controls[4] = {current, control1, control2, end};
          const int32 segments =
            std::clamp(std::max(detail::CubicSegments(current, control1, control2, end, _tolerance),
                                static_cast<int32>(std::ceil(ControlTurn(controls) / _maxTurn))),
                       1, detail::MaxCurveSegments);
          for (int32 i = 1; i < segments; ++i)
          {
            AddPoint(detail::CubicPoint(current, control1, control2, end,
                                        static_cast<float>(i) / static_cast<float>(segments)),
                     false);
          }
          current = end;
          AddPoint(current, true);
          drewSegment = true;
          break;
        }
        case PathVerb::Close:
          FinishContour(true, drewSegment);
          drewSegment = false;
          break;
      }
    }
    FinishContour(false, drewSegment);

    _outline = nullptr;
    _style = nullptr;
    return outline;
  }

  void PathStroker::AddPoint(Vec2f point, bool corner) noexcept
  {
    if (!_points.empty() && SamePoint(_points.back(), point))
    {
      _corners.back() = _corners.back() || corner;
      return;
    }
    _points.push_back(point);
    _corners.push_back(corner ? 1u : 0u);
  }

  void PathStroker::FinishContour(bool closed, bool drewSegment) noexcept
  {
    if (_points.empty())
    {
      return;
    }

    // A closing line back to the start is the closing segment itself.
    if (closed && _points.size() > 1u && SamePoint(_points.front(), _points.back()))
    {
      _corners.front() = _corners.front() || _corners.back();
      _points.pop_back();
      _corners.pop_back();
    }

    if (_points.size() == 1u)
    {
      // A zero-length contour draws a dot under round and square caps, but a lone move draws nothing.
      if (drewSegment || closed)
      {
        AddDot(_points[0], {1.f, 0.f});
      }
    }
    else if (!_dashes.empty())
    {
      Dash(closed);
    }
    else if (closed)
    {
      StrokeClosed(_points, _corners);
    }
    else
    {
      StrokeOpen(_points, _corners, Direction(_points[0], _points[1]));
    }

    _points.clear();
    _corners.clear();
  }

  void PathStroker::Dash(bool closed) noexcept
  {
    const size_t count = _points.size();
    const size_t segmentCount = closed ? count : count - 1u;

    float length = 0.f;
    for (size_t i = 0u; i < segmentCount; ++i)
    {
      length += detail::Length(Subtract(_points[(i + 1u) % count], _points[i]));
    }
    if (length / detail::PatternLength(_dashes) > static_cast<float>(detail::MaxDashes))
    {
      if (closed)
      {
        StrokeClosed(_points, _corners);
      }
      else
      {
        StrokeOpen(_points, _corners, Direction(_points[0], _points[1]));
      }
      return;
    }

    auto position = detail::StartDashes(_dashes, _style->DashOffset);

    auto addToPiece = [&](Vec2f point, bool corner)
    {
      if (!_piece.empty() && SamePoint(_piece.back(), point))
      {
        return;
      }
      _piece.push_back(point);
      _pieceCorners.push_back(corner ? 1u : 0u);
    };
    auto finishPiece = [&](Vec2f direction)
    {
      if (_piece.size() == 1u)
      {
        AddDot(_piece[0], direction);
      }
      else if (_piece.size() > 1u)
      {
        StrokeOpen(_piece, _pieceCorners, direction);
      }
      _piece.clear();
      _pieceCorners.clear();
    };

    _piece.clear();
    _pieceCorners.clear();
    if (position.On)
    {
      addToPiece(_points[0], false);
    }

    Vec2f direction = Direction(_points[0], _points[1]);
    for (size_t i = 0u; i < segmentCount; ++i)
    {
      const auto from = _points[i];
      const size_t toIndex = (i + 1u) % count;
      const auto to = _points[toIndex];
      const float segmentLength = detail::Length(Subtract(to, from));
      direction = Direction(from, to);

      // Each dash or gap that ends along this segment.
      float along = 0.f;
      while (position.Remaining <= segmentLength - along)
      {
        along += position.Remaining;
        const auto point = Add(from, Scaled(direction, along));
        if (position.On)
        {
          addToPiece(point, false);
          finishPiece(direction);
        }
        else
        {
          _piece.clear();
          _pieceCorners.clear();
          addToPiece(point, false);
        }
        position.Next(_dashes);
      }
      position.Remaining -= segmentLength - along;
      if (position.On)
      {
        addToPiece(to, _corners[toIndex] != 0u);
      }
    }
    if (position.On)
    {
      finishPiece(direction);
    }
  }

  void PathStroker::StrokeOpen(Span<const Vec2f> points, Span<const uint8> corners, Vec2f direction) noexcept
  {
    const size_t count = points.size();
    if (count == 1u)
    {
      AddDot(points[0], direction);
      return;
    }

    for (size_t i = 0u; i + 1u < count; ++i)
    {
      AddSegment(points[i], points[i + 1u]);
    }
    for (size_t i = 1u; i + 1u < count; ++i)
    {
      AddJoin(points[i], Direction(points[i - 1u], points[i]), Direction(points[i], points[i + 1u]),
              corners[i] != 0u);
    }
    AddCap(points[0], Direction(points[1], points[0]));
    AddCap(points[count - 1u], Direction(points[count - 2u], points[count - 1u]));
  }

  void PathStroker::StrokeClosed(Span<const Vec2f> points, Span<const uint8> corners) noexcept
  {
    const size_t count = points.size();
    for (size_t i = 0u; i < count; ++i)
    {
      AddSegment(points[i], points[(i + 1u) % count]);
    }
    for (size_t i = 0u; i < count; ++i)
    {
      const auto &previous = points[(i + count - 1u) % count];
      const auto &next = points[(i + 1u) % count];
      AddJoin(points[i], Direction(previous, points[i]), Direction(points[i], next), corners[i] != 0u);
    }
  }

  void PathStroker::AddSegment(Vec2f from, Vec2f to) noexcept
  {
    const auto normal = Scaled(Perpendicular(Direction(from, to)), _halfWidth);
    _polygon.assign({Add(from, normal), Add(to, normal), Subtract(to, normal), Subtract(from, normal)});
    AddPolygon();
  }

  void PathStroker::AddJoin(Vec2f point, Vec2f incoming, Vec2f outgoing, bool corner) noexcept
  {
    const float cross = Cross(incoming, outgoing);
    const float dot = Dot(incoming, outgoing);
    if (dot > 0.f && std::abs(cross) < 1e-6f)
    {
      // Straight on: the segments meet edge to edge.
      return;
    }

    // Points inside a flattened curve turn so little that a bevel matches the curve's edge.
    const auto join = corner ? _style->Join : LineJoin::Bevel;
    if (join == LineJoin::Round)
    {
      AddCircle(point);
      return;
    }

    // The join fills the gap on the outside of the turn.
    const float side = cross > 0.f ? -1.f : 1.f;
    const auto incomingNormal = Scaled(Perpendicular(incoming), _halfWidth * side);
    const auto outgoingNormal = Scaled(Perpendicular(outgoing), _halfWidth * side);
    const auto a = Add(point, incomingNormal);
    const auto b = Add(point, outgoingNormal);

    if (join == LineJoin::Miter)
    {
      // The miter's length over the width is one over the cosine of half the turn.
      const float cosHalfTurn = std::sqrt(std::max((1.f + dot) * 0.5f, 0.f));
      if (cosHalfTurn > 1e-6f && 1.f / cosHalfTurn <= _style->MiterLimit)
      {
        const auto bisector = Direction({}, Add(incomingNormal, outgoingNormal));
        const auto tip = Add(point, Scaled(bisector, _halfWidth / cosHalfTurn));
        _polygon.assign({point, a, tip, b});
        AddPolygon();
        return;
      }
    }

    _polygon.assign({point, a, b});
    AddPolygon();
  }

  void PathStroker::AddCap(Vec2f point, Vec2f direction) noexcept
  {
    switch (_style->Cap)
    {
      case LineCap::Butt:  break;
      case LineCap::Round: AddCircle(point); break;
      case LineCap::Square:
      {
        const auto normal = Scaled(Perpendicular(direction), _halfWidth);
        const auto end = Add(point, Scaled(direction, _halfWidth));
        _polygon.assign(
          {Add(point, normal), Add(end, normal), Subtract(end, normal), Subtract(point, normal)});
        AddPolygon();
        break;
      }
    }
  }

  void PathStroker::AddDot(Vec2f point, Vec2f direction) noexcept
  {
    switch (_style->Cap)
    {
      case LineCap::Butt:  break;
      case LineCap::Round: AddCircle(point); break;
      case LineCap::Square:
      {
        const auto along = Scaled(direction, _halfWidth);
        const auto normal = Scaled(Perpendicular(direction), _halfWidth);
        _polygon.assign({Add(Subtract(point, along), normal), Add(Add(point, along), normal),
                         Subtract(Add(point, along), normal), Subtract(Subtract(point, along), normal)});
        AddPolygon();
        break;
      }
    }
  }

  void PathStroker::AddCircle(Vec2f centre) noexcept
  {
    // Chords that stay within the tolerance of the circle.
    constexpr float fullTurn = 2.f * std::numbers::pi_v<float>;
    const float step =
      _tolerance < _halfWidth ? 2.f * std::acos(1.f - _tolerance / _halfWidth) : fullTurn / 8.f;
    const auto segments =
      std::clamp(static_cast<int32>(std::ceil(fullTurn / step)), 8, detail::MaxCurveSegments);

    _polygon.clear();
    for (int32 i = 0; i < segments; ++i)
    {
      const float angle = fullTurn * static_cast<float>(i) / static_cast<float>(segments);
      _polygon.push_back({centre.x + _halfWidth * std::cos(angle), centre.y + _halfWidth * std::sin(angle)});
    }
    AddPolygon();
  }

  void PathStroker::AddPolygon() noexcept
  {
    float area = 0.f;
    for (size_t i = 0u; i < _polygon.size(); ++i)
    {
      area += Cross(_polygon[i], _polygon[(i + 1u) % _polygon.size()]);
    }
    if (std::abs(area) < 1e-12f)
    {
      return;
    }
    if (area < 0.f)
    {
      std::reverse(_polygon.begin(), _polygon.end());
    }

    _outline->MoveTo(_polygon[0]);
    for (size_t i = 1u; i < _polygon.size(); ++i)
    {
      _outline->LineTo(_polygon[i]);
    }
    _outline->Close();
  }
}
