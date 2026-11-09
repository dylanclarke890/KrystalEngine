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

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo
{
  class TabSize;

  namespace style
  {
    // <'tab-size'> = <number [0,∞]> | <length [0,∞]>
    // https://drafts.csswg.org/css-text-3/#propdef-tab-size
    struct TabSize
    {
      using Spaces = style::Number<css::NonNegative, float>;
      using Length = style::Length<css::NonNegative, float>;

      constexpr TabSize(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept
          : _value {Spaces {literal}}
      {
      }

      constexpr TabSize(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept : _value {Length {literal}}
      {
      }

      constexpr TabSize(Spaces spaces) noexcept : _value {spaces}
      {
      }

      constexpr TabSize(Length length) noexcept : _value {length}
      {
      }

      KRYS_NODISCARD constexpr bool IsSpaces() const noexcept
      {
        return HoldsAlternative<Spaces>();
      }
      KRYS_NODISCARD constexpr bool IsLength() const noexcept
      {
        return HoldsAlternative<Length>();
      }
      KRYS_NODISCARD constexpr Maybe<Spaces> TrySpaces() const noexcept
      {
        return IsSpaces() ? Maybe(std::get<Spaces>(_value)) : null;
      }
      KRYS_NODISCARD constexpr Maybe<Length> TryLength() const noexcept
      {
        return IsLength() ? Maybe(std::get<Length>(_value)) : null;
      }

      KRYS_NODISCARD constexpr bool IsZero() const noexcept
      {
        return krys::SwitchOn(_value, [](auto value) { return value.IsZero(); });
      }

      template <typename U>
      KRYS_NODISCARD constexpr bool HoldsAlternative() const noexcept
      {
        return krys::HoldsAlternative<U>(_value);
      }

      template <typename... F>
      constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
      {
        return krys::SwitchOn(_value, std::forward<F>(f)...);
      }

      KRYS_NODISCARD constexpr bool operator==(const TabSize &) const noexcept = default;
      KRYS_NODISCARD constexpr bool HasSameType(const TabSize &other) const noexcept
      {
        return _value.index() == other._value.index();
      }

    private:
      friend struct Blending<TabSize>;

      Variant<Spaces, Length> _value;
    };

    // MARK: - Conversion

    template <>
    struct CSSValueConversion<TabSize>
    {
      KRYS_NODISCARD TabSize operator()(BuilderState &, const css::Value &) noexcept;
    };

    // MARK: - Blending

    template <>
    struct Blending<TabSize>
    {
      KRYS_NODISCARD bool CanBlend(const TabSize &, const TabSize &) noexcept;
      KRYS_NODISCARD TabSize Blend(const TabSize &, const TabSize &, const BlendingContext &) noexcept;
    };

    // MARK: - Platform

    template <>
    struct ToPlatform<TabSize>
    {
      KRYS_NODISCARD boo::TabSize operator()(const TabSize &) noexcept;
    };

  }
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TabSize);
