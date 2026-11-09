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

#include "Krystal.Booey/Style/Scope/ScopedName.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <single-animation-name> = none | <keyframes-name>
  // https://www.w3.org/TR/css-animations-1/#propdef-animation-name
  struct SingleAnimationName
  {
    SingleAnimationName(css::keywords::None) noexcept : _value {ScopedName {css::CSSOMStringAtom::Null()}}
    {
    }

    SingleAnimationName(ScopedName &&keyframesName) noexcept : _value {krys::move(keyframesName)}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return _value.Name == css::CSSOMStringAtom::Null();
    }
    KRYS_NODISCARD bool IsKeyframesName() const noexcept
    {
      return _value.Name != css::CSSOMStringAtom::Null();
    }
    KRYS_NODISCARD Maybe<ScopedName> TryKeyframesName() const noexcept
    {
      return IsKeyframesName() ? Maybe<ScopedName> {_value} : std::nullopt;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNone())
      {
        return visitor(css::keywords::None {});
      }
      return visitor(_value);
    }

    KRYS_NODISCARD bool operator==(const SingleAnimationName &) const noexcept = default;

  private:
    ScopedName _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<SingleAnimationName>
  {
    KRYS_NODISCARD SingleAnimationName operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SingleAnimationName)
