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

#include "Krystal.Booey/CSS/TypedOM/Color/CSSRGB.hpp"

namespace krys::boo::css
{
  dom::ExceptionOr<Ref<CSSRGB>> CSSRGB::Create(CSSColorRGBComp red, CSSColorRGBComp green,
                                               CSSColorRGBComp blue, CSSColorPercent alpha) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssrgb-cssrgb
    auto rectifiedRed = RectifyCSSColorRGBComp(krys::move(red));
    if (rectifiedRed.HasException())
    {
      return rectifiedRed.ReleaseException();
    }
    auto rectifiedGreen = RectifyCSSColorRGBComp(krys::move(green));
    if (rectifiedGreen.HasException())
    {
      return rectifiedGreen.ReleaseException();
    }
    auto rectifiedBlue = RectifyCSSColorRGBComp(krys::move(blue));
    if (rectifiedBlue.HasException())
    {
      return rectifiedBlue.ReleaseException();
    }
    auto rectifiedAlpha = RectifyCSSColorPercent(krys::move(alpha));
    if (rectifiedAlpha.HasException())
    {
      return rectifiedAlpha.ReleaseException();
    }
    return AdoptRef(*new CSSRGB(rectifiedRed.ReleaseValue(), rectifiedGreen.ReleaseValue(),
                                rectifiedBlue.ReleaseValue(), rectifiedAlpha.ReleaseValue()));
  }

  CSSRGB::CSSRGB(RectifiedCSSColorComponent red, RectifiedCSSColorComponent green,
                 RectifiedCSSColorComponent blue, RectifiedCSSColorComponent alpha) noexcept
      : _red(krys::move(red)), _green(krys::move(green)), _blue(krys::move(blue)), _alpha(krys::move(alpha))
  {
  }

  CSSColorRGBComp CSSRGB::R() const noexcept
  {
    return ToCSSColorComponent(_red);
  }

  dom::ExceptionOr<void> CSSRGB::SetR(CSSColorRGBComp red) noexcept
  {
    auto rectified = RectifyCSSColorRGBComp(krys::move(red));
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _red = rectified.ReleaseValue();
    return {};
  }

  CSSColorRGBComp CSSRGB::G() const noexcept
  {
    return ToCSSColorComponent(_green);
  }

  dom::ExceptionOr<void> CSSRGB::SetG(CSSColorRGBComp green) noexcept
  {
    auto rectified = RectifyCSSColorRGBComp(krys::move(green));
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _green = rectified.ReleaseValue();
    return {};
  }

  CSSColorRGBComp CSSRGB::B() const noexcept
  {
    return ToCSSColorComponent(_blue);
  }

  dom::ExceptionOr<void> CSSRGB::SetB(CSSColorRGBComp blue) noexcept
  {
    auto rectified = RectifyCSSColorRGBComp(krys::move(blue));
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _blue = rectified.ReleaseValue();
    return {};
  }

  CSSColorPercent CSSRGB::Alpha() const noexcept
  {
    return ToCSSColorComponent(_alpha);
  }

  dom::ExceptionOr<void> CSSRGB::SetAlpha(CSSColorPercent alpha) noexcept
  {
    auto rectified = RectifyCSSColorPercent(krys::move(alpha));
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _alpha = rectified.ReleaseValue();
    return {};
  }

  dom::ExceptionOr<RectifiedCSSColorComponent>
    CSSRGB::RectifyCSSColorRGBComp(CSSColorRGBComp component) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#rectify-a-csscolorrgbcomp
    // 1. If val is a double, replace it with a new unit value of (val * 100, "percent").
    // 2. If val is a CSSNumericValue that matches <number> or <percentage>, return val.
    // 3. If val is a DOMString or CSSKeywordValue, rectify a keywordish value from it; if its value is an
    // ASCII
    //    case-insensitive match for "none", return it.
    // 4. Throw a SyntaxError.
    if (auto *numeric = std::get_if<RefPtr<CSSNumericValue>>(&component))
    {
      if (*numeric && (*numeric)->Type().MatchesNumber())
      {
        return RectifiedCSSColorComponent {krys::move(*numeric)};
      }
    }
    return RectifyCSSColorPercent(krys::move(component));
  }

  CSSOMString CSSRGB::ColorFunctionText() const noexcept
  {
    const CSSColorPercent components[] {R(), G(), B()};
    return FunctionText(u8"rgb", components, Alpha());
  }
}
