/*
 * Copyright (C) 2021-2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
