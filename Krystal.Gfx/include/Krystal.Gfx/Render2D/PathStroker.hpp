#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Maths/Vector.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Gfx/Render2D/Path2D.hpp"
#include "Krystal.Gfx/Render2D/StrokeStyle.hpp"
#include "Krystal.Gfx/Render2D/Transform2D.hpp"

namespace krys::gfx
{
  /// @brief Turns strokes into outlines to fill.
  ///
  /// The path is flattened into polylines, dashed if the style has dashes, and every piece of the stroke
  /// becomes a polygon of its own: a rect along each line, a join at each corner and a cap at each open end.
  /// The polygons all wind the same way, so filling the outline under `FillRule::NonZero` merges them.
  ///
  /// The outline is in the path's coordinates, to fill with the same transform; the transform only sets
  /// how finely curves, joins and caps are flattened, so that they stay within the rasteriser's tolerance
  /// in target pixels. Curves are also split finely enough that the outer edge of a wide stroke stays
  /// within it.
  class PathStroker
  {
    KRYS_NON_COPYABLE(PathStroker);

  private:
    /// @brief The contour or dash being stroked.
    List<Vec2f> _points {};
    /// @brief Whether each point is a corner between two of the path's segments, which takes the style's
    /// join; points inside flattened curves take bevels.
    List<uint8> _corners {};
    List<Vec2f> _piece {};
    List<uint8> _pieceCorners {};
    List<Vec2f> _polygon {};
    List<float> _dashes {};

    Path2D *_outline {nullptr};
    const StrokeStyle *_style {nullptr};
    float _halfWidth {0.f};
    float _tolerance {0.f};
    float _maxTurn {0.f};

  public:
    PathStroker() noexcept = default;
    PathStroker(PathStroker &&) noexcept = default;
    PathStroker &operator=(PathStroker &&) noexcept = default;
    ~PathStroker() noexcept = default;

    /// @brief The outline of `path` stroked with `style`, to fill under `FillRule::NonZero` with
    /// `transform`. Empty when nothing would show.
    KRYS_NODISCARD Path2D Stroke(const Path2D &path, const StrokeStyle &style,
                                 const Transform2D &transform) noexcept;

  private:
    /// @brief Adds a point to the contour, merging it with the last if they're the same.
    void AddPoint(Vec2f point, bool corner) noexcept;
    /// @brief Strokes the contour in `_points`, dashed or not, then clears it.
    void FinishContour(bool closed, bool drewSegment) noexcept;
    void Dash(bool closed) noexcept;
    /// @brief Strokes an open polyline with caps; a single point is a dot along `direction`.
    void StrokeOpen(Span<const Vec2f> points, Span<const uint8> corners, Vec2f direction) noexcept;
    /// @brief Strokes a closed polyline, with joins all the way round.
    void StrokeClosed(Span<const Vec2f> points, Span<const uint8> corners) noexcept;

    void AddSegment(Vec2f from, Vec2f to) noexcept;
    void AddJoin(Vec2f point, Vec2f incoming, Vec2f outgoing, bool corner) noexcept;
    void AddCap(Vec2f point, Vec2f direction) noexcept;
    void AddDot(Vec2f point, Vec2f direction) noexcept;
    void AddCircle(Vec2f centre) noexcept;
    /// @brief Adds `_polygon` to the outline, reversed if it winds the wrong way.
    void AddPolygon() noexcept;
  };
}
