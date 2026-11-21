/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

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
