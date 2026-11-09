/*
 * Copyright (C) 2021-2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Color/ColorInterpolation.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo
{
  Color InterpolateColors(ColourInterpolationMethod colorInterpolationMethod, Color color1,
                          double color1Multiplier, Color color2, double color2Multiplier) noexcept
  {
    return krys::SwitchOn(
      colorInterpolationMethod.ColourSpace,
      [&]<typename MethodColourSpace>(const MethodColourSpace &colorSpace) -> Color
      {
        using ColorType = typename MethodColourSpace::ColourType;
        switch (colorInterpolationMethod.AlphaPremultiplication)
        {
          case AlphaPremultiplication::Premultiplied:
          {
            return InterpolateColourComponents<AlphaPremultiplication::Premultiplied>(
              colorSpace, color1.ToColorTypeLossyCarryingForwardMissing<ColorType>(), color1Multiplier,
              color2.ToColorTypeLossyCarryingForwardMissing<ColorType>(), color2Multiplier);
          }
          case AlphaPremultiplication::Unpremultiplied:
          {
            return InterpolateColourComponents<AlphaPremultiplication::Unpremultiplied>(
              colorSpace, color1.ToColorTypeLossyCarryingForwardMissing<ColorType>(), color1Multiplier,
              color2.ToColorTypeLossyCarryingForwardMissing<ColorType>(), color2Multiplier);
          }
        }

        krys_unreachable();
      });
  }
}