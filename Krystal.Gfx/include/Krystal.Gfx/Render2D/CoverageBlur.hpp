#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Gfx/Render2D/PathRasteriser.hpp"

/// @brief Gaussian blurs of coverage masks, for shadows.
namespace krys::gfx::detail
{
  /// @brief From this standard deviation, blurs are three box blurs; below it, a Gaussian kernel, whose cost
  /// grows with the blur.
  constexpr float BoxBlurSigma = 8.f;

  /// @brief How many pixels a blur with standard deviation `sigma` spreads coverage on each side.
  KRYS_NODISCARD uint32 BlurExtent(float sigma) noexcept;

  /// @brief The mask blurred by a Gaussian with standard deviation `sigma` pixels, its bounds grown by
  /// `BlurExtent(sigma)`. From `BoxBlurSigma`, three box blurs stand in for the Gaussian, sized as SVG's
  /// `feGaussianBlur` sizes them (within a few percent of the Gaussian); below it, each pixel is weighted by
  /// the Gaussian's mass over it, out to three standard deviations, which is exact for coverage that's even
  /// across each pixel. Without a blur, the mask is returned as it is.
  KRYS_NODISCARD CoverageMask BlurCoverage(const CoverageMask &mask, float sigma) noexcept;
}
