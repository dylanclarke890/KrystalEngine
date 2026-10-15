#include "Krystal.Booey/Style/Values/Easing/StepsEasingFunction.hpp"

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
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
