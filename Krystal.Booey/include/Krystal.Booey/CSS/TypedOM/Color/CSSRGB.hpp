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

#pragma once

#include "Krystal.Booey/CSS/TypedOM/Color/CSSColorValue.hpp"

namespace krys::boo::css
{
  /// @brief An RGB channel: a number (a fraction), a numeric value, or `none`.
  /// @see https://drafts.css-houdini.org/css-typed-om/#typedefdef-csscolorrgbcomp
  using CSSColorRGBComp = Variant<double, RefPtr<CSSNumericValue>, CSSOMString, RefPtr<CSSKeywordValue>>;

  /// @brief A colour in the srgb colour space.
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssrgb
  class CSSRGB final : public CSSColorValue
  {
  private:
    RectifiedCSSColorComponent _red;
    RectifiedCSSColorComponent _green;
    RectifiedCSSColorComponent _blue;
    RectifiedCSSColorComponent _alpha;

  public:
    /// @return A `SyntaxError` if a component isn't of its type or `none`.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssrgb-cssrgb
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSRGB>> Create(CSSColorRGBComp red, CSSColorRGBComp green,
                                                               CSSColorRGBComp blue,
                                                               CSSColorPercent alpha = 1.0) noexcept;

    KRYS_NODISCARD CSSColorRGBComp R() const noexcept;
    /// @return A `SyntaxError` if @p red isn't a number or percentage or `none`.
    dom::ExceptionOr<void> SetR(CSSColorRGBComp red) noexcept;
    KRYS_NODISCARD CSSColorRGBComp G() const noexcept;
    /// @return A `SyntaxError` if @p green isn't a number or percentage or `none`.
    dom::ExceptionOr<void> SetG(CSSColorRGBComp green) noexcept;
    KRYS_NODISCARD CSSColorRGBComp B() const noexcept;
    /// @return A `SyntaxError` if @p blue isn't a number or percentage or `none`.
    dom::ExceptionOr<void> SetB(CSSColorRGBComp blue) noexcept;
    KRYS_NODISCARD CSSColorPercent Alpha() const noexcept;
    /// @return A `SyntaxError` if @p alpha isn't a percentage or `none`.
    dom::ExceptionOr<void> SetAlpha(CSSColorPercent alpha) noexcept;

    /// @return A `SyntaxError` if @p component is neither a number, a percentage nor `none`.
    /// @see https://drafts.css-houdini.org/css-typed-om/#rectify-a-csscolorrgbcomp
    KRYS_NODISCARD static dom::ExceptionOr<RectifiedCSSColorComponent>
      RectifyCSSColorRGBComp(CSSColorRGBComp component) noexcept;

    KRYS_NODISCARD Ref<CSSKeywordValue> ColorSpace() const noexcept final;

    KRYS_NODISCARD CSSStyleValueType StyleValueType() const noexcept final
    {
      return CSSStyleValueType::CSSRGB;
    }

  private:
    CSSRGB(RectifiedCSSColorComponent red, RectifiedCSSColorComponent green, RectifiedCSSColorComponent blue,
           RectifiedCSSColorComponent alpha) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSRGB)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleValue &styleValue) noexcept
  {
    return styleValue.StyleValueType() == krys::boo::css::CSSStyleValueType::CSSRGB;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
