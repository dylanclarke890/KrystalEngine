/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <opacity-value> = <number [0,1]@(allow-both-at-parse)> | <percentage [0,100]@(allow-both-at-parse,
  // converted-to-number)> https://drafts.csswg.org/css-color-4/#typedef-opacity-opacity-value
  struct Opacity
  {
    using Number = style::Number<css::ClosedUnitRange, float>;

    Number Value;

    constexpr Opacity(Number number) noexcept : Value {number}
    {
    }

    constexpr Opacity(float literal) noexcept : Value {literal}
    {
    }

    constexpr Opacity(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept : Value {literal}
    {
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return Value == 0;
    }

    KRYS_NODISCARD constexpr bool IsTransparent() const noexcept
    {
      return Value == 0;
    }

    KRYS_NODISCARD constexpr bool IsOpaque() const noexcept
    {
      return Value == 1;
    }

    KRYS_NODISCARD constexpr bool operator==(const Opacity &) const noexcept = default;
    KRYS_NODISCARD constexpr auto operator<=>(const Opacity &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(Opacity, Value);

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<Opacity>
  {
    KRYS_NODISCARD Opacity operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::Opacity)
