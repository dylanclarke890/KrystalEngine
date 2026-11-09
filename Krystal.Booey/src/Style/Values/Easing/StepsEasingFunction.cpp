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

#include "Krystal.Booey/Style/Values/Easing/StepsEasingFunction.hpp"

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::style
{
  css::StepsEasingFunction ToCSSStepsEasingFunction(const StepsTimingFunction &function,
                                                    const RenderStyle &) noexcept
  {
    auto position = function.GetStepPosition();
    if (!position)
    {
      return {{css::StepsEasingParameters::JumpEnd {{function.NumberOfSteps()}}}};
    }

    switch (*position)
    {
      case StepsTimingFunction::StepPosition::JumpStart:
        return {{css::StepsEasingParameters::JumpStart {{function.NumberOfSteps()}}}};

      case StepsTimingFunction::StepPosition::JumpEnd:
        return {{css::StepsEasingParameters::JumpEnd {{function.NumberOfSteps()}}}};

      case StepsTimingFunction::StepPosition::JumpNone:
        return {{css::StepsEasingParameters::JumpNone {{function.NumberOfSteps()}}}};

      case StepsTimingFunction::StepPosition::JumpBoth:
        return {{css::StepsEasingParameters::JumpBoth {{function.NumberOfSteps()}}}};

      case StepsTimingFunction::StepPosition::Start:
        return {{css::StepsEasingParameters::Start {{function.NumberOfSteps()}}}};

      case StepsTimingFunction::StepPosition::End:
        return {{css::StepsEasingParameters::End {{function.NumberOfSteps()}}}};
    }

    krys_unreachable();
  }

  KRYS_NODISCARD static StepsTimingFunction::StepPosition ToStepPosition(css::keywords::JumpStart) noexcept
  {
    return StepsTimingFunction::StepPosition::JumpStart;
  }

  KRYS_NODISCARD static StepsTimingFunction::StepPosition ToStepPosition(css::keywords::JumpEnd) noexcept
  {
    return StepsTimingFunction::StepPosition::JumpEnd;
  }

  KRYS_NODISCARD static StepsTimingFunction::StepPosition ToStepPosition(css::keywords::JumpBoth) noexcept
  {
    return StepsTimingFunction::StepPosition::JumpBoth;
  }

  KRYS_NODISCARD static StepsTimingFunction::StepPosition ToStepPosition(css::keywords::Start) noexcept
  {
    return StepsTimingFunction::StepPosition::Start;
  }

  KRYS_NODISCARD static StepsTimingFunction::StepPosition ToStepPosition(css::keywords::End) noexcept
  {
    return StepsTimingFunction::StepPosition::End;
  }

  KRYS_NODISCARD static StepsTimingFunction::StepPosition ToStepPosition(css::keywords::JumpNone) noexcept
  {
    return StepsTimingFunction::StepPosition::JumpNone;
  }

  Ref<TimingFunction> CreateTimingFunction(const BuilderState &state,
                                           const css::StepsEasingFunction &function) noexcept
  {
    return krys::SwitchOn(function->Value,
                          [&](const auto &value) -> Ref<TimingFunction>
                          {
                            return StepsTimingFunction::Create(ConvertToStyle(value.Steps, state).Value,
                                                               ToStepPosition(value.Keyword));
                          });
  }

  Ref<TimingFunction> CreateTimingFunctionDeprecated(const css::StepsEasingFunction &function) noexcept
  {
    if (!css::CollectComputedStyleDependencies(function).CanResolveDependenciesWithConversionData({}))
    {
      return StepsTimingFunction::Create();
    }

    return krys::SwitchOn(function->Value,
                          [&](const auto &value) -> Ref<TimingFunction>
                          {
                            return StepsTimingFunction::Create(
                              ConvertToStyleNoConversionDataRequired(value.Steps).Value,
                              ToStepPosition(value.Keyword));
                          });
  }
}
