/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/TypedOM/CSSStyleValue.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::css
{
  class CSSKeywordValue;

  /// @brief A keyword, as a string or a keyword value.
  /// @see https://drafts.css-houdini.org/css-typed-om/#typedefdef-csskeywordish
  using CSSKeywordish = Variant<CSSOMString, RefPtr<CSSKeywordValue>>;

  /// @brief A CSS keyword, or an identifier.
  /// @see https://drafts.css-houdini.org/css-typed-om/#csskeywordvalue
  class CSSKeywordValue final : public CSSStyleValue
  {
  private:
    CSSOMString _value;

  public:
    /// @return A `TypeError` if @p value is empty.
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSKeywordValue>> Create(const CSSOMString &value) noexcept;

    KRYS_NODISCARD const CSSOMString &Value() const noexcept
    {
      return _value;
    }
    /// @return A `TypeError`, leaving the value unchanged, if @p value is empty.
    dom::ExceptionOr<void> SetValue(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSStyleValueType StyleValueType() const noexcept final
    {
      return CSSStyleValueType::CSSKeywordValue;
    }

    /// @see https://drafts.css-houdini.org/css-typed-om/#rectify-a-keywordish-value
    KRYS_NODISCARD static Ref<CSSKeywordValue> RectifyKeywordish(CSSKeywordish &&keywordish) noexcept;

    KRYS_NODISCARD RefPtr<css::Value> ToCSSValue() const noexcept final;

  private:
    void Serialise(StringBuilder &builder, EnumSet<SerialisationArguments> arguments) const noexcept final;

    explicit CSSKeywordValue(const CSSOMString &value) noexcept : _value(value)
    {
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSKeywordValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleValue &styleValue) noexcept
  {
    return styleValue.StyleValueType() == krys::boo::css::CSSStyleValueType::CSSKeywordValue;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
