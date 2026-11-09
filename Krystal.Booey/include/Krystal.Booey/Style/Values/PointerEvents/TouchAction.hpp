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

#include "Krystal.Booey/Style/TouchAction.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'touch-action'> = auto | none | [ [ pan-x | pan-left | pan-right ] || [ pan-y | pan-up | pan-down ] ] |
  // manipulation
  // FIXME: Currently implemented grammar is: auto | none | [ pan-x || pan-y || pinch-zoom ] | manipulation
  // https://w3c.github.io/pointerevents/#the-touch-action-css-property

  enum class TouchActionValue : uint8
  {
    PanX,
    PanY,
    PinchZoom,
  };

  using TouchActionValueEnumSet = css::SpaceSeparatedEnumSet<TouchActionValue>;

  struct TouchAction
  {
    constexpr TouchAction(css::keywords::Auto keyword) noexcept : _value {keyword}
    {
    }
    constexpr TouchAction(css::keywords::None keyword) noexcept : _value {keyword}
    {
    }
    constexpr TouchAction(css::keywords::Manipulation keyword) noexcept : _value {keyword}
    {
    }
    constexpr TouchAction(TouchActionValueEnumSet &&set) noexcept : _value {krys::move(set)}
    {
    }
    constexpr TouchAction(TouchActionValue value) noexcept : _value {TouchActionValueEnumSet {value}}
    {
    }

    KRYS_NODISCARD constexpr bool IsNone() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::None>(_value);
    }
    KRYS_NODISCARD constexpr bool IsAuto() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::Auto>(_value);
    }
    KRYS_NODISCARD constexpr bool IsManipulation() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::Manipulation>(_value);
    }
    KRYS_NODISCARD constexpr bool IsEnumSet() const noexcept
    {
      return krys::HoldsAlternative<TouchActionValueEnumSet>(_value);
    }
    KRYS_NODISCARD constexpr Maybe<TouchActionValueEnumSet> TryEnumSet() const noexcept
    {
      return IsEnumSet() ? Maybe(std::get<TouchActionValueEnumSet>(_value)) : null;
    }

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(_value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD constexpr bool operator==(const TouchAction &) const noexcept = default;

  private:
    Variant<css::keywords::Auto, css::keywords::None, TouchActionValueEnumSet, css::keywords::Manipulation>
      _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<TouchAction>
  {
    KRYS_NODISCARD TouchAction operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Platform

  template <>
  struct ToPlatform<TouchAction>
  {
    KRYS_NODISCARD EnumSet<boo::TouchAction> operator()(const TouchAction &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TouchAction)
