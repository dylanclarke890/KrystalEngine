#pragma once

#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Maths/Interpolate.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::UI
{
  /// @brief Ordered by top, right, bottom, left.
  using EdgeSizes = Array<float, 4>;

  /// @brief Ordered by top-left, top-right, bottom-right, bottom-left.
  using CornerSizes = Array<float, 4>;
  using CornerSizes2 = Array<Maths::Vec2, 4>;
  using CornerPositions = Array<Maths::Vec2, 4>;

  enum class BoxEdge : uint8
  {
    Top,
    Right,
    Bottom,
    Left
  };

  constexpr auto operator+(BoxEdge c)
  {
    return static_cast<std::underlying_type_t<BoxEdge>>(c);
  }

  enum class BoxCorner : uint8
  {
    TopLeft,
    TopRight,
    BottomRight,
    BottomLeft
  };

  constexpr auto operator+(BoxCorner c)
  {
    return static_cast<std::underlying_type_t<BoxCorner>>(c);
  }

  constexpr int RoundToInteger(float value) noexcept
  {
    if (value > 0.0f)
      return int(value + 0.5f);

    return int(value - 0.5f);
  }

  constexpr Gfx::ColourbPremultiplied RoundedLerp(float t, Gfx::ColourbPremultiplied v0,
                                                  Gfx::ColourbPremultiplied v1) noexcept
  {
    return Gfx::ColourbPremultiplied {
      static_cast<unsigned char>(
        RoundToInteger(Maths::Lerp(t, static_cast<float>(v0[0]), static_cast<float>(v1[0])))),
      static_cast<unsigned char>(
        RoundToInteger(Maths::Lerp(t, static_cast<float>(v0[1]), static_cast<float>(v1[1])))),
      static_cast<unsigned char>(
        RoundToInteger(Maths::Lerp(t, static_cast<float>(v0[2]), static_cast<float>(v1[2])))),
      static_cast<unsigned char>(
        RoundToInteger(Maths::Lerp(t, static_cast<float>(v0[3]), static_cast<float>(v1[3])))),
    };
  }
}