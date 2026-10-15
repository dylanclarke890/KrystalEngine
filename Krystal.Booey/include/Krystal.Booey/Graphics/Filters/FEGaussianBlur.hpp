#pragma once

#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief The Gaussian blur filter effect.
  /// @note FIXME: Only the kernel and outset calculations are here; the effect itself comes with filter
  /// rendering.
  struct FEGaussianBlur
  {
    /// @brief The size of the box blur kernels that approximate a Gaussian blur of `stdDeviation`.
    KRYS_NODISCARD static IntSize CalculateUnscaledKernelSize(FloatSize stdDeviation) noexcept;

    /// @brief How far a blur of `stdDeviation` spreads past each edge.
    KRYS_NODISCARD static IntSize CalculateOutsetSize(FloatSize stdDeviation) noexcept;
    KRYS_NODISCARD static IntOutsets CalculateOutsets(const FloatSize &stdDeviation) noexcept;
  };
}
