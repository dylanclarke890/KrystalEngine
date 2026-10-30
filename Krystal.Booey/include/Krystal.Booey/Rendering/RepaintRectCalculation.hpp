#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief Whether an SVG renderer's repaint rect may be approximate (`Fast`) or must be exact.
  enum class RepaintRectCalculation : bool
  {
    Fast,
    Accurate
  };
}
