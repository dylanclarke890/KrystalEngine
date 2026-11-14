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

#include "Krystal.Booey/CSS/TypedOM/Color/CSSHWB.hpp"

namespace krys::boo::css
{
  namespace
  {
    /// @brief @p hue, if it's an angle.
    KRYS_NODISCARD dom::ExceptionOr<Ref<CSSNumericValue>> RectifyHWBHue(Ref<CSSNumericValue> hue) noexcept
    {
      if (!hue->Type().Matches<CSSNumericBaseType::Angle>())
      {
        return dom::Exception {dom::ExceptionCode::SyntaxError};
      }
      return hue;
    }

    /// @brief @p numberish as a percentage, a number being a fraction.
    KRYS_NODISCARD dom::ExceptionOr<Ref<CSSNumericValue>>
      RectifyHWBPercent(const CSSNumberish &numberish) noexcept
    {
      auto rectified = CSSColorValue::RectifyCSSColorPercent(CSSColorValue::ToCSSColorPercent(numberish));
      if (rectified.HasException())
      {
        return rectified.ReleaseException();
      }
      // A numberish can't be `none`.
      return Ref<CSSNumericValue>(std::get<RefPtr<CSSNumericValue>>(rectified.ReleaseValue()));
    }
  }

  dom::ExceptionOr<Ref<CSSHWB>> CSSHWB::Create(Ref<CSSNumericValue> hue, CSSNumberish whiteness,
                                               CSSNumberish blackness, CSSNumberish alpha) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-csshwb-csshwb
    auto rectifiedHue = RectifyHWBHue(krys::move(hue));
    if (rectifiedHue.HasException())
    {
      return rectifiedHue.ReleaseException();
    }
    auto rectifiedWhiteness = RectifyHWBPercent(whiteness);
    if (rectifiedWhiteness.HasException())
    {
      return rectifiedWhiteness.ReleaseException();
    }
    auto rectifiedBlackness = RectifyHWBPercent(blackness);
    if (rectifiedBlackness.HasException())
    {
      return rectifiedBlackness.ReleaseException();
    }
    auto rectifiedAlpha = RectifyHWBPercent(alpha);
    if (rectifiedAlpha.HasException())
    {
      return rectifiedAlpha.ReleaseException();
    }
    return AdoptRef(*new CSSHWB(rectifiedHue.ReleaseValue(), rectifiedWhiteness.ReleaseValue(),
                                rectifiedBlackness.ReleaseValue(), rectifiedAlpha.ReleaseValue()));
  }

  CSSHWB::CSSHWB(Ref<CSSNumericValue> hue, Ref<CSSNumericValue> whiteness, Ref<CSSNumericValue> blackness,
                 Ref<CSSNumericValue> alpha) noexcept
      : _hue(krys::move(hue)), _whiteness(krys::move(whiteness)), _blackness(krys::move(blackness)),
        _alpha(krys::move(alpha))
  {
  }

  dom::ExceptionOr<void> CSSHWB::SetH(Ref<CSSNumericValue> hue) noexcept
  {
    auto rectified = RectifyHWBHue(krys::move(hue));
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _hue = rectified.ReleaseValue();
    return {};
  }

  CSSNumberish CSSHWB::W() const noexcept
  {
    return RefPtr<CSSNumericValue>(_whiteness);
  }

  dom::ExceptionOr<void> CSSHWB::SetW(CSSNumberish whiteness) noexcept
  {
    auto rectified = RectifyHWBPercent(whiteness);
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _whiteness = rectified.ReleaseValue();
    return {};
  }

  CSSNumberish CSSHWB::B() const noexcept
  {
    return RefPtr<CSSNumericValue>(_blackness);
  }

  dom::ExceptionOr<void> CSSHWB::SetB(CSSNumberish blackness) noexcept
  {
    auto rectified = RectifyHWBPercent(blackness);
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _blackness = rectified.ReleaseValue();
    return {};
  }

  CSSNumberish CSSHWB::Alpha() const noexcept
  {
    return RefPtr<CSSNumericValue>(_alpha);
  }

  dom::ExceptionOr<void> CSSHWB::SetAlpha(CSSNumberish alpha) noexcept
  {
    auto rectified = RectifyHWBPercent(alpha);
    if (rectified.HasException())
    {
      return rectified.ReleaseException();
    }
    _alpha = rectified.ReleaseValue();
    return {};
  }

  CSSOMString CSSHWB::ColorFunctionText() const noexcept
  {
    const CSSColorPercent components[] {CSSColorPercent {RefPtr<CSSNumericValue>(_hue)},
                                        ToCSSColorPercent(W()), ToCSSColorPercent(B())};
    return FunctionText(u8"hwb", components, ToCSSColorPercent(Alpha()));
  }
}
