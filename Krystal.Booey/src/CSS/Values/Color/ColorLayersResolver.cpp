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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Values/Color/ColorLayersResolver.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo::css
{
  namespace
  {
    struct BlendRGB
    {
      float R;
      float G;
      float B;
    };

    KRYS_NODISCARD float BlendLum(const BlendRGB &c) noexcept
    {
      return 0.3f * c.R + 0.59f * c.G + 0.11f * c.B;
    }

    KRYS_NODISCARD BlendRGB BlendClipColor(BlendRGB c) noexcept
    {
      float l = BlendLum(c);
      float n = std::min({c.R, c.G, c.B});
      float x = std::max({c.R, c.G, c.B});
      auto clip = [&](auto scale)
      {
        c.R = l + (c.R - l) * scale;
        c.G = l + (c.G - l) * scale;
        c.B = l + (c.B - l) * scale;
      };
      if (n < 0.0f)
      {
        clip(l / (l - n));
      }
      if (x > 1.0f)
      {
        clip((1.0f - l) / (x - l));
      }
      return c;
    }

    KRYS_NODISCARD BlendRGB BlendSetLum(const BlendRGB &c, float l) noexcept
    {
      float d = l - BlendLum(c);
      return BlendClipColor({c.R + d, c.G + d, c.B + d});
    }

    KRYS_NODISCARD float BlendSat(const BlendRGB &c) noexcept
    {
      return std::max({c.R, c.G, c.B}) - std::min({c.R, c.G, c.B});
    }

    KRYS_NODISCARD BlendRGB BlendSetSat(const BlendRGB &c, float s) noexcept
    {
      float n = std::min({c.R, c.G, c.B});
      float x = std::max({c.R, c.G, c.B});
      if (x <= n)
      {
        return {0.0f, 0.0f, 0.0f};
      }
      float scale = s / (x - n);
      return {(c.R - n) * scale, (c.G - n) * scale, (c.B - n) * scale};
    }

    /// @see https://drafts.fxtf.org/compositing/#blendingseparable
    KRYS_NODISCARD float BlendSeparable(BlendMode mode, float cb, float cs) noexcept
    {
      auto multiply = [](float a, float b)
      {
        return a * b;
      };
      auto screen = [](float a, float b)
      {
        return a + b - a * b;
      };
      auto hardLight = [&](float backdrop, float source)
      {
        return source <= 0.5f ? multiply(backdrop, 2.0f * source) : screen(backdrop, 2.0f * source - 1.0f);
      };

      switch (mode)
      {
        case BlendMode::Multiply:   return multiply(cb, cs);
        case BlendMode::Screen:     return screen(cb, cs);
        case BlendMode::Overlay:    return hardLight(cs, cb);
        case BlendMode::Darken:     return std::min(cb, cs);
        case BlendMode::Lighten:    return std::max(cb, cs);
        case BlendMode::HardLight:  return hardLight(cb, cs);
        case BlendMode::Difference: return std::abs(cb - cs);
        case BlendMode::Exclusion:  return cb + cs - 2.0f * cb * cs;
        case BlendMode::ColorDodge:
          if (cb == 0.0f)
          {
            return 0.0f;
          }
          return cs >= 1.0f ? 1.0f : std::min(1.0f, cb / (1.0f - cs));
        case BlendMode::ColorBurn:
          if (cb >= 1.0f)
          {
            return 1.0f;
          }
          return cs == 0.0f ? 0.0f : 1.0f - std::min(1.0f, (1.0f - cb) / cs);
        case BlendMode::SoftLight:
        {
          if (cs <= 0.5f)
          {
            return cb - (1.0f - 2.0f * cs) * cb * (1.0f - cb);
          }
          float d = cb <= 0.25f ? ((16.0f * cb - 12.0f) * cb + 4.0f) * cb : std::sqrt(cb);
          return cb + (2.0f * cs - 1.0f) * (d - cb);
        }
        default: return cs;
      }
    }

    /// @see https://drafts.fxtf.org/compositing/#blendingnonseparable
    KRYS_NODISCARD BlendRGB BlendFunction(BlendMode mode, const BlendRGB &cb, const BlendRGB &cs) noexcept
    {
      switch (mode)
      {
        case BlendMode::Hue:        return BlendSetLum(BlendSetSat(cs, BlendSat(cb)), BlendLum(cb));
        case BlendMode::Saturation: return BlendSetLum(BlendSetSat(cb, BlendSat(cs)), BlendLum(cb));
        case BlendMode::Color:      return BlendSetLum(cs, BlendLum(cb));
        case BlendMode::Luminosity: return BlendSetLum(cb, BlendLum(cs));
        default:
          return {BlendSeparable(mode, cb.R, cs.R), BlendSeparable(mode, cb.G, cs.G),
                  BlendSeparable(mode, cb.B, cs.B)};
      }
    }

    /// @brief @p source, blended with @p mode, composited over @p backdrop.
    /// @see https://drafts.fxtf.org/compositing/#blending
    KRYS_NODISCARD boo::Color BlendColorLayer(BlendMode mode, const boo::Color &backdrop,
                                              const boo::Color &source) noexcept
    {
      auto [bR, bG, bB, ab] = backdrop.ToColorTypeLossy<SRGBA<float>>().Resolved();
      auto [sR, sG, sB, as] = source.ToColorTypeLossy<SRGBA<float>>().Resolved();

      float ao = as + ab * (1.0f - as);
      if (ao <= 0.0f)
      {
        return boo::Color(SRGBA<float> {0.0f, 0.0f, 0.0f, 0.0f});
      }

      // Cs = (1 - ab) x Cs + ab x B(Cb, Cs), then Co = as x Cs + ab x Cb x (1 - as).
      auto blended = BlendFunction(mode, {bR, bG, bB}, {sR, sG, sB});
      auto composite = [&](float cb, float cs, float b)
      {
        return (as * ((1.0f - ab) * cs + ab * b) + ab * cb * (1.0f - as)) / ao;
      };
      return boo::Color(SRGBA<float> {composite(bR, sR, blended.R), composite(bG, sG, blended.G),
                                      composite(bB, sB, blended.B), ao});
    }
  }

  boo::Color BlendSourceOver(const ColorLayersResolver &colorLayers) noexcept
  {
    krys_debug_assert(!colorLayers.Colors.empty());

    Maybe<boo::Color> resultingColor;
    for (const auto &color : colorLayers.Colors)
    {
      if (!resultingColor)
      {
        resultingColor = color;
      }
      else if (colorLayers.BlendMode == BlendMode::Normal)
      {
        resultingColor = boo::BlendSourceOver(color, *resultingColor);
      }
      else
      {
        resultingColor = BlendColorLayer(colorLayers.BlendMode, color, *resultingColor);
      }

      // Nothing beneath an opaque colour shows through, unless it's blended with it.
      if (color.IsOpaque() && colorLayers.BlendMode == BlendMode::Normal)
      {
        return *resultingColor;
      }
    }
    return *resultingColor;
  }
}
