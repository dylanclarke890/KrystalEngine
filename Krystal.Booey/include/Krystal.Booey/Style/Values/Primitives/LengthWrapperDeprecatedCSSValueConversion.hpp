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

#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Deprecated Conversions

  KRYS_NODISCARD Maybe<css::ToLengthConversionData>
    DeprecatedLengthConversionCreateCSSToLengthConversionData(RefPtr<dom::Element>) noexcept;

  template <LengthWrapperBaseDerived T>
  struct DeprecatedCSSValueConversion<T>
  {
    KRYS_NODISCARD Maybe<T> operator()(const RefPtr<dom::Element> &element,
                                       const css::PrimitiveValue &primitiveValue) noexcept
    {
      auto convertLengthPercentage = [&]() -> Maybe<T>
      {
        auto conversionData = DeprecatedLengthConversionCreateCSSToLengthConversionData(element);
        if (!conversionData)
        {
          if (primitiveValue.IsCalculated())
          {
            return null;
          }

          if (primitiveValue.IsPx())
          {
            return T {typename T::Fixed {
                        css::ClampToRange<T::Fixed::Range, float>(
                          primitiveValue.ResolveAsLengthNoConversionDataRequired(), css::MinValueForCssLength,
                          css::MaxValueForCssLength),
                      },
                      primitiveValue.PrimitiveType() == css::UnitType::QuirkyEm};
          }

          if (primitiveValue.IsPercentage())
          {
            return T {typename T::Percentage {
              css::ClampToRange<T::Percentage::Range, float>(
                primitiveValue.ResolveAsPercentageNoConversionDataRequired()),
            }};
          }

          return null;
        }

        if (primitiveValue.IsLength())
        {
          return T {
            typename T::Fixed {
              css::ClampToRange<T::Fixed::Range, float>(primitiveValue.ResolveAsLength(*conversionData),
                                                        css::MinValueForCssLength, css::MaxValueForCssLength),
            },
            primitiveValue.PrimitiveType() == css::UnitType::QuirkyEm};
        }

        if (primitiveValue.IsPercentage())
        {
          return T {typename T::Percentage {
            css::ClampToRange<T::Percentage::Range, float>(
              primitiveValue.ResolveAsPercentage(*conversionData)),
          }};
        }

        if (primitiveValue.IsCalculatedPercentageWithLength())
        {
          return T {typename T::Calc {primitiveValue.ProtectedCSSCalcValue()->CreateCalculationValue(
            *conversionData, css::SymbolTable {})}};
        }

        return null;
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
            (CSSValueConversion<T>::ProcessKeyword(keyword, valueId, result) || ...);
            return result;
          },
          keywordsTuple);

        return result;
      }
    }

    KRYS_NODISCARD Maybe<T> operator()(const RefPtr<dom::Element> &element, const css::Value &value) noexcept
    {
      auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value);
      if (!primitiveValue)
      {
        return null;
      }

      return (*this)(element, *primitiveValue);
    }
  };
}
