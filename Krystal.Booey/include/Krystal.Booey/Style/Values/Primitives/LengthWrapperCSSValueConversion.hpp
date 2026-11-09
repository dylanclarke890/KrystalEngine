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

#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  template <LengthWrapperBaseDerived T>
  struct CSSValueConversion<T>
  {
    template <typename K>
    static bool ProcessKeyword(const K &keyword, css::ValueId valueId, Maybe<T> &result) noexcept
    {
      if (valueId == keyword.Value)
      {
        result = T {keyword};
        return true;
      }

      return false;
    }

    KRYS_NODISCARD static css::ToLengthConversionData
      SelectConversionData(BuilderState &builderState) noexcept
    {
      if constexpr (T::Fixed::Range.ZoomOptions == css::RangeZoomOptions::Default)
      {
        return builderState.UseSVGZoomRulesForLength()
                 ? builderState.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f)
                 : builderState.CssToLengthConversionData();
      }
      else if constexpr (T::Fixed::Range.ZoomOptions == css::RangeZoomOptions::Unzoomed)
      {
        if (EvaluationTimeZoomEnabled(builderState))
        {
          return builderState.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f,
                                                                               T::Fixed::Range.ZoomOptions);
        }

        return builderState.UseSVGZoomRulesForLength()
                 ? builderState.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f)
                 : builderState.CssToLengthConversionData();
      }
    }

    KRYS_NODISCARD T operator()(BuilderState &state, const css::PrimitiveValue &primitiveValue) noexcept
    {
      using namespace css::Literals;

      auto convertLengthPercentage = [&]() -> T
      {
        auto conversionData = SelectConversionData(state);

        if (primitiveValue.IsLength())
        {
          return T {
            typename T::Fixed {
              css::ClampToRange<T::Fixed::Range, float>(primitiveValue.ResolveAsLength(conversionData),
                                                        css::MinValueForCssLength, css::MaxValueForCssLength),
            },
            primitiveValue.PrimitiveType() == css::UnitType::QuirkyEm};
        }

        if (primitiveValue.IsPercentage())
        {
          return T {typename T::Percentage {
            css::ClampToRange<T::Percentage::Range, float>(
              primitiveValue.ResolveAsPercentage(conversionData)),
          }};
        }

        if (primitiveValue.IsCalculatedPercentageWithLength())
        {
          return T {typename T::Calc {primitiveValue.ProtectedCSSCalcValue()->CreateCalculationValue(
            conversionData, css::SymbolTable {})}};
        }

        krys_debug_assert_msg(false, "Unexpected length-percentage value");
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return 0_css_px;
      };

      if constexpr (!T::Keywords::Count)
      {
        return convertLengthPercentage();
      }
      else
      {
        auto valueId = primitiveValue.ValueId();
        if (valueId == css::ValueId::Invalid)
        {
          return convertLengthPercentage();
        }

        constexpr auto keywordsTuple = T::Keywords::Tuple;

        auto result = std::apply(
          [&](const auto &...keyword)
          {
            Maybe<T> result;
            (ProcessKeyword(keyword, valueId, result) || ...);
            return result;
          },
          keywordsTuple);

        if (result)
        {
          return *result;
        }

        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return 0_css_px;
      }
    }

    KRYS_NODISCARD T operator()(BuilderState &state, const css::Value &value) noexcept
    {
      using namespace css::Literals;

      auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
      if (!primitiveValue)
      {
        return 0_css_px;
      }

      return (*this)(state, *primitiveValue);
    }
  };
}
