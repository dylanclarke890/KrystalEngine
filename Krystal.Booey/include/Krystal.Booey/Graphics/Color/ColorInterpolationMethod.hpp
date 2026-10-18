#pragma once

#include "Krystal.Core/Colour/ColourInterpolationMethod.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  KRYS_NODISCARD constexpr utf8_stringview
    SerialisationForCSS(ColourInterpolationColourSpace interpolationColorSpace) noexcept
  {
    switch (interpolationColorSpace)
    {
      case ColourInterpolationColourSpace::HSL:             return u8"hsl";
      case ColourInterpolationColourSpace::HWB:             return u8"hwb";
      case ColourInterpolationColourSpace::LCH:             return u8"lch";
      case ColourInterpolationColourSpace::Lab:             return u8"lab";
      case ColourInterpolationColourSpace::OKLCH:           return u8"oklch";
      case ColourInterpolationColourSpace::OKLab:           return u8"oklab";
      case ColourInterpolationColourSpace::SRGB:            return u8"srgb";
      case ColourInterpolationColourSpace::SRGBLinear:      return u8"srgb-linear";
      case ColourInterpolationColourSpace::DisplayP3:       return u8"display-p3";
      case ColourInterpolationColourSpace::DisplayP3Linear: return u8"display-p3-linear";
      case ColourInterpolationColourSpace::A98RGB:          return u8"a98-rgb";
      case ColourInterpolationColourSpace::ProPhotoRGB:     return u8"prophoto-rgb";
      case ColourInterpolationColourSpace::Rec2020:         return u8"rec2020";
      case ColourInterpolationColourSpace::XYZD50:          return u8"xyz-d50";
      case ColourInterpolationColourSpace::XYZD65:          return u8"xyz-d65";
    }

    krys_unreachable();
  }

  void SerialisationForCSS(StringBuilder &builder,
                           ColourInterpolationColourSpace interpolationColorSpace) noexcept;

  void SerialisationForCSS(StringBuilder &builder, HueInterpolationMethod hueInterpolationMethod) noexcept;

  void SerialisationForCSS(StringBuilder &builder, const ColourInterpolationMethod &method) noexcept;

  KRYS_NODISCARD utf8_string SerialisationForCSS(const ColourInterpolationMethod &method) noexcept;
}
