#pragma once

#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::UI
{
  /// @brief Ordered by top, right, bottom, left.
  using EdgeSizes = Array<float, 4>;

  constexpr static size_t TopEdge = 0u;
  constexpr static size_t RightEdge = 1u;
  constexpr static size_t BottomEdge = 2u;
  constexpr static size_t LeftEdge = 3u;

  /// @brief Ordered by top-left, top-right, bottom-right, bottom-left.
  using CornerSizes = Array<float, 4>;
  using CornerSizes2 = Array<Maths::Vec2, 4>;
  using CornerPositions = Array<Maths::Vec2, 4>;

  constexpr static size_t TopLeftCorner = 0u;
  constexpr static size_t TopRightCorner = 1u;
  constexpr static size_t BottomRightCorner = 2u;
  constexpr static size_t BottomLeftCorner = 3u;
}