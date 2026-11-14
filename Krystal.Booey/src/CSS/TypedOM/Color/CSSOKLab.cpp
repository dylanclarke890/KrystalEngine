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

#include "Krystal.Booey/CSS/TypedOM/Color/CSSOKLab.hpp"

namespace krys::boo::css
{
  dom::ExceptionOr<Ref<CSSOKLab>> CSSOKLab::Create(CSSColorPercent lightness, CSSColorNumber a,
                                                   CSSColorNumber b, CSSColorPercent alpha) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssoklab-cssoklab
    auto rectifiedLightness = RectifyCSSColorPercent(krys::move(lightness));
    if (rectifiedLightness.HasException())
    {
      return rectifiedLightness.ReleaseException();
    }
    auto rectifiedA = RectifyCSSColorNumber(krys::move(a));
    if (rectifiedA.HasException())
    {
      return rectifiedA.ReleaseException();
    }
    auto rectifiedB = RectifyCSSColorNumber(krys::move(b));
    if (rectifiedB.HasException())
    {
      return rectifiedB.ReleaseException();
    }
    auto rectifiedAlpha = RectifyCSSColorPercent(krys::move(alpha));
    if (rectifiedAlpha.HasException())
    {
      return rectifiedAlpha.ReleaseException();
    }
    return AdoptRef(*new CSSOKLab(rectifiedLightness.ReleaseValue(), rectifiedA.ReleaseValue(),
                                  rectifiedB.ReleaseValue(), rectifiedAlpha.ReleaseValue()));
  }

  CSSOKLab::CSSOKLab(RectifiedCSSColorComponent lightness, RectifiedCSSColorComponent a,
                     RectifiedCSSColorComponent b, RectifiedCSSColorComponent alpha) noexcept
      : _lightness(krys::move(lightness)), _a(krys::move(a)), _b(krys::move(b)), _alpha(krys::move(alpha))
  {
  }

  CSSColorPercent CSSOKLab::L() const noexcept
  {
    return ToCSSColorComponent(_lightness);
  }

  dom::ExceptionOr<void> CSSOKLab::SetL(CSSColorPercent lightness) noexcept
  {
    auto rectified = RectifyCSSColorPercent(krys::move(lightness));
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _lightness = rectified.ReleaseValue();
    return {};
  }

  CSSColorNumber CSSOKLab::A() const noexcept
  {
    return ToCSSColorComponent(_a);
  }

  dom::ExceptionOr<void> CSSOKLab::SetA(CSSColorNumber a) noexcept
  {
    auto rectified = RectifyCSSColorNumber(krys::move(a));
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _a = rectified.ReleaseValue();
    return {};
  }

  CSSColorNumber CSSOKLab::B() const noexcept
  {
    return ToCSSColorComponent(_b);
  }

  dom::ExceptionOr<void> CSSOKLab::SetB(CSSColorNumber b) noexcept
  {
    auto rectified = RectifyCSSColorNumber(krys::move(b));
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _b = rectified.ReleaseValue();
    return {};
  }

  CSSColorPercent CSSOKLab::Alpha() const noexcept
  {
    return ToCSSColorComponent(_alpha);
  }

  dom::ExceptionOr<void> CSSOKLab::SetAlpha(CSSColorPercent alpha) noexcept
  {
    auto rectified = RectifyCSSColorPercent(krys::move(alpha));
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _alpha = rectified.ReleaseValue();
    return {};
  }

  CSSOMString CSSOKLab::ColorFunctionText() const noexcept
  {
    const CSSColorPercent components[] {L(), A(), B()};
    return FunctionText(u8"oklab", components, Alpha());
  }
}
