#include "Krystal.Booey/Core/Color/ColorLuminance.hpp"
#include "Krystal.Booey/Core/Color/Color.hpp"

namespace krys::boo
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
