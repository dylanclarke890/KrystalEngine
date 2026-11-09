/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Style/Values/Box/Margin.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"

#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"

namespace krys::boo::style
{
  MarginEdge CSSValueConversion<MarginEdge>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Auto {};
    }

    return operator()(state, *primitiveValue);
  }

  MarginEdge CSSValueConversion<MarginEdge>::operator()(BuilderState &state,
                                                        const css::PrimitiveValue &primitiveValue) noexcept
  {
    if (primitiveValue.ValueId() == css::ValueId::Auto)
    {
      return css::keywords::Auto {};
    }

    auto cssToLengthConversionDataWithTextZoomFactorIfNeeded =
      [](BuilderState &state, bool isFontIndependentUnit) -> css::ToLengthConversionData
    {
      auto usedZoom = 1.0f;

      if (isFontIndependentUnit)
      {
        usedZoom = EvaluationTimeZoomEnabled(state) ? 1.0f : state.Style().UsedZoom();
      }
      else
      {
        usedZoom = state.ZoomWithTextZoomFactor();
      }

      return state.CssToLengthConversionData().CopyWithAdjustedZoom(usedZoom,
                                                                    MarginEdge::Fixed::Range.ZoomOptions);
    };

    auto conversionData = state.UseSVGZoomRulesForLength()
                            ? state.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f)
                            : cssToLengthConversionDataWithTextZoomFactorIfNeeded(
                                state, primitiveValue.IsFontIndependentLength());

    if (primitiveValue.IsLength())
    {
      return MarginEdge {typename MarginEdge::Fixed {
                           css::ClampToRange<MarginEdge::Fixed::Range, float>(
                             primitiveValue.ResolveAsLength(conversionData), css::MinValueForCssLength,
                             css::MaxValueForCssLength),
                         },
                         primitiveValue.PrimitiveType() == css::UnitType::QuirkyEm};
    }

    if (primitiveValue.IsPercentage())
    {
      return MarginEdge {typename MarginEdge::Percentage {
        css::ClampToRange<MarginEdge::Percentage::Range, float>(
          primitiveValue.ResolveAsPercentage(conversionData)),
      }};
    }

    if (primitiveValue.IsCalculatedPercentageWithLength())
    {
      return MarginEdge {typename MarginEdge::Calc {
        primitiveValue.ProtectedCSSCalcValue()->CreateCalculationValue(conversionData, css::SymbolTable {})}};
    }

    state.SetCurrentPropertyInvalidAtComputedValueTime();
    return css::keywords::Auto {};
  }
}
