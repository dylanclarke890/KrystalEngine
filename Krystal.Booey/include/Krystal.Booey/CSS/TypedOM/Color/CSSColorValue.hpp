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

#include "Krystal.Booey/CSS/TypedOM/CSSKeywordValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSStyleValue.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::css
{
  /// @brief A colour component that's a percentage: a number (a fraction), a numeric value, or `none`.
  /// @see https://drafts.css-houdini.org/css-typed-om/#typedefdef-csscolorpercent
  using CSSColorPercent = Variant<double, RefPtr<CSSNumericValue>, CSSOMString, RefPtr<CSSKeywordValue>>;
  /// @brief A colour component that's a number: a number, a numeric value, or `none`.
  /// @see https://drafts.css-houdini.org/css-typed-om/#typedefdef-csscolornumber
  using CSSColorNumber = Variant<double, RefPtr<CSSNumericValue>, CSSOMString, RefPtr<CSSKeywordValue>>;
  /// @brief A colour component that's an angle: a number (of degrees), a numeric value, or `none`.
  /// @see https://drafts.css-houdini.org/css-typed-om/#typedefdef-csscolorangle
  using CSSColorAngle = Variant<double, RefPtr<CSSNumericValue>, CSSOMString, RefPtr<CSSKeywordValue>>;
  /// @brief A colour component after rectifying: a numeric value, or the keyword `none`.
  using RectifiedCSSColorComponent = Variant<RefPtr<CSSNumericValue>, RefPtr<CSSKeywordValue>>;

  /// @brief A colour value.
  /// @see https://drafts.css-houdini.org/css-typed-om/#csscolorvalue
  // FIXME: Converting between colour spaces (`to()`), parsing (`parse()`), serialisation and conversion to
  // CSS values.
  class CSSColorValue : public CSSStyleValue
  {
  public:
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-csscolorvalue-colorspace
    KRYS_NODISCARD virtual Ref<CSSKeywordValue> ColorSpace() const noexcept = 0;

    /// @return A `SyntaxError` if @p percent is neither a percentage nor `none`.
    /// @see https://drafts.css-houdini.org/css-typed-om/#rectify-a-csscolorpercent
    KRYS_NODISCARD static dom::ExceptionOr<RectifiedCSSColorComponent>
      RectifyCSSColorPercent(CSSColorPercent percent) noexcept;
    /// @return A `SyntaxError` if @p angle is neither an angle nor `none`.
    /// @see https://drafts.css-houdini.org/css-typed-om/#rectify-a-csscolorangle
    KRYS_NODISCARD static dom::ExceptionOr<RectifiedCSSColorComponent>
      RectifyCSSColorAngle(CSSColorAngle angle) noexcept;
    /// @return A `SyntaxError` if @p number is neither a number nor `none`.
    /// @see https://drafts.css-houdini.org/css-typed-om/#rectify-a-csscolornumber
    KRYS_NODISCARD static dom::ExceptionOr<RectifiedCSSColorComponent>
      RectifyCSSColorNumber(CSSColorNumber number) noexcept;
    /// @brief @p component as the variant the component's getters return.
    KRYS_NODISCARD static CSSColorPercent
      ToCSSColorComponent(const RectifiedCSSColorComponent &component) noexcept;
    /// @brief @p numberish as a percentage component, a number being a fraction.
    KRYS_NODISCARD static CSSColorPercent ToCSSColorPercent(const CSSNumberish &numberish) noexcept;

    KRYS_NODISCARD RefPtr<Value> ToCSSValue() const noexcept final;

  protected:
    CSSColorValue() noexcept = default;

    /// @brief The keyword value @p colorSpace.
    KRYS_NODISCARD static Ref<CSSKeywordValue> ColorSpaceKeyword(utf8_stringview colorSpace) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSColorValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleValue &styleValue) noexcept
  {
    return krys::boo::css::IsCSSColorValue(styleValue.StyleValueType());
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
