/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthResolution.hpp"
#include "Krystal.Core/Maths/Extras.hpp"

namespace krys::boo::style
{
  // MARK: Length Canonicalization

  double CanonicalizeLength(double value, css::LengthUnit unit, css::NoConversionDataRequiredTag) noexcept
  {
    return ComputeNonCalcLengthDouble(value, unit, {});
  }

  double CanonicalizeLength(double value, css::LengthUnit unit,
                            const css::ToLengthConversionData &conversionData) noexcept
  {
    return ComputeNonCalcLengthDouble(value, unit, conversionData);
  }

  float ClampLengthToAllowedLimits(double value) noexcept
  {
    return ClampTo<float>(static_cast<float>(value), css::MinValueForCssLength, css::MaxValueForCssLength);
  }

  float CanonicalizeAndClampLength(double value, css::LengthUnit unit,
                                   css::NoConversionDataRequiredTag token) noexcept
  {
    return ClampLengthToAllowedLimits(CanonicalizeLength(value, unit, token));
  }

  float CanonicalizeAndClampLength(double value, css::LengthUnit unit,
                                   const css::ToLengthConversionData &conversionData) noexcept
  {
    return ClampLengthToAllowedLimits(CanonicalizeLength(value, unit, conversionData));
  }

  // MARK: ToCSS utilities

  float AdjustForZoom(float value, const RenderStyle &style) noexcept
  {
    return AdjustFloatForAbsoluteZoom(value, style);
  }

  bool EvaluationTimeZoomEnabled(const RenderStyle &style) noexcept
  {
    return style.EvaluationTimeZoomEnabled();
  }

  bool EvaluationTimeZoomEnabled(const BuilderState &state) noexcept
  {
    return state.GetDocument().GetSettings().Values.EvaluationTimeZoomEnabled;
  }
}
