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
  /// @brief A colour in the hwb colour space.
  /// @see https://drafts.css-houdini.org/css-typed-om/#csshwb
  class CSSHWB final : public CSSColorValue
  {
  private:
    Ref<CSSNumericValue> _hue;
    Ref<CSSNumericValue> _whiteness;
    Ref<CSSNumericValue> _blackness;
    Ref<CSSNumericValue> _alpha;

  public:
    /// @return A `SyntaxError` if @p hue isn't an angle, or @p whiteness, @p blackness or @p alpha isn't a
    /// percentage (a number being a fraction).
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-csshwb-csshwb
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSHWB>> Create(Ref<CSSNumericValue> hue,
                                                               CSSNumberish whiteness, CSSNumberish blackness,
                                                               CSSNumberish alpha = 1.0) noexcept;

    KRYS_NODISCARD const CSSNumericValue &H() const noexcept
    {
      return *_hue;
    }
    /// @return A `SyntaxError` if @p hue isn't an angle.
    dom::ExceptionOr<void> SetH(Ref<CSSNumericValue> hue) noexcept;
    KRYS_NODISCARD CSSNumberish W() const noexcept;
    /// @return A `SyntaxError` if @p whiteness isn't a percentage.
    dom::ExceptionOr<void> SetW(CSSNumberish whiteness) noexcept;
    KRYS_NODISCARD CSSNumberish B() const noexcept;
    /// @return A `SyntaxError` if @p blackness isn't a percentage.
    dom::ExceptionOr<void> SetB(CSSNumberish blackness) noexcept;
    KRYS_NODISCARD CSSNumberish Alpha() const noexcept;
    /// @return A `SyntaxError` if @p alpha isn't a percentage.
    dom::ExceptionOr<void> SetAlpha(CSSNumberish alpha) noexcept;

    KRYS_NODISCARD CSSOMString ColorFunctionText() const noexcept final;

    KRYS_NODISCARD CSSStyleValueType StyleValueType() const noexcept final
    {
      return CSSStyleValueType::CSSHWB;
    }

  private:
    CSSHWB(Ref<CSSNumericValue> hue, Ref<CSSNumericValue> whiteness, Ref<CSSNumericValue> blackness,
           Ref<CSSNumericValue> alpha) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSHWB)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleValue &styleValue) noexcept
  {
    return styleValue.StyleValueType() == krys::boo::css::CSSStyleValueType::CSSHWB;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
