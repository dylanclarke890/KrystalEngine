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

#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumericOrKeyword.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::style
{
  template <auto R, typename V, css::PrimitiveKeyword... Ks>
  struct ToCSS<PrimitiveNumericOrKeyword<LengthPercentage<R, V>, Ks...>>
  {
    using Result = css::PrimitiveNumericOrKeyword<css::LengthPercentage<R, V>, Ks...>;

    KRYS_NODISCARD Result operator()(const PrimitiveNumericOrKeyword<LengthPercentage<R, V>, Ks...> &value,
                                     const RenderStyle &style) noexcept
    {
      return krys::SwitchOn(
        value,
        [&](const typename LengthPercentage<R, V>::Dimension &length) -> Result
        {
          if constexpr (R.ZoomOptions == css::RangeZoomOptions::Default)
          {
            return css::LengthPercentageRaw<R, V> {length.Unit,
                                                   AdjustForZoom(length.UnresolvedValue(), style)};
          }
          else if constexpr (R.ZoomOptions == css::RangeZoomOptions::Unzoomed)
          {
            if (EvaluationTimeZoomEnabled(style))
            {
              return css::LengthPercentageRaw<R, V> {length.Unit, length.UnresolvedValue()};
            }

            return css::LengthPercentageRaw<R, V> {length.Unit,
                                                   AdjustForZoom(length.UnresolvedValue(), style)};
          }
        },
        [&](const typename LengthPercentage<R, V>::Percentage &percentage) -> Result
        { return css::LengthPercentageRaw<R, V> {percentage.Unit, percentage.Value}; },
        [&](const typename LengthPercentage<R, V>::Calc &calculation) -> Result
        {
          return css::UnevaluatedCalc<css::LengthPercentageRaw<R, V>> {
            css::calc::Value::Create(*calculation.ProtectedCalculation(), style)};
        },
        [&]<css::ValueId Id>(const css::KeywordValueConstant<Id> &identifier) -> Result
        { return ConvertToCSS(identifier, style); });
    }
  };

  template <css::Numeric N, css::PrimitiveKeyword... Ks>
  struct ToStyle<css::PrimitiveNumericOrKeyword<N, Ks...>>
  {
    using From = css::PrimitiveNumericOrKeyword<N, Ks...>;
    using To = typename ToStyleMapping<From>::type;

    template <typename... Rest>
    KRYS_NODISCARD To operator()(const From &value, Rest &&...rest) noexcept
    {
      return krys::SwitchOn(value, [&](const auto &alternative) -> To
                            { return ConvertToStyle(alternative, std::forward<Rest>(rest)...); });
    }

    // Implement `BuilderState` overload to explicitly forward to the `ToLengthConversionData` overload.
    template <typename... Rest>
    KRYS_NODISCARD To operator()(const From &value, const BuilderState &state, Rest &&...rest) noexcept
    {
      return ConvertToStyle(value, ConversionData<typename N::Raw>(state), std::forward<Rest>(rest)...);
    }
  };
}
