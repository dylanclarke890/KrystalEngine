#pragma once

#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief The drop shadow filter effect.
  /// @note FIXME: Only the outset calculation is here; the effect itself comes with filter rendering.
  struct FEDropShadow
  {
    /// @brief How far a shadow at `offset`, blurred by `stdDeviation`, spreads past each edge.
    KRYS_NODISCARD static IntOutsets CalculateOutsets(const FloatSize &offset,
                                                      const FloatSize &stdDeviation) noexcept;
  };
}
