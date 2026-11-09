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
  // <'-webkit-initial-letter'> = normal | <number [0,∞]>{1,2}@(default=previous)
  // NOTE: There is a standard `initial-letter` property with a different grammar that is not yet implemented.
  // https://drafts.csswg.org/css-inline/#propdef-initial-letter
  struct WebkitInitialLetter
  {
    constexpr WebkitInitialLetter(css::keywords::Normal) noexcept
    {
    }

    constexpr WebkitInitialLetter(Number<css::NonNegative, float> height) noexcept : _value {height, height}
    {
    }

    constexpr WebkitInitialLetter(Number<css::NonNegative, float> height,
                                  Number<css::NonNegative, float> drop) noexcept
        : _value {height, drop}
    {
    }

    KRYS_NODISCARD constexpr bool IsNormal() const noexcept
    {
      return _value.first().IsZero() && _value.second().IsZero();
    }

    KRYS_NODISCARD constexpr float Height() const noexcept
    {
      return _value.first().Value;
    }
    KRYS_NODISCARD constexpr float Drop() const noexcept
    {
      return _value.second().Value;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNormal())
      {
        return visitor(css::keywords::Normal {});
      }
      return visitor(_value);
    }

    KRYS_NODISCARD bool operator==(const WebkitInitialLetter &) const noexcept = default;

  private:
    css::MinimallySerialisingSpaceSeparatedPair<Number<css::NonNegative, float>> _value {0, 0};
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<WebkitInitialLetter>
  {
    KRYS_NODISCARD WebkitInitialLetter operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::WebkitInitialLetter)
