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

#include "Krystal.Booey/Style/Values/ScrollAnimations/ScrollFunction.hpp"
#include "Krystal.Booey/Style/Values/ScrollAnimations/ViewFunction.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <single-animation-timeline> = auto | none | <custom-ident> | <scroll()> | <view()>
  // https://www.w3.org/TR/css-animations-2/#typedef-single-animation-timeline
  struct SingleAnimationTimeline
  {
    SingleAnimationTimeline(css::keywords::Auto keyword) noexcept : _value {keyword}
    {
    }

    SingleAnimationTimeline(css::keywords::None keyword) noexcept : _value {keyword}
    {
    }

    SingleAnimationTimeline(css::CustomIdentifier &&identifier) noexcept : _value {identifier}
    {
    }

    SingleAnimationTimeline(ScrollFunction &&scrollFunction) noexcept : _value {krys::move(scrollFunction)}
    {
    }

    SingleAnimationTimeline(ViewFunction &&viewFunction) noexcept : _value {krys::move(viewFunction)}
    {
    }

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return std::holds_alternative<css::keywords::Auto>(_value);
    }
    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return std::holds_alternative<css::keywords::None>(_value);
    }
    KRYS_NODISCARD bool IsCustomIdentifier() const noexcept
    {
      return std::holds_alternative<css::CustomIdentifier>(_value);
    }
    KRYS_NODISCARD Maybe<css::CustomIdentifier> TryCustomIdentifier() const noexcept
    {
      return IsCustomIdentifier() ? std::make_optional(std::get<css::CustomIdentifier>(_value))
                                  : std::nullopt;
    }
    KRYS_NODISCARD bool IsScrollFunction() const noexcept
    {
      return std::holds_alternative<ScrollFunction>(_value);
    }
    KRYS_NODISCARD Maybe<ScrollFunction> TryScrollFunction() const noexcept
    {
      return IsScrollFunction() ? std::make_optional(std::get<ScrollFunction>(_value)) : std::nullopt;
    }
    KRYS_NODISCARD bool IsViewFunction() const noexcept
    {
      return std::holds_alternative<ViewFunction>(_value);
    }
    KRYS_NODISCARD Maybe<ViewFunction> TryViewFunction() const noexcept
    {
      return IsViewFunction() ? std::make_optional(std::get<ViewFunction>(_value)) : std::nullopt;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(_value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const SingleAnimationTimeline &) const noexcept = default;

  private:
    Variant<css::keywords::Auto, css::keywords::None, css::CustomIdentifier, ScrollFunction, ViewFunction>
      _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<SingleAnimationTimeline>
  {
    KRYS_NODISCARD SingleAnimationTimeline operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SingleAnimationTimeline)
