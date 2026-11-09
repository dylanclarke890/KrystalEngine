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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/Backgrounds/LineWidth.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  KRYS_NODISCARD static LineWidth HandleKeywordValue(BuilderState &state, css::ValueId valueID) noexcept
  {
    float keywordValue;
    switch (valueID)
    {
      case css::ValueId::Thin:   keywordValue = 1.0f; break;
      case css::ValueId::Medium: keywordValue = 3.0f; break;
      case css::ValueId::Thick:  keywordValue = 5.0f; break;
      default:
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        keywordValue = 3.0f; // Medium
        break;
    }

    return LineWidth::Length {
      FloorToDevicePixel(keywordValue * state.Style().UsedZoom(), state.Style().DeviceScaleFactor())};
  }

  LineWidth CSSValueConversion<LineWidth>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return LineWidth::Length {3.0f};
    }

    if (primitiveValue->IsValueId())
    {
      return HandleKeywordValue(state, primitiveValue->ValueId());
    }

    // Any original result that was >= 1 should not be allowed to fall below 1. This keeps border lines from
    // vanishing.

    auto result = primitiveValue->ResolveAsLength<float>(state.CssToLengthConversionData());
    if (state.Style().UsedZoom() < 1.0f && result < 1.0f)
    {
      auto originalLength =
        primitiveValue->ResolveAsLength<float>(state.CssToLengthConversionData().CopyWithAdjustedZoom(1.0));
      if (originalLength >= 1.0f)
      {
        return LineWidth::Length {1.0f};
      }
    }

    if (auto minimumLineWidth = 1.0f / state.Style().DeviceScaleFactor();
        result > 0.0f && result < minimumLineWidth)
    {
      return LineWidth::Length {minimumLineWidth};
    }

    return LineWidth::Length {FloorToDevicePixel(result, state.Style().DeviceScaleFactor())};
  }

  // MARK: - Evaluate

  FloatBoxExtent Evaluation<LineWidthBox, FloatBoxExtent>::operator()(const LineWidthBox &value,
                                                                      ZoomNeeded zoom) noexcept
  {
    return {
      Evaluate<float>(value.Top(), zoom),
      Evaluate<float>(value.Right(), zoom),
      Evaluate<float>(value.Bottom(), zoom),
      Evaluate<float>(value.Left(), zoom),
    };
  }

  LayoutBoxExtent Evaluation<LineWidthBox, LayoutBoxExtent>::operator()(const LineWidthBox &value,
                                                                        ZoomNeeded zoom) noexcept
  {
    return {
      Evaluate<LayoutUnit>(value.Top(), zoom),
      Evaluate<LayoutUnit>(value.Right(), zoom),
      Evaluate<LayoutUnit>(value.Bottom(), zoom),
      Evaluate<LayoutUnit>(value.Left(), zoom),
    };
  }
}
