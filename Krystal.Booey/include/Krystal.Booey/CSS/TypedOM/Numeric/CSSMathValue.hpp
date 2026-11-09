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

#include "Krystal.Booey/CSS/TypedOM/CSSNumericValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathOperator.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSNumericArray.hpp"

namespace krys::boo::css
{
  /// @brief A math expression of numeric values.
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssmathvalue
  class CSSMathValue : public CSSNumericValue
  {
  public:
    explicit CSSMathValue(CSSNumericType type) noexcept : CSSNumericValue(krys::move(type))
    {
    }

    KRYS_NODISCARD virtual CSSMathOperator GetOperator() const noexcept = 0;

    template <typename T>
    KRYS_NODISCARD bool EqualsImpl(const CSSNumericValue &other) const noexcept;

    KRYS_NODISCARD RefPtr<css::Value> ToCSSValue() const noexcept final;
  };

  template <typename T>
  bool CSSMathValue::EqualsImpl(const CSSNumericValue &other) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#equal-numeric-value
    auto *otherT = DynamicDowncast<T>(other);
    if (!otherT)
    {
      return false;
    }

    krys_debug_assert(StyleValueType() == other.StyleValueType());
    const auto &thisValues = static_cast<const T *>(this)->Values();
    const auto &otherValues = otherT->Values();
    auto length = thisValues.Length();
    if (length != otherValues.Length())
    {
      return false;
    }

    for (size_t i = 0; i < length; ++i)
    {
      if (!thisValues.Array()[i]->Equals(*otherValues.Array()[i]))
      {
        return false;
      }
    }

    return true;
  }
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSMathValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleValue &styleValue) noexcept
  {
    return krys::boo::css::IsCSSMathValue(styleValue.StyleValueType());
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
