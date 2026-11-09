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

#include "Krystal.Booey/Style/Values/Easing/SpringEasingFunction.hpp"

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::style
{
  css::SpringEasingFunction ToCSSSpringEasingFunction(const SpringTimingFunction &function,
                                                      const RenderStyle &style) noexcept
  {
    return css::SpringEasingFunction {{
      .Mass = ConvertToCSS(Number<css::SpringEasingParameters::Positive> {function.Mass()}, style),
      .Stiffness = ConvertToCSS(Number<css::SpringEasingParameters::Positive> {function.Stiffness()}, style),
      .Damping = ConvertToCSS(Number<css::NonNegative> {function.Damping()}, style),
      .InitialVelocity = ConvertToCSS(Number<> {function.InitialVelocity()}, style),
    }};
  }

  Ref<TimingFunction> CreateTimingFunction(const BuilderState &state,
                                           const css::SpringEasingFunction &function) noexcept
  {
    return SpringTimingFunction::Create(
      ConvertToStyle(function->Mass, state).Value, ConvertToStyle(function->Stiffness, state).Value,
      ConvertToStyle(function->Damping, state).Value, ConvertToStyle(function->InitialVelocity, state).Value);
  }

  Ref<TimingFunction> CreateTimingFunctionDeprecated(const css::SpringEasingFunction &function) noexcept
  {
    if (!css::CollectComputedStyleDependencies(function).CanResolveDependenciesWithConversionData({}))
    {
      return SpringTimingFunction::Create(1, 1, 0, 0);
    }

    return SpringTimingFunction::Create(
      ConvertToStyleNoConversionDataRequired(function->Mass).Value,
      ConvertToStyleNoConversionDataRequired(function->Stiffness).Value,
      ConvertToStyleNoConversionDataRequired(function->Damping).Value,
      ConvertToStyleNoConversionDataRequired(function->InitialVelocity).Value);
  }
}
