/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/TypedOM/Color/CSSOKLCH.hpp"

namespace krys::boo::css
{
  dom::ExceptionOr<Ref<CSSOKLCH>> CSSOKLCH::Create(CSSColorPercent lightness, CSSColorPercent chroma,
                                                   CSSColorAngle hue, CSSColorPercent alpha) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssoklch-cssoklch
    auto rectifiedLightness = RectifyCSSColorPercent(krys::move(lightness));
    if (rectifiedLightness.HasException())
    {
      return rectifiedLightness.ReleaseException();
    }
    auto rectifiedChroma = RectifyCSSColorPercent(krys::move(chroma));
    if (rectifiedChroma.HasException())
    {
      return rectifiedChroma.ReleaseException();
    }
    auto rectifiedHue = RectifyCSSColorAngle(krys::move(hue));
    if (rectifiedHue.HasException())
    {
      return rectifiedHue.ReleaseException();
    }
    auto rectifiedAlpha = RectifyCSSColorPercent(krys::move(alpha));
    if (rectifiedAlpha.HasException())
    {
      return rectifiedAlpha.ReleaseException();
    }
    return AdoptRef(*new CSSOKLCH(rectifiedLightness.ReleaseValue(), rectifiedChroma.ReleaseValue(),
                                  rectifiedHue.ReleaseValue(), rectifiedAlpha.ReleaseValue()));
  }

  CSSOKLCH::CSSOKLCH(RectifiedCSSColorComponent lightness, RectifiedCSSColorComponent chroma,
                     RectifiedCSSColorComponent hue, RectifiedCSSColorComponent alpha) noexcept
      : _lightness(krys::move(lightness)), _chroma(krys::move(chroma)), _hue(krys::move(hue)),
        _alpha(krys::move(alpha))
  {
  }

  CSSColorPercent CSSOKLCH::L() const noexcept
  {
    return ToCSSColorComponent(_lightness);
  }

  dom::ExceptionOr<void> CSSOKLCH::SetL(CSSColorPercent lightness) noexcept
  {
    auto rectified = RectifyCSSColorPercent(krys::move(lightness));
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _lightness = rectified.ReleaseValue();
    return {};
  }

  CSSColorPercent CSSOKLCH::C() const noexcept
  {
    return ToCSSColorComponent(_chroma);
  }

  dom::ExceptionOr<void> CSSOKLCH::SetC(CSSColorPercent chroma) noexcept
  {
    auto rectified = RectifyCSSColorPercent(krys::move(chroma));
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _chroma = rectified.ReleaseValue();
    return {};
  }

  CSSColorAngle CSSOKLCH::H() const noexcept
  {
    return ToCSSColorComponent(_hue);
  }

  dom::ExceptionOr<void> CSSOKLCH::SetH(CSSColorAngle hue) noexcept
  {
    auto rectified = RectifyCSSColorAngle(krys::move(hue));
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _hue = rectified.ReleaseValue();
    return {};
  }

  CSSColorPercent CSSOKLCH::Alpha() const noexcept
  {
    return ToCSSColorComponent(_alpha);
  }

  dom::ExceptionOr<void> CSSOKLCH::SetAlpha(CSSColorPercent alpha) noexcept
  {
    auto rectified = RectifyCSSColorPercent(krys::move(alpha));
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _alpha = rectified.ReleaseValue();
    return {};
  }

  Ref<CSSKeywordValue> CSSOKLCH::ColorSpace() const noexcept
  {
    return ColorSpaceKeyword(u8"oklch");
  }
}
