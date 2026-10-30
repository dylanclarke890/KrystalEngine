#include "Krystal.Booey/Style/Values/Easing/CubicBezierEasingFunction.hpp"

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
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
