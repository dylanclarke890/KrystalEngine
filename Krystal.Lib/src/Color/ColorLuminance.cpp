#include "Krystal.Lib/Color/ColorLuminance.hpp"
#include "Krystal.Lib/Color/Color.hpp"

namespace Krys
{
  double RelativeLuminance(const Color &color) noexcept
  {
    return color.CallOnUnderlyingType([&](const auto &underlyingColor)
                                      { return RelativeLuminance(underlyingColor); });
  }

  double ContrastRatio(const Color &colorA, const Color &colorB) noexcept
  {
    return colorA.CallOnUnderlyingType(
      [&](const auto &underlyingColorA)
      {
        return colorB.CallOnUnderlyingType([&](const auto &underlyingColorB)
                                           { return ContrastRatio(underlyingColorA, underlyingColorB); });
      });
  }
}
