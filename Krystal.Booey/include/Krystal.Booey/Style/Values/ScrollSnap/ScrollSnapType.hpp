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

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'scroll-snap-type'> = none | [ x | y | block | inline | both ] [ mandatory | proximity
  // ]?@(default=proximity) https://drafts.csswg.org/css-scroll-snap-1/#propdef-scroll-snap-type
  struct ScrollSnapType
  {
    struct ScrollSnapContainer
    {
      ScrollSnapAxis Axis;
      ScrollSnapStrictness Strictness;

      KRYS_NODISCARD constexpr bool operator==(const ScrollSnapContainer &) const noexcept = default;
    };

    constexpr ScrollSnapType(css::keywords::None) noexcept
    {
    }
    constexpr ScrollSnapType(ScrollSnapAxis axis,
                             ScrollSnapStrictness strictness = ScrollSnapStrictness::Proximity) noexcept
        : _value {ScrollSnapContainer {axis, strictness}}
    {
    }

    KRYS_NODISCARD constexpr bool IsNone() const noexcept
    {
      return !_value;
    }
    KRYS_NODISCARD constexpr bool IsContainer() const noexcept
    {
      return !!_value;
    }
    KRYS_NODISCARD constexpr Maybe<ScrollSnapContainer> TryContainer() const noexcept
    {
      return _value;
    }

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNone())
      {
        return visitor(css::keywords::None {});
      }

      switch (_value->Strictness)
      {
        case ScrollSnapStrictness::Proximity: return visitor(_value->Axis);
        case ScrollSnapStrictness::Mandatory:
          return visitor(css::SpaceSeparatedTuple {_value->Axis, css::keywords::Mandatory {}});
      }
      krys_unreachable();
    }

    KRYS_NODISCARD constexpr bool operator==(const ScrollSnapType &) const noexcept = default;

  private:
    Maybe<ScrollSnapContainer> _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<ScrollSnapType>
  {
    KRYS_NODISCARD ScrollSnapType operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ScrollSnapType)
