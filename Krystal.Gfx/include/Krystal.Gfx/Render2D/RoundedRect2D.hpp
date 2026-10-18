#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Gfx/Render2D/Rect2D.hpp"

namespace krys::gfx
{
  /// @brief The radius of each corner of a rounded rectangle, in pixels.
  struct CornerRadii2D
  {
    float TopLeft {0.f};
    float TopRight {0.f};
    float BottomRight {0.f};
    float BottomLeft {0.f};

    constexpr bool operator==(const CornerRadii2D &) const noexcept = default;

    /// @brief The same radius on every corner.
    KRYS_NODISCARD constexpr static CornerRadii2D Uniform(float radius) noexcept
    {
      return {radius, radius, radius, radius};
    }
  };

  /// @brief A rectangle with circular corners.
  struct RoundedRect2D
  {
    Rect2D Rect {};
    CornerRadii2D Radii {};

    constexpr bool operator==(const RoundedRect2D &) const noexcept = default;
  };
}
