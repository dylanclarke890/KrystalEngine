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

#include "Krystal.Booey/Style/Values/Text/WordSpacing.hpp"

#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"

namespace krys::boo::style
{
  WordSpacing CSSValueConversion<WordSpacing>::operator()(BuilderState &state,
                                                          const css::Value &value) noexcept
  {
    auto cssToLengthConversionDataWithTextZoomFactor = [](BuilderState &state) -> css::ToLengthConversionData
    {
      auto zoom = state.ZoomWithTextZoomFactor();
      if (zoom == state.CssToLengthConversionData().Zoom())
      {
        return state.CssToLengthConversionData();
      }
      return state.CssToLengthConversionData().CopyWithAdjustedZoom(zoom,
                                                                    WordSpacing::Fixed::Range.ZoomOptions);
    };

    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Normal {};
    }

    if (primitiveValue->ValueId() == css::ValueId::Normal)
    {
      return css::keywords::Normal {};
    }

    auto conversionData = state.UseSVGZoomRulesForLength()
                            ? state.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f)
                            : cssToLengthConversionDataWithTextZoomFactor(state);

    if (primitiveValue->IsLength())
    {
      return WordSpacing {typename WordSpacing::Fixed {
                            css::ClampToRange<WordSpacing::Fixed::Range, float>(
                              primitiveValue->ResolveAsLength(conversionData), css::MinValueForCssLength,
                              css::MaxValueForCssLength),
                          },
                          primitiveValue->PrimitiveType() == css::UnitType::QuirkyEm};
    }

    if (primitiveValue->IsPercentage())
    {
      return WordSpacing {typename WordSpacing::Percentage {
        css::ClampToRange<WordSpacing::Percentage::Range, float>(
          primitiveValue->ResolveAsPercentage(conversionData)),
      }};
    }

    if (primitiveValue->IsCalculatedPercentageWithLength())
    {
      return WordSpacing {
        typename WordSpacing::Calc {primitiveValue->ProtectedCSSCalcValue()->CreateCalculationValue(
          conversionData, css::SymbolTable {})}};
    }

    state.SetCurrentPropertyInvalidAtComputedValueTime();
    return css::keywords::Normal {};
  }
}
