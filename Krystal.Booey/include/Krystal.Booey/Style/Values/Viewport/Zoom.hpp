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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::style
{
  // <'zoom'> = <number [0,∞]> | <percentage [0,∞]> | normal
  // NOTE: `normal` is non-standard and gets resolved to `1` at style building time.
  // https://drafts.csswg.org/css-viewport/#propdef-zoom
  struct Zoom
  {
    using Representation = NumberOrPercentageResolvedToNumber<css::NonNegative, css::NonNegative, float>;
    using Number = Representation::Number;
    using Percentage = Representation::Percentage;

    Representation Value;

    constexpr Zoom(Representation value) noexcept : Value {value}
    {
    }
    constexpr Zoom(Number number) noexcept : Value {number}
    {
    }
    constexpr Zoom(Percentage percentage) noexcept : Value {percentage}
    {
    }
    constexpr Zoom(typename Number::ResolvedValueType value) noexcept : Value {value}
    {
    }
    constexpr Zoom(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept : Value {literal}
    {
    }
    constexpr Zoom(css::ValueLiteral<css::PercentageUnit::Percentage> literal) noexcept : Value {literal}
    {
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return Value.IsZero();
    }

    KRYS_NODISCARD constexpr bool operator==(const Zoom &) const noexcept = default;
    KRYS_NODISCARD constexpr bool operator==(typename Number::ResolvedValueType other) const noexcept
    {
      return Value == other;
    }
  };
  DEFINE_TYPE_WRAPPER_GET(Zoom, Value)
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::Zoom)
