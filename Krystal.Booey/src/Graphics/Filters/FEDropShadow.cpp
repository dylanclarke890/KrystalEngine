#include "Krystal.Booey/Graphics/Filters/FEDropShadow.hpp"
#include "Krystal.Booey/Graphics/Filters/FEGaussianBlur.hpp"
#include <algorithm>

namespace krys::boo
{
  IntOutsets FEDropShadow::CalculateOutsets(const FloatSize &offset, const FloatSize &stdDeviation) noexcept
  {
    IntSize outsetSize = FEGaussianBlur::CalculateOutsetSize(stdDeviation);

    int top = std::max<int>(0, static_cast<int>(outsetSize.Height() - offset.Height()));
    int right = std::max<int>(0, static_cast<int>(outsetSize.Width() + offset.Width()));
    int bottom = std::max<int>(0, static_cast<int>(outsetSize.Height() + offset.Height()));
    int left = std::max<int>(0, static_cast<int>(outsetSize.Width() - offset.Width()));

    return {top, right, bottom, left};
  }
}
