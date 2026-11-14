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

#include "Krystal.Booey/CSS/TypedOM/Color/CSSHSL.hpp"

namespace krys::boo::css
{
  dom::ExceptionOr<Ref<CSSHSL>> CSSHSL::Create(CSSColorAngle hue, CSSColorPercent saturation,
                                               CSSColorPercent lightness, CSSColorPercent alpha) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-csshsl-csshsl
    auto rectifiedHue = RectifyCSSColorAngle(krys::move(hue));
    if (rectifiedHue.HasException())
    {
      return rectifiedHue.ReleaseException();
    }
    auto rectifiedSaturation = RectifyCSSColorPercent(krys::move(saturation));
    if (rectifiedSaturation.HasException())
    {
      return rectifiedSaturation.ReleaseException();
    }
    auto rectifiedLightness = RectifyCSSColorPercent(krys::move(lightness));
    if (rectifiedLightness.HasException())
    {
      return rectifiedLightness.ReleaseException();
    }
    auto rectifiedAlpha = RectifyCSSColorPercent(krys::move(alpha));
    if (rectifiedAlpha.HasException())
    {
      return rectifiedAlpha.ReleaseException();
    }
    return AdoptRef(*new CSSHSL(rectifiedHue.ReleaseValue(), rectifiedSaturation.ReleaseValue(),
                                rectifiedLightness.ReleaseValue(), rectifiedAlpha.ReleaseValue()));
  }

  CSSHSL::CSSHSL(RectifiedCSSColorComponent hue, RectifiedCSSColorComponent saturation,
                 RectifiedCSSColorComponent lightness, RectifiedCSSColorComponent alpha) noexcept
      : _hue(krys::move(hue)), _saturation(krys::move(saturation)), _lightness(krys::move(lightness)),
        _alpha(krys::move(alpha))
  {
  }

  CSSColorAngle CSSHSL::H() const noexcept
  {
    return ToCSSColorComponent(_hue);
  }

  dom::ExceptionOr<void> CSSHSL::SetH(CSSColorAngle hue) noexcept
  {
    auto rectified = RectifyCSSColorAngle(krys::move(hue));
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _hue = rectified.ReleaseValue();
    return {};
  }

  CSSColorPercent CSSHSL::S() const noexcept
  {
    return ToCSSColorComponent(_saturation);
  }

  dom::ExceptionOr<void> CSSHSL::SetS(CSSColorPercent saturation) noexcept
  {
    auto rectified = RectifyCSSColorPercent(krys::move(saturation));
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _saturation = rectified.ReleaseValue();
    return {};
  }

  CSSColorPercent CSSHSL::L() const noexcept
  {
    return ToCSSColorComponent(_lightness);
  }

  dom::ExceptionOr<void> CSSHSL::SetL(CSSColorPercent lightness) noexcept
  {
    auto rectified = RectifyCSSColorPercent(krys::move(lightness));
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _lightness = rectified.ReleaseValue();
    return {};
  }

  CSSColorPercent CSSHSL::Alpha() const noexcept
  {
    return ToCSSColorComponent(_alpha);
  }

  dom::ExceptionOr<void> CSSHSL::SetAlpha(CSSColorPercent alpha) noexcept
  {
    auto rectified = RectifyCSSColorPercent(krys::move(alpha));
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _alpha = rectified.ReleaseValue();
    return {};
  }

  CSSOMString CSSHSL::ColorFunctionText() const noexcept
  {
    const CSSColorPercent components[] {H(), S(), L()};
    return FunctionText(u8"hsl", components, Alpha());
  }
}
