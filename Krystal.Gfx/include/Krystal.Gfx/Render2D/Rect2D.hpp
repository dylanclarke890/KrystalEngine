#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

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

  /// @brief A rectangle of whole pixels, with a top-left origin and y pointing down.
  struct PixelRect2D
  {
    int32 X {0};
    int32 Y {0};
    uint32 Width {0u};
    uint32 Height {0u};

    constexpr bool operator==(const PixelRect2D &) const noexcept = default;

    KRYS_NODISCARD constexpr bool IsEmpty() const noexcept
    {
      return Width == 0u || Height == 0u;
    }
  };
}
