#include "Krystal.Booey/Graphics/Filters/FEGaussianBlur.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD float GaussianKernelFactor() noexcept
    {
      return 3 / 4.f * std::sqrt(2 * std::numbers::pi_v<float>);
    }

    KRYS_NODISCARD int ClampedToKernelSize(float value) noexcept
    {
      constexpr unsigned MaxKernelSize = 500;

      // Limit the kernel size to 500. A bigger radius won't make a big difference for the result image but
      // inflates the absolute paint rect too much. This is compatible with Firefox' behavior.
      unsigned size =
        std::max<unsigned>(2, static_cast<unsigned>(std::floor(value * GaussianKernelFactor() + 0.5f)));
      return ClampTo<int>(std::min(size, MaxKernelSize));
    }
  }

  IntSize FEGaussianBlur::CalculateUnscaledKernelSize(FloatSize stdDeviation) noexcept
  {
    krys_debug_assert(stdDeviation.Width() >= 0 && stdDeviation.Height() >= 0);
    IntSize kernelSize;

    if (stdDeviation.Width())
    {
      kernelSize.SetWidth(ClampedToKernelSize(stdDeviation.Width()));
    }

    if (stdDeviation.Height())
    {
      kernelSize.SetHeight(ClampedToKernelSize(stdDeviation.Height()));
    }

    return kernelSize;
  }

  IntSize FEGaussianBlur::CalculateOutsetSize(FloatSize stdDeviation) noexcept
  {
    auto kernelSize = CalculateUnscaledKernelSize(stdDeviation);

    // We take the half kernel size and multiply it with three, because we run box blur three times.
    return {3 * kernelSize.Width() / 2, 3 * kernelSize.Height() / 2};
  }

  IntOutsets FEGaussianBlur::CalculateOutsets(const FloatSize &stdDeviation) noexcept
  {
    IntSize outsetSize = CalculateOutsetSize(stdDeviation);
    return {outsetSize.Height(), outsetSize.Width(), outsetSize.Height(), outsetSize.Width()};
  }
}
