#include "Krystal.Booey/Style/Values/Easing/EasingFunction.hpp"

#include "Krystal.Booey/CSS/Values/Easing/EasingFunction.hpp"
#include "Krystal.Booey/CSS/Values/EasingFunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/Values/Easing/CubicBezierEasingFunction.hpp"
#include "Krystal.Booey/Style/Values/Easing/LinearEasingFunction.hpp"
#include "Krystal.Booey/Style/Values/Easing/SpringEasingFunction.hpp"
#include "Krystal.Booey/Style/Values/Easing/StepsEasingFunction.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style
{
  KRYS_NODISCARD static css::EasingFunction ToCSSEasingFunction(const TimingFunction &function,
                                                                const RenderStyle &style) noexcept
  {
    switch (function.GetType())
    {
      case TimingFunction::Type::CubicBezierFunction:
      {
        const auto &cubicBezierFunction = Downcast<CubicBezierTimingFunction>(function);
        switch (cubicBezierFunction.GetTimingFunctionPreset())
        {
          case CubicBezierTimingFunction::TimingFunctionPreset::Ease:    return {css::keywords::Ease {}};
          case CubicBezierTimingFunction::TimingFunctionPreset::EaseIn:  return {css::keywords::EaseIn {}};
          case CubicBezierTimingFunction::TimingFunctionPreset::EaseOut: return {css::keywords::EaseOut {}};
          case CubicBezierTimingFunction::TimingFunctionPreset::EaseInOut:
            return {css::keywords::EaseInOut {}};
          case CubicBezierTimingFunction::TimingFunctionPreset::Custom: break;
        }
        return {ToCSSCubicBezierEasingFunction(cubicBezierFunction, style)};
      }

      case TimingFunction::Type::LinearFunction:
      {
        const auto &linearFunction = Downcast<LinearTimingFunction>(function);
        if (linearFunction.Points().empty())
        {
          return {css::keywords::Linear {}};
        }
        return {ToCSSLinearEasingFunction(linearFunction, style)};
      }

      case TimingFunction::Type::StepsFunction:
        return {ToCSSStepsEasingFunction(Downcast<StepsTimingFunction>(function), style)};

      case TimingFunction::Type::SpringFunction:
        return {ToCSSSpringEasingFunction(Downcast<SpringTimingFunction>(function), style)};
    }

    krys_unreachable();
  }

  KRYS_NODISCARD static Ref<TimingFunction> CreateTimingFunction(BuilderState &state,
                                                                 const css::EasingFunction &function) noexcept
  {
    return krys::SwitchOn(
      function.Value,
      [&](const css::keywords::Linear &) -> Ref<TimingFunction> { return LinearTimingFunction::Create(); },
      [&](const css::LinearEasingFunction &function) -> Ref<TimingFunction>
      { return CreateTimingFunction(state, function); },
      [&](const css::keywords::Ease &) -> Ref<TimingFunction> { return CubicBezierTimingFunction::Create(); },
      [&](const css::keywords::EaseIn &) -> Ref<TimingFunction>
      { return CubicBezierTimingFunction::Create(CubicBezierTimingFunction::TimingFunctionPreset::EaseIn); },
      [&](const css::keywords::EaseOut &) -> Ref<TimingFunction>
      { return CubicBezierTimingFunction::Create(CubicBezierTimingFunction::TimingFunctionPreset::EaseOut); },
      [&](const css::keywords::EaseInOut &) -> Ref<TimingFunction>
      {
        return CubicBezierTimingFunction::Create(CubicBezierTimingFunction::TimingFunctionPreset::EaseInOut);
      },
      [&](const css::CubicBezierEasingFunction &function) -> Ref<TimingFunction>
      { return CreateTimingFunction(state, function); },
      [&](const css::keywords::StepStart &) -> Ref<TimingFunction>
      { return StepsTimingFunction::Create(1, StepsTimingFunction::StepPosition::Start); },
      [&](const css::keywords::StepEnd &) -> Ref<TimingFunction>
      { return StepsTimingFunction::Create(1, StepsTimingFunction::StepPosition::End); },
      [&](const css::StepsEasingFunction &function) -> Ref<TimingFunction>
      { return CreateTimingFunction(state, function); },
      [&](const css::SpringEasingFunction &function) -> Ref<TimingFunction>
      { return CreateTimingFunction(state, function); });
  }

  Ref<TimingFunction> CreateTimingFunctionDeprecated(const css::EasingFunction &function) noexcept
  {
    return krys::SwitchOn(
      function.Value,
      [&](const css::keywords::Linear &) -> Ref<TimingFunction> { return LinearTimingFunction::Create(); },
      [&](const css::LinearEasingFunction &function) -> Ref<TimingFunction>
      { return CreateTimingFunctionDeprecated(function); },
      [&](const css::keywords::Ease &) -> Ref<TimingFunction> { return CubicBezierTimingFunction::Create(); },
      [&](const css::keywords::EaseIn &) -> Ref<TimingFunction>
      { return CubicBezierTimingFunction::Create(CubicBezierTimingFunction::TimingFunctionPreset::EaseIn); },
      [&](const css::keywords::EaseOut &) -> Ref<TimingFunction>
      { return CubicBezierTimingFunction::Create(CubicBezierTimingFunction::TimingFunctionPreset::EaseOut); },
      [&](const css::keywords::EaseInOut &) -> Ref<TimingFunction>
      {
        return CubicBezierTimingFunction::Create(CubicBezierTimingFunction::TimingFunctionPreset::EaseInOut);
      },
      [&](const css::CubicBezierEasingFunction &function) -> Ref<TimingFunction>
      { return CreateTimingFunctionDeprecated(function); },
      [&](const css::keywords::StepStart &) -> Ref<TimingFunction>
      { return StepsTimingFunction::Create(1, StepsTimingFunction::StepPosition::Start); },
      [&](const css::keywords::StepEnd &) -> Ref<TimingFunction>
      { return StepsTimingFunction::Create(1, StepsTimingFunction::StepPosition::End); },
      [&](const css::StepsEasingFunction &function) -> Ref<TimingFunction>
      { return CreateTimingFunctionDeprecated(function); },
      [&](const css::SpringEasingFunction &function) -> Ref<TimingFunction>
      { return CreateTimingFunctionDeprecated(function); });
  }

  KRYS_NODISCARD static Ref<TimingFunction> CreateTimingFunctionFromValueID(BuilderState &state,
                                                                            css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Linear: return LinearTimingFunction::Create();
      case css::ValueId::Ease:   return CubicBezierTimingFunction::Create();
      case css::ValueId::EaseIn:
        return CubicBezierTimingFunction::Create(CubicBezierTimingFunction::TimingFunctionPreset::EaseIn);
      case css::ValueId::EaseOut:
        return CubicBezierTimingFunction::Create(CubicBezierTimingFunction::TimingFunctionPreset::EaseOut);
      case css::ValueId::EaseInOut:
        return CubicBezierTimingFunction::Create(CubicBezierTimingFunction::TimingFunctionPreset::EaseInOut);
      case css::ValueId::StepStart:
        return StepsTimingFunction::Create(1, StepsTimingFunction::StepPosition::Start);
      case css::ValueId::StepEnd:
        return StepsTimingFunction::Create(1, StepsTimingFunction::StepPosition::End);
      default: state.SetCurrentPropertyInvalidAtComputedValueTime(); return LinearTimingFunction::Create();
    }
  }

  KRYS_NODISCARD static RefPtr<TimingFunction>
    CreateTimingFunctionFromValueIDDeprecated(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Linear: return LinearTimingFunction::Create();
      case css::ValueId::Ease:   return CubicBezierTimingFunction::Create();
      case css::ValueId::EaseIn:
        return CubicBezierTimingFunction::Create(CubicBezierTimingFunction::TimingFunctionPreset::EaseIn);
      case css::ValueId::EaseOut:
        return CubicBezierTimingFunction::Create(CubicBezierTimingFunction::TimingFunctionPreset::EaseOut);
      case css::ValueId::EaseInOut:
        return CubicBezierTimingFunction::Create(CubicBezierTimingFunction::TimingFunctionPreset::EaseInOut);
      case css::ValueId::StepStart:
        return StepsTimingFunction::Create(1, StepsTimingFunction::StepPosition::Start);
      case css::ValueId::StepEnd:
        return StepsTimingFunction::Create(1, StepsTimingFunction::StepPosition::End);
      default: return nullptr;
    }
  }

  RefPtr<TimingFunction> CreateTimingFunctionDeprecated(const css::Value &value) noexcept
  {
    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      return CreateTimingFunctionFromValueIDDeprecated(primitiveValue->ValueId());
    }
    if (const auto *easingFunctionValue = DynamicDowncast<css::EasingFunctionValue>(value))
    {
      return CreateTimingFunctionDeprecated(easingFunctionValue->GetEasingFunction());
    }
    return {};
  }

  // MARK: - Conversion

  EasingFunction CSSValueConversion<EasingFunction>::operator()(BuilderState &state,
                                                                const css::Value &value) noexcept
  {
    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      return {CreateTimingFunctionFromValueID(state, primitiveValue->ValueId())};
    }
    if (const auto *easingFunctionValue = DynamicDowncast<css::EasingFunctionValue>(value))
    {
      return {CreateTimingFunction(state, easingFunctionValue->GetEasingFunction())};
    }

    state.SetCurrentPropertyInvalidAtComputedValueTime();
    return {LinearTimingFunction::Create()};
  }

  Ref<css::Value> CSSValueCreation<EasingFunction>::operator()(css::ValuePool &, const RenderStyle &style,
                                                               const EasingFunction &value) noexcept
  {
    return css::EasingFunctionValue::Create(ToCSSEasingFunction(*value.Value, style));
  }

  // MARK: - Serialization

  void Serialiser<EasingFunction>::operator()(css::CSSOMString &builder,
                                              const css::SerialisationContext &context,
                                              const RenderStyle &style, const EasingFunction &value) noexcept
  {
    // FIXME: Optimize by avoiding CSSEasingFunction conversion.
    css::Serialise(builder, context, ToCSSEasingFunction(*value.Value, style));
  }
}
