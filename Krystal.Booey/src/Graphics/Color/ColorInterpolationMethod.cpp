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

#include "Krystal.Booey/Graphics/Color/ColorInterpolationMethod.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo
{
  void SerialisationForCSS(StringBuilder &builder,
                           ColourInterpolationColourSpace interpolationColorSpace) noexcept
  {
    builder.Append(SerialisationForCSS(interpolationColorSpace));
  }

  void SerialisationForCSS(StringBuilder &builder, HueInterpolationMethod hueInterpolationMethod) noexcept
  {
    switch (hueInterpolationMethod)
    {
      case HueInterpolationMethod::Shorter:
      {
        break;
      }
      case HueInterpolationMethod::Longer:
      {
        builder.Append(u8" longer hue");
        break;
      }
      case HueInterpolationMethod::Increasing:
      {
        builder.Append(u8" increasing hue");
        break;
      }
      case HueInterpolationMethod::Decreasing:
      {
        builder.Append(u8" decreasing hue");
        break;
      }
    }
  }

  void SerialisationForCSS(StringBuilder &builder, const ColourInterpolationMethod &method) noexcept
  {
    krys::SwitchOn(method.ColourSpace,
                   [&]<typename MethodColourSpace>(const MethodColourSpace &type)
                   {
                     SerialisationForCSS(builder, type.InterpolationColourSpace);
                     if constexpr (hasHueInterpolationMethod<MethodColourSpace>)
                     {
                       SerialisationForCSS(builder, type.HueInterpolationMethod);
                     }
                   });
  }

  utf8_string SerialisationForCSS(const ColourInterpolationMethod &method) noexcept
  {
    StringBuilder builder;
    SerialisationForCSS(builder, method);
    return krys::move(builder).ToString();
  }
}
