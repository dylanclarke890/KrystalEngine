#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Maths/Vector.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <algorithm>
#include <cmath>

/// @brief Splitting curves into lines, shared by the rasteriser and the stroker.
namespace krys::gfx::detail
{
  /// @brief The most lines a single curve is flattened into.
  constexpr int32 MaxCurveSegments = 1'024;

  KRYS_NODISCARD inline float Length(Vec2f v) noexcept
  {
    return std::sqrt(v.x * v.x + v.y * v.y);
  }

  KRYS_NODISCARD inline Vec2f Lerp(Vec2f a, Vec2f b, float t) noexcept
  {
    return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t};
  }

  /// @brief The number of lines that keep a curve within `tolerance`, given its largest second difference
  /// and the constant from its second derivative's bound.
  KRYS_NODISCARD inline int32 SegmentsFor(float secondDifference, float factor, float tolerance) noexcept
  {
    const float segments = std::ceil(std::sqrt(secondDifference * factor / tolerance));
    if (!std::isfinite(segments))
    {
      return MaxCurveSegments;
    }
    return std::clamp(static_cast<int32>(segments), 1, MaxCurveSegments);
  }

  /// @brief Lines that keep a quadratic within `tolerance`: a line spanning h of the parameter strays at
  /// most |p0 - 2 p1 + p2| h^2 / 4 from the curve.
  KRYS_NODISCARD inline int32 QuadSegments(Vec2f from, Vec2f control, Vec2f to, float tolerance) noexcept
  {
    const float difference = Length({from.x - 2.f * control.x + to.x, from.y - 2.f * control.y + to.y});
    return SegmentsFor(difference, 0.25f, tolerance);
  }

  /// @brief Lines that keep a cubic within `tolerance`: a line spanning h of the parameter strays at most
  /// 3 max|second difference| h^2 / 4 from the curve.
  KRYS_NODISCARD inline int32 CubicSegments(Vec2f from, Vec2f control1, Vec2f control2, Vec2f to,
                                            float tolerance) noexcept
  {
    const float difference =
      std::max(Length({from.x - 2.f * control1.x + control2.x, from.y - 2.f * control1.y + control2.y}),
               Length({control1.x - 2.f * control2.x + to.x, control1.y - 2.f * control2.y + to.y}));
    return SegmentsFor(difference, 0.75f, tolerance);
  }

  KRYS_NODISCARD inline Vec2f QuadPoint(Vec2f from, Vec2f control, Vec2f to, float t) noexcept
  {
    return Lerp(Lerp(from, control, t), Lerp(control, to, t), t);
  }

  KRYS_NODISCARD inline Vec2f CubicPoint(Vec2f from, Vec2f control1, Vec2f control2, Vec2f to,
                                         float t) noexcept
  {
    const Vec2f a = Lerp(from, control1, t);
    const Vec2f b = Lerp(control1, control2, t);
    const Vec2f c = Lerp(control2, to, t);
    return Lerp(Lerp(a, b, t), Lerp(b, c, t), t);
  }
}
