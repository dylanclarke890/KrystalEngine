#pragma once

namespace krys::gfx
{
  /// @brief An axis-aligned rectangle in pixels, with a top-left origin and y pointing down.
  struct Rect2D
  {
    float X {0.f};
    float Y {0.f};
    float Width {0.f};
    float Height {0.f};

    constexpr bool operator==(const Rect2D &) const noexcept = default;
  };
}
