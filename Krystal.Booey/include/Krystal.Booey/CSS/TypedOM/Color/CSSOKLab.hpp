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
  /// @brief A colour in the oklab colour space.
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssoklab
  class CSSOKLab final : public CSSColorValue
  {
  private:
    RectifiedCSSColorComponent _lightness;
    RectifiedCSSColorComponent _a;
    RectifiedCSSColorComponent _b;
    RectifiedCSSColorComponent _alpha;

  public:
    /// @return A `SyntaxError` if a component isn't of its type or `none`.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssoklab-cssoklab
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSOKLab>> Create(CSSColorPercent lightness, CSSColorNumber a,
                                                                 CSSColorNumber b,
                                                                 CSSColorPercent alpha = 1.0) noexcept;

    KRYS_NODISCARD CSSColorPercent L() const noexcept;
    /// @return A `SyntaxError` if @p lightness isn't a percentage or `none`.
    dom::ExceptionOr<void> SetL(CSSColorPercent lightness) noexcept;
    KRYS_NODISCARD CSSColorNumber A() const noexcept;
    /// @return A `SyntaxError` if @p a isn't a number or `none`.
    dom::ExceptionOr<void> SetA(CSSColorNumber a) noexcept;
    KRYS_NODISCARD CSSColorNumber B() const noexcept;
    /// @return A `SyntaxError` if @p b isn't a number or `none`.
    dom::ExceptionOr<void> SetB(CSSColorNumber b) noexcept;
    KRYS_NODISCARD CSSColorPercent Alpha() const noexcept;
    /// @return A `SyntaxError` if @p alpha isn't a percentage or `none`.
    dom::ExceptionOr<void> SetAlpha(CSSColorPercent alpha) noexcept;

    KRYS_NODISCARD Ref<CSSKeywordValue> ColorSpace() const noexcept final;

    KRYS_NODISCARD CSSStyleValueType StyleValueType() const noexcept final
    {
      return CSSStyleValueType::CSSOKLab;
    }

  private:
    CSSOKLab(RectifiedCSSColorComponent lightness, RectifiedCSSColorComponent a, RectifiedCSSColorComponent b,
             RectifiedCSSColorComponent alpha) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSOKLab)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleValue &styleValue) noexcept
  {
    return styleValue.StyleValueType() == krys::boo::css::CSSStyleValueType::CSSOKLab;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
