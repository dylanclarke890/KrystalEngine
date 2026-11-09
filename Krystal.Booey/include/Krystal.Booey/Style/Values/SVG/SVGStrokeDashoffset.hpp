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

namespace krys::boo::style
{
  struct SVGStrokeDashoffsetLength : LengthWrapperBase<LengthPercentage<>>
  {
    using Base::Base;
  };

  // <'stroke-dashoffset'> = <length-percentage> | <number>@(converted-to-px)
  // https://svgwg.org/svg2-draft/painting.html#StrokeDashoffsetProperty
  struct SVGStrokeDashoffset
  {
    using Fixed = SVGStrokeDashoffsetLength::Fixed;
    using Percentage = SVGStrokeDashoffsetLength::Percentage;
    using Calc = SVGStrokeDashoffsetLength::Calc;

    SVGStrokeDashoffsetLength Value;

    SVGStrokeDashoffset(SVGStrokeDashoffsetLength &&length) noexcept : Value {krys::move(length)}
    {
    }
    SVGStrokeDashoffset(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept
        : Value {Fixed {static_cast<float>(literal.Value)}}
    {
    }
    SVGStrokeDashoffset(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept : Value {literal}
    {
    }
    SVGStrokeDashoffset(css::ValueLiteral<css::PercentageUnit::Percentage> literal) noexcept : Value {literal}
    {
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(Value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const SVGStrokeDashoffset &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(SVGStrokeDashoffset, Value);

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<SVGStrokeDashoffset>
  {
    KRYS_NODISCARD SVGStrokeDashoffset operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SVGStrokeDashoffsetLength)
DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::SVGStrokeDashoffset)
