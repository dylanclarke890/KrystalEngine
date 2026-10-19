#pragma once

#include "Krystal.Booey/Graphics/PathElement.hpp"
#include "Krystal.Booey/Graphics/PathImpl.hpp"
#include "Krystal.Booey/Graphics/PathSegment.hpp"
#include "Krystal.Booey/Graphics/WindRule.hpp"
#include "Krystal.Core/Types/DataRef.hpp"
#include <variant>

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class GraphicsContext;
  class LayoutRoundedRect;
  class PathTraversalState;

  /// @brief A path made of line, curve, arc and shape segments.
  /// FIXME: Stroke hit testing (`strokeContains`), stroke bounds (`strokeBoundingRect`) and the platform path
  /// (`platformPath`, used where a segment can't produce elements or be transformed, and by `addPath`) come
  /// with the graphics backend.
  class Path
  {
  public:
    Path() noexcept = default;
    Path(PathSegment &&) noexcept;
    Path(List<PathSegment> &&) noexcept;
    explicit Path(const List<FloatPoint> &points) noexcept;
    Path(Ref<PathImpl> &&) noexcept;

    Path(const Path &) noexcept = default;
    Path(Path &&) noexcept = default;
    Path &operator=(const Path &) = default;
    Path &operator=(Path &&) = default;

    KRYS_NODISCARD bool DefinitelyEqual(const Path &) const noexcept;

    void MoveTo(const FloatPoint &) noexcept;
    void AddLineTo(const FloatPoint &) noexcept;
    void AddQuadCurveTo(const FloatPoint &controlPoint, const FloatPoint &endPoint) noexcept;
    void AddBezierCurveTo(const FloatPoint &controlPoint1, const FloatPoint &controlPoint2,
                          const FloatPoint &endPoint) noexcept;
    void AddArcTo(const FloatPoint &point1, const FloatPoint &point2, float radius) noexcept;
    void AddArc(const FloatPoint &, float radius, float startAngle, float endAngle,
                RotationDirection) noexcept;
    void AddEllipse(const FloatPoint &, float radiusX, float radiusY, float rotation, float startAngle,
                    float endAngle, RotationDirection) noexcept;
    void AddEllipseInRect(const FloatRect &) noexcept;
    void AddRect(const FloatRect &) noexcept;
    void AddRoundedRect(const FloatRoundedRect &,
                        PathRoundedRect::Strategy = PathRoundedRect::Strategy::PreferNative) noexcept;
    void AddRoundedRect(const FloatRect &, const FloatSize &roundingRadii,
                        PathRoundedRect::Strategy = PathRoundedRect::Strategy::PreferNative) noexcept;
    void AddContinuousRoundedRect(const FloatRect &, float cornerRadius) noexcept;
    void AddContinuousRoundedRect(const FloatRect &, float cornerWidth, float cornerHeight) noexcept;
    void AddRoundedRect(const LayoutRoundedRect &) noexcept;
    void CloseSubpath() noexcept;

    void AddPath(const Path &, const AffineTransform &) noexcept;

    void ApplySegments(const PathSegmentApplier &) const noexcept;
    void ApplyElements(const PathElementApplier &) const noexcept;

    /// @brief Whether @p point is inside the path, filled with @p rule; curves are flattened into lines.
    KRYS_NODISCARD bool Contains(const FloatPoint &point, WindRule rule = WindRule::NonZero) const noexcept;
    void Clear() noexcept;

    void Translate(const FloatSize &delta) noexcept;
    void Transform(const AffineTransform &) noexcept;

    KRYS_NODISCARD constexpr static float CircleControlPoint() noexcept
    {
      return PathImpl::CircleControlPoint();
    }

    KRYS_NODISCARD Maybe<PathSegment> SingleSegment() const noexcept;
    KRYS_NODISCARD Maybe<PathDataLine> SingleDataLine() const noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept;
    KRYS_NODISCARD bool DefinitelySingleLine() const noexcept;

    KRYS_NODISCARD const PathSegment *SingleSegmentIfExists() const noexcept
    {
      return AsSingle();
    }
    KRYS_NODISCARD const List<PathSegment> *SegmentsIfExists() const noexcept;
    KRYS_NODISCARD List<PathSegment> Segments() const noexcept;

    KRYS_NODISCARD float Length() const noexcept;
    KRYS_NODISCARD bool IsClosed() const noexcept;
    KRYS_NODISCARD bool HasSubpaths() const noexcept;
    KRYS_NODISCARD FloatPoint CurrentPoint() const noexcept;
    KRYS_NODISCARD PathTraversalState TraversalStateAtLength(float length) const noexcept;
    KRYS_NODISCARD FloatPoint PointAtLength(float length) const noexcept;

    KRYS_NODISCARD FloatRect FastBoundingRect() const noexcept;
    KRYS_NODISCARD FloatRect BoundingRect() const noexcept;

  private:
    PathImpl &SetImpl(Ref<PathImpl> &&) noexcept;
    PathImpl &EnsureImpl() noexcept;

    KRYS_NODISCARD PathSegment *AsSingle() noexcept
    {
      return std::get_if<PathSegment>(&_data);
    }
    KRYS_NODISCARD const PathSegment *AsSingle() const noexcept
    {
      return std::get_if<PathSegment>(&_data);
    }

    KRYS_NODISCARD PathImpl *AsImpl() noexcept;
    KRYS_NODISCARD const PathImpl *AsImpl() const noexcept;

    KRYS_NODISCARD Maybe<FloatPoint> InitialMoveToPoint() const noexcept;

    Variant<std::monostate, PathSegment, DataRef<PathImpl>> _data;
  };

  inline Path::Path(PathSegment &&segment) noexcept : _data(krys::move(segment))
  {
  }

  inline bool Path::IsEmpty() const noexcept
  {
    return std::holds_alternative<std::monostate>(_data);
  }

  inline void Path::MoveTo(const FloatPoint &point) noexcept
  {
    if (IsEmpty())
    {
      _data = PathSegment(PathMoveTo {point});
    }
    else
    {
      EnsureImpl().Add(PathMoveTo {point});
    }
  }

  inline void Path::AddLineTo(const FloatPoint &point) noexcept
  {
    if (auto initial = InitialMoveToPoint())
    {
      _data = PathSegment(PathDataLine {*initial, point});
    }
    else
    {
      EnsureImpl().Add(PathLineTo {point});
    }
  }

  inline void Path::AddQuadCurveTo(const FloatPoint &controlPoint, const FloatPoint &endPoint) noexcept
  {
    if (auto initial = InitialMoveToPoint())
    {
      _data = PathSegment(PathDataQuadCurve {*initial, controlPoint, endPoint});
    }
    else
    {
      EnsureImpl().Add(PathQuadCurveTo {controlPoint, endPoint});
    }
  }

  inline void Path::AddBezierCurveTo(const FloatPoint &controlPoint1, const FloatPoint &controlPoint2,
                                     const FloatPoint &endPoint) noexcept
  {
    if (auto initial = InitialMoveToPoint())
    {
      _data = PathSegment(PathDataBezierCurve {*initial, controlPoint1, controlPoint2, endPoint});
    }
    else
    {
      EnsureImpl().Add(PathBezierCurveTo {controlPoint1, controlPoint2, endPoint});
    }
  }

  inline void Path::AddArcTo(const FloatPoint &point1, const FloatPoint &point2, float radius) noexcept
  {
    if (auto initial = InitialMoveToPoint())
    {
      _data = PathSegment(PathDataArc {*initial, point1, point2, radius});
    }
    else
    {
      EnsureImpl().Add(PathArcTo {point1, point2, radius});
    }
  }

  inline void Path::AddArc(const FloatPoint &point, float radius, float startAngle, float endAngle,
                           RotationDirection direction) noexcept
  {
    // Platform arc routines hang or crash when passed a non-finite radius or angle.
    if (!std::isfinite(radius) || !std::isfinite(startAngle) || !std::isfinite(endAngle))
    {
      return;
    }

    if (IsEmpty())
    {
      _data = PathSegment(PathArc {point, radius, startAngle, endAngle, direction});
    }
    else
    {
      EnsureImpl().Add(PathArc {point, radius, startAngle, endAngle, direction});
    }
  }

  inline void Path::AddEllipse(const FloatPoint &point, float radiusX, float radiusY, float rotation,
                               float startAngle, float endAngle, RotationDirection direction) noexcept
  {
    if (IsEmpty())
    {
      _data = PathSegment(PathEllipse {point, radiusX, radiusY, rotation, startAngle, endAngle, direction});
    }
    else
    {
      EnsureImpl().Add(PathEllipse {point, radiusX, radiusY, rotation, startAngle, endAngle, direction});
    }
  }

  inline void Path::AddEllipseInRect(const FloatRect &rect) noexcept
  {
    if (IsEmpty())
    {
      _data = PathSegment(PathEllipseInRect {rect});
    }
    else
    {
      EnsureImpl().Add(PathEllipseInRect {rect});
    }
  }

  inline void Path::AddRect(const FloatRect &rect) noexcept
  {
    if (IsEmpty())
    {
      _data = PathSegment(PathRect {rect});
    }
    else
    {
      EnsureImpl().Add(PathRect {rect});
    }
  }

  inline void Path::CloseSubpath() noexcept
  {
    if (std::holds_alternative<std::monostate>(_data))
    {
      return;
    }
    if (const auto *segment = std::get_if<PathSegment>(&_data))
    {
      const auto *data = &segment->GetData();
      if (auto *arc = std::get_if<PathArc>(data))
      {
        _data = PathSegment(PathClosedArc {*arc});
        return;
      }
      if (std::holds_alternative<PathCloseSubpath>(*data))
      {
        return;
      }
    }
    auto &impl = EnsureImpl();
    if (impl.IsClosed())
    {
      return;
    }
    impl.Add(PathCloseSubpath {});
  }

  inline FloatPoint Path::CurrentPoint() const noexcept
  {
    if (std::holds_alternative<std::monostate>(_data))
    {
      return {0, 0};
    }
    if (const auto *segment = std::get_if<PathSegment>(&_data))
    {
      FloatPoint lastMoveToPoint;
      return segment->CalculateEndPoint({}, lastMoveToPoint);
    }
    return AsImpl()->CurrentPoint();
  }

  inline Maybe<FloatPoint> Path::InitialMoveToPoint() const noexcept
  {
    if (std::holds_alternative<std::monostate>(_data))
    {
      return FloatPoint {0, 0};
    }
    if (const auto *segment = std::get_if<PathSegment>(&_data))
    {
      if (const auto *moveTo = std::get_if<PathMoveTo>(&segment->GetData()))
      {
        return moveTo->Point;
      }
    }
    return std::nullopt;
  }
}

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, const Path &path) noexcept;
}
