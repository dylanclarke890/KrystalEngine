#pragma once

#include "Krystal.Core/Colour/ColourConversion.hpp"

namespace krys
{
  template <typename ColourType>
  KRYS_NODISCARD inline double RelativeLuminance(const ColourType &color) noexcept
  {
    // https://en.wikipedia.org/wiki/Relative_luminance
    return ConvertColour<XYZA<float, WhitePoint::D65>>(color).Resolved().Y;
  }

  KRYS_NODISCARD constexpr double ContrastRatio(double relativeLuminanceA, double relativeLuminanceB) noexcept
  {
    // Uses the WCAG 2.0 definition of contrast ratio.
    // https://www.w3.org/TR/WCAG20/#contrast-ratiodef
    auto lighterLuminance = relativeLuminanceA;
    auto darkerLuminance = relativeLuminanceB;

    if (lighterLuminance < darkerLuminance)
    {
      std::swap(lighterLuminance, darkerLuminance);
    }

    return (lighterLuminance + 0.05) / (darkerLuminance + 0.05);
  }

  template <typename ColourTypeA, typename ColourTypeB>
  KRYS_NODISCARD inline double ContrastRatio(const ColourTypeA &colorA, const ColourTypeB &colorB) noexcept
  {
    return ContrastRatio(RelativeLuminance(colorA), RelativeLuminance(colorB));
  }
}