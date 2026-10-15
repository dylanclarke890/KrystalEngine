#include "Krystal.Booey/Style/Values/Easing/SpringEasingFunction.hpp"

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"
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
