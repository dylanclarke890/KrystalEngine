/*
 * Copyright (C) 2022-2023 Apple Inc. All rights reserved.
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

/*
 * Copyright (C) 2006 Apple Inc. All rights reserved.
 * Copyright (C) 2012 Rik Cabanier (cabanier@adobe.com)
 * Copyright (C) 2014 Adobe Systems Incorporated. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

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

#include "Krystal.Core/Colour/ColourTextStream.hpp"
#include "Krystal.Core/Strings/TextStream.hpp"

namespace krys
{
  TextStream &operator<<(TextStream &ts, AlphaPremultiplication premultiplication) noexcept
  {
    switch (premultiplication)
    {
      case AlphaPremultiplication::Premultiplied:   ts << "premultiplied"; break;
      case AlphaPremultiplication::Unpremultiplied: ts << "unpremultiplied"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, BlendMode blendMode) noexcept
  {
    return ts << BlendModeName(blendMode);
  }

  TextStream &operator<<(TextStream &ts, GradientSpreadMethod spreadMethod) noexcept
  {
    switch (spreadMethod)
    {
      case GradientSpreadMethod::Pad:     ts << "pad"; break;
      case GradientSpreadMethod::Reflect: ts << "reflect"; break;
      case GradientSpreadMethod::Repeat:  ts << "repeat"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, ColourInterpolationColourSpace interpolationColorSpace) noexcept
  {
    switch (interpolationColorSpace)
    {
      case ColourInterpolationColourSpace::HSL:             ts << "HSL"; break;
      case ColourInterpolationColourSpace::HWB:             ts << "HWB"; break;
      case ColourInterpolationColourSpace::LCH:             ts << "LCH"; break;
      case ColourInterpolationColourSpace::Lab:             ts << "Lab"; break;
      case ColourInterpolationColourSpace::OKLCH:           ts << "OKLCH"; break;
      case ColourInterpolationColourSpace::OKLab:           ts << "OKLab"; break;
      case ColourInterpolationColourSpace::SRGB:            ts << "sRGB"; break;
      case ColourInterpolationColourSpace::SRGBLinear:      ts << "sRGB linear"; break;
      case ColourInterpolationColourSpace::DisplayP3:       ts << "Display P3"; break;
      case ColourInterpolationColourSpace::DisplayP3Linear: ts << "Display P3 Linear"; break;
      case ColourInterpolationColourSpace::A98RGB:          ts << "A98 RGB"; break;
      case ColourInterpolationColourSpace::ProPhotoRGB:     ts << "ProPhoto RGB"; break;
      case ColourInterpolationColourSpace::Rec2020:         ts << "Rec2020"; break;
      case ColourInterpolationColourSpace::XYZD50:          ts << "XYZ D50"; break;
      case ColourInterpolationColourSpace::XYZD65:          ts << "XYZ D65"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, HueInterpolationMethod hueInterpolationMethod) noexcept
  {
    switch (hueInterpolationMethod)
    {
      case HueInterpolationMethod::Shorter:    ts << "shorter"; break;
      case HueInterpolationMethod::Longer:     ts << "longer"; break;
      case HueInterpolationMethod::Increasing: ts << "increasing"; break;
      case HueInterpolationMethod::Decreasing: ts << "decreasing"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, const ColourInterpolationMethod &method) noexcept
  {
    std::visit(
      [&]<typename TColourSpace>(const TColourSpace &type)
      {
        ts << TColourSpace::InterpolationColourSpace;
        if constexpr (hasHueInterpolationMethod<TColourSpace>)
        {
          ts << ' ' << type.HueInterpolationMethod;
        }
        ts << ' ' << method.AlphaPremultiplication;
      },
      method.ColourSpace);
    return ts;
  }
}
