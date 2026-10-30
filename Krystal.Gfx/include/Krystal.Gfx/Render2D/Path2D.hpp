#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Maths/Vector.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Gfx/Render2D/Rect2D.hpp"

namespace krys::gfx
{
  /// @brief Which points a path fills when its contours overlap or nest.
  enum class FillRule : uint8
  {
    /// @brief Inside wherever the contours wind around the point a non-zero number of times.
    NonZero,
    /// @brief Inside wherever a ray from the point crosses the contours an odd number of times.
    EvenOdd,
  };

  enum class PathVerb : uint8
  {
    /// @brief Starts a contour at one point.
    MoveTo,
    /// @brief A line to one point.
    LineTo,
    /// @brief A quadratic Bézier: a control point and an end point.
    QuadTo,
    /// @brief A cubic Bézier: two control points and an end point.
    CubicTo,
    /// @brief Closes the contour with a line back to its start; takes no points.
    Close,
  };

  /// @brief A 2D path: contours of lines and quadratic and cubic Béziers, in pixels with y pointing down.
  ///
  /// A segment added without a current point starts a contour at its first point. Filling closes every
  /// contour, whether or not it ends with `Close`.
  class Path2D
  {
  private:
    List<PathVerb> _verbs {};
    List<Vec2f> _points {};
    Vec2f _contourStart {};
    bool _hasCurrentPoint {false};

  public:
    void MoveTo(Vec2f point) noexcept;
    void LineTo(Vec2f point) noexcept;
    void QuadTo(Vec2f control, Vec2f end) noexcept;
    void CubicTo(Vec2f control1, Vec2f control2, Vec2f end) noexcept;
    /// @brief Closes the current contour; the current point moves back to its start.
    void Close() noexcept;

    /// @brief Adds a closed contour around a rect, clockwise on screen.
    void AddRect(const Rect2D &rect) noexcept;
    /// @brief Adds a closed contour around a rect with elliptical corners, clockwise on screen.
    /// @param radii The x and y radius of the top-left, top-right, bottom-right and bottom-left corners.
    /// Negative radii are treated as zero, and radii that don't fit are scaled down together until they do,
    /// as CSS does for `border-radius`.
    void AddRoundedRect(const Rect2D &rect, const Array<Vec2f, 4> &radii) noexcept;
    /// @brief Adds a closed contour around an ellipse, clockwise on screen.
    void AddEllipse(Vec2f centre, Vec2f radii) noexcept;
    /// @brief Adds an elliptical arc from `startAngle` to `endAngle` (radians, from the x axis, increasing
    /// clockwise on screen), as cubics. A line joins the current point to the arc's start, if there is one.
    /// A sweep of a full turn or more draws the whole ellipse.
    void AddArc(Vec2f centre, Vec2f radii, float startAngle, float endAngle,
                bool anticlockwise = false) noexcept;

    void Clear() noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _verbs.empty();
    }

    KRYS_NODISCARD Span<const PathVerb> Verbs() const noexcept
    {
      return _verbs;
    }

    /// @brief Every verb's points, in order.
    KRYS_NODISCARD Span<const Vec2f> Points() const noexcept
    {
      return _points;
    }

    KRYS_NODISCARD bool operator==(const Path2D &other) const noexcept;

  private:
    /// @brief Starts a contour at `point` if there's no current point.
    void EnsureContour(Vec2f point) noexcept;
  };
}
