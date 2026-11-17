/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Properties/Consumers/ColorInterpolationMethod.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/Color/ColorInterpolationMethod.hpp"

namespace krys::boo::css::parse_property
{
  // MARK: - color-mix()

  KRYS_NODISCARD static Maybe<HueInterpolationMethod>
    ConsumeHueInterpolationMethod(TokenRange &tokens) noexcept
  {
    auto method = [&]() -> Maybe<HueInterpolationMethod>
    {
      switch (tokens.Peek().ValueId())
      {
        case ValueId::Shorter:
        {
          return HueInterpolationMethod::Shorter;
        }
        case ValueId::Longer:
        {
          return HueInterpolationMethod::Longer;
        }
        case ValueId::Increasing:
        {
          return HueInterpolationMethod::Increasing;
        }
        case ValueId::Decreasing:
        {
          return HueInterpolationMethod::Decreasing;
        }
        default:
        {
          return null;
        }
      }
    }();

    if (method)
    {
      tokens.Discard();
      tokens.DiscardWhitespace();
    }

    return method;
  }

  Maybe<ColourInterpolationMethod> ConsumeColorInterpolationMethod(TokenRange &tokens,
                                                                   PropertyParserState &) noexcept
  {
    // <rectangular-color-space> = srgb | srgb-linear | display-p3 | a98-rgb | prophoto-rgb | rec2020 | lab |
    // oklab | xyz | xyz-d50 | xyz-d65
    // <polar-color-space> = hsl | hwb | lch | oklch
    // <hue-interpolation-method> = [ shorter | longer | increasing | decreasing ] hue
    // <color-interpolation-method> = in [ <rectangular-color-space> | <polar-color-space>
    // <hue-interpolation-method>? ]

    krys_debug_assert(tokens.Peek().ValueId() == ValueId::In);
    tokens.Discard();
    tokens.DiscardWhitespace();

    auto consumePolarColorSpace = [](TokenRange &tokens,
                                     auto colorInterpolationMethod) -> Maybe<ColourInterpolationMethod>
    {
      // Consume the color space identifier.
      tokens.Discard();
      tokens.DiscardWhitespace();

      // <hue-interpolation-method> is optional, so if it is not provided, we just use the default value
      // specified in the passed in 'colorInterpolationMethod' parameter.
      auto hueInterpolationMethod = ConsumeHueInterpolationMethod(tokens);
      if (!hueInterpolationMethod)
      {
        return ColourInterpolationMethod {colorInterpolationMethod, AlphaPremultiplication::Premultiplied};
      }

      // If the hue-interpolation-method was provided it must be followed immediately by the 'hue' identifier.
      if (!ConsumeIdentRaw<ValueId::Hue>(tokens))
      {
        return null;
      }

      colorInterpolationMethod.HueInterpolationMethod = *hueInterpolationMethod;

      return ColourInterpolationMethod {colorInterpolationMethod, AlphaPremultiplication::Premultiplied};
    };

    auto consumeRectangularColorSpace = [](TokenRange &tokens,
                                           auto colorInterpolationMethod) -> Maybe<ColourInterpolationMethod>
    {
      // Consume the color space identifier.
      tokens.Discard();
      tokens.DiscardWhitespace();

      return ColourInterpolationMethod {colorInterpolationMethod, AlphaPremultiplication::Premultiplied};
    };

    switch (tokens.Peek().ValueId())
    {
      case ValueId::Hsl:
      {
        return consumePolarColorSpace(tokens, ColourInterpolationMethod::HSL {});
      }
      case ValueId::Hwb:
      {
        return consumePolarColorSpace(tokens, ColourInterpolationMethod::HWB {});
      }
      case ValueId::Lch:
      {
        return consumePolarColorSpace(tokens, ColourInterpolationMethod::LCH {});
      }
      case ValueId::Lab:
      {
        return consumeRectangularColorSpace(tokens, ColourInterpolationMethod::Lab {});
      }
      case ValueId::Oklch:
      {
        return consumePolarColorSpace(tokens, ColourInterpolationMethod::OKLCH {});
      }
      case ValueId::Oklab:
      {
        return consumeRectangularColorSpace(tokens, ColourInterpolationMethod::OKLab {});
      }
      case ValueId::SRGB:
      {
        return consumeRectangularColorSpace(tokens, ColourInterpolationMethod::SRGB {});
      }
      case ValueId::SrgbLinear:
      {
        return consumeRectangularColorSpace(tokens, ColourInterpolationMethod::SRGBLinear {});
      }
      case ValueId::DisplayP3:
      {
        return consumeRectangularColorSpace(tokens, ColourInterpolationMethod::DisplayP3 {});
      }
      case ValueId::DisplayP3Linear:
      {
        return consumeRectangularColorSpace(tokens, ColourInterpolationMethod::DisplayP3Linear {});
      }
      case ValueId::A98Rgb:
      {
        return consumeRectangularColorSpace(tokens, ColourInterpolationMethod::A98RGB {});
      }
      case ValueId::ProphotoRgb:
      {
        return consumeRectangularColorSpace(tokens, ColourInterpolationMethod::ProPhotoRGB {});
      }
      case ValueId::Rec2020:
      {
        return consumeRectangularColorSpace(tokens, ColourInterpolationMethod::Rec2020 {});
      }
      case ValueId::XyzD50:
      {
        return consumeRectangularColorSpace(tokens, ColourInterpolationMethod::XYZD50 {});
      }
      case ValueId::Xyz:
      case ValueId::XyzD65:
      {
        return consumeRectangularColorSpace(tokens, ColourInterpolationMethod::XYZD65 {});
      }
      default:
      {
        return null;
      }
    }
  }
}
