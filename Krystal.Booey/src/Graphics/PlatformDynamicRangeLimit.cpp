#include "Krystal.Booey/Graphics/PlatformDynamicRangeLimit.hpp"
#include "Krystal.Booey/Core/MathExtras.hpp"
#include "Krystal.Core/Assert.hpp"
#include <cmath>

namespace krys::boo
{
  PlatformDynamicRangeLimit::ValueType
    PlatformDynamicRangeLimit::NormalizedAverage(float standardPercent, float constrainedPercent,
                                                 float noLimitPercent) noexcept
  {
    if (!std::isfinite(standardPercent) || standardPercent < 0 || standardPercent > 100
        || !std::isfinite(constrainedPercent) || constrainedPercent < 0 || constrainedPercent > 100
        || !std::isfinite(noLimitPercent) || noLimitPercent < 0 || noLimitPercent > 100)
    {
      krys_debug_assert(std::isfinite(standardPercent));
      krys_debug_assert(standardPercent >= 0);
      krys_debug_assert(standardPercent <= 100);
      krys_debug_assert(std::isfinite(constrainedPercent));
      krys_debug_assert(constrainedPercent >= 0);
      krys_debug_assert(constrainedPercent <= 100);
      krys_debug_assert(std::isfinite(noLimitPercent));
      krys_debug_assert(noLimitPercent >= 0);
      krys_debug_assert(noLimitPercent <= 100);
      // Unexpected value -> Clamp HDR down to prevent misuses.
      return StandardValue;
    }

    float sum = standardPercent + constrainedPercent + noLimitPercent;
    if (!sum)
    {
      krys_debug_assert(sum);
      return StandardValue;
    }

    float weightedSum =
      standardPercent * StandardValue + constrainedPercent * ConstrainedValue + noLimitPercent * NoLimitValue;
    float weightedAverage = SafeFPDivision(weightedSum, sum);
    return std::clamp(weightedAverage, 0.0f, 1.0f);
  }
}
