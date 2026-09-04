#pragma once

#include "Krystal.Lib/Color/ColorConversion.hpp"

namespace Krys
{
  class Color;

  template <typename ColorType>
  KRYS_NODISCARD inline double RelativeLuminance(const ColorType &color) noexcept
  {
    // https://en.wikipedia.org/wiki/Relative_luminance

    // FIXME: This can be optimized a bit by observing that in some cases the conversion
    // to XYZA<float, WhitePoint::D65> in its entirety is unnecessary to get just the Y
    // component. For instance, for SRGBA<float>, this could be done as:
    //
    //     convertColor<LinearSRGBA<float>>(color) * LinearSRGBA<float>::linearToXYZ.row(1)
    //
    // (for a hypothetical row() function on ColorMatrix). We would probably want to implement
    // this in ColorConversion.h as a sibling function to convertColor which can get a channel
    // of a color in another space in this kind of optimal way.

    return ConvertColor<XYZA<float, WhitePoint::D65>>(color).Resolved().y;
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

  template <typename ColorTypeA, typename ColorTypeB>
  KRYS_NODISCARD inline double ContrastRatio(const ColorTypeA &colorA, const ColorTypeB &colorB) noexcept
  {
    return ContrastRatio(RelativeLuminance(colorA), RelativeLuminance(colorB));
  }

  KRYS_NODISCARD double RelativeLuminance(const Color &) noexcept;
  KRYS_NODISCARD double ContrastRatio(const Color &, const Color &) noexcept;
}