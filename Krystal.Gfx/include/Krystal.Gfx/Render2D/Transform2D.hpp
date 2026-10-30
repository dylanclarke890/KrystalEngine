#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Maths/Vector.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <cmath>

namespace krys::gfx
{
  /// @brief A 2D affine transform, laid out as CSS's `matrix(a, b, c, d, e, f)`:
  /// `x' = A x + C y + E` and `y' = B x + D y + F`.
  struct Transform2D
  {
    float A {1.f};
    float B {0.f};
    float C {0.f};
    float D {1.f};
    float E {0.f};
    float F {0.f};

    constexpr bool operator==(const Transform2D &) const noexcept = default;

    KRYS_NODISCARD constexpr static Transform2D Identity() noexcept
    {
      return {};
    }

    KRYS_NODISCARD constexpr static Transform2D Translation(float x, float y) noexcept
    {
      return {.E = x, .F = y};
    }

    KRYS_NODISCARD constexpr static Transform2D Scale(float x, float y) noexcept
    {
      return {.A = x, .D = y};
    }

    /// @brief A rotation by `radians`, clockwise on screen (y points down).
    KRYS_NODISCARD static Transform2D Rotation(float radians) noexcept
    {
      const float cosine = std::cos(radians);
      const float sine = std::sin(radians);
      return {.A = cosine, .B = sine, .C = -sine, .D = cosine};
    }

    /// @brief The transform that applies `other` first, then this one.
    KRYS_NODISCARD constexpr Transform2D operator*(const Transform2D &other) const noexcept
    {
      return {.A = A * other.A + C * other.B,
              .B = B * other.A + D * other.B,
              .C = A * other.C + C * other.D,
              .D = B * other.C + D * other.D,
              .E = A * other.E + C * other.F + E,
              .F = B * other.E + D * other.F + F};
    }

    KRYS_NODISCARD constexpr Vec2f Apply(Vec2f point) const noexcept
    {
      return {A * point.x + C * point.y + E, B * point.x + D * point.y + F};
    }

    KRYS_NODISCARD constexpr float Determinant() const noexcept
    {
      return A * D - B * C;
    }

    /// @brief The transform that undoes this one; empty when this one flattens the plane.
    KRYS_NODISCARD Maybe<Transform2D> Inverse() const noexcept
    {
      const float determinant = Determinant();
      if (determinant == 0.f || !std::isfinite(determinant))
      {
        return {};
      }
      const float scale = 1.f / determinant;
      Transform2D inverse {.A = D * scale, .B = -B * scale, .C = -C * scale, .D = A * scale};
      inverse.E = -(inverse.A * E + inverse.C * F);
      inverse.F = -(inverse.B * E + inverse.D * F);
      return inverse;
    }

    /// @brief Whether the transform maps axis-aligned rects to axis-aligned rects: scales, translations,
    /// flips and quarter turns.
    KRYS_NODISCARD constexpr bool PreservesAxisAlignment() const noexcept
    {
      return (B == 0.f && C == 0.f) || (A == 0.f && D == 0.f);
    }
  };
}
