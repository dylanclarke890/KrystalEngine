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

#include "Krystal.Booey/Style/Values/Easing/CubicBezierEasingFunction.hpp"

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::style
{
  css::CubicBezierEasingFunction ToCSSCubicBezierEasingFunction(const CubicBezierTimingFunction &function,
                                                                const RenderStyle &style) noexcept
  {
    krys_debug_assert(function.GetTimingFunctionPreset()
                      == CubicBezierTimingFunction::TimingFunctionPreset::Custom);

    return css::CubicBezierEasingFunction {{{
      css::CubicBezierEasingParameters::Coordinate {
        ConvertToCSS(Number<css::ClosedUnitRange> {function.X1()}, style),
        ConvertToCSS(Number<> {function.Y1()}, style)},
      css::CubicBezierEasingParameters::Coordinate {
        ConvertToCSS(Number<css::ClosedUnitRange> {function.X2()}, style),
        ConvertToCSS(Number<> {function.Y2()}, style)},
    }}};
  }

  Ref<TimingFunction> CreateTimingFunction(const BuilderState &state,
                                           const css::CubicBezierEasingFunction &function) noexcept
  {
    return CubicBezierTimingFunction::Create(ConvertToStyle(get<0>(get<0>(function->Value)), state).Value,
                                             ConvertToStyle(get<1>(get<0>(function->Value)), state).Value,
                                             ConvertToStyle(get<0>(get<1>(function->Value)), state).Value,
                                             ConvertToStyle(get<1>(get<1>(function->Value)), state).Value);
  }

  Ref<TimingFunction> CreateTimingFunctionDeprecated(const css::CubicBezierEasingFunction &function) noexcept
  {
    if (!css::CollectComputedStyleDependencies(function).CanResolveDependenciesWithConversionData({}))
    {
      return CubicBezierTimingFunction::Create();
    }

    return CubicBezierTimingFunction::Create(
      ConvertToStyleNoConversionDataRequired(get<0>(get<0>(function->Value))).Value,
      ConvertToStyleNoConversionDataRequired(get<1>(get<0>(function->Value))).Value,
      ConvertToStyleNoConversionDataRequired(get<0>(get<1>(function->Value))).Value,
      ConvertToStyleNoConversionDataRequired(get<1>(get<1>(function->Value))).Value);
  }
}
