#include "Krystal.Booey/Style/Values/Motion/OffsetRotate.hpp"

#include "Krystal.Booey/CSS/Values/OffsetRotateValue.hpp"

#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // MARK: - Conversion

  OffsetRotate CSSValueConversion<OffsetRotate>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    const css::PrimitiveValue *modifierValue = nullptr;
    const css::PrimitiveValue *angleValue = nullptr;

    if (const auto *offsetRotateValue = DynamicDowncast<css::OffsetRotateValue>(value))
    {
      modifierValue = offsetRotateValue->Modifier();
      angleValue = offsetRotateValue->GetAngle();
    }
    else if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      // Values coming from CSSTypedOM didn't go through the parser and may not have been converted to a
      // css::OffsetRotateValue.
      if (primitiveValue->ValueId() == css::ValueId::Auto
          || primitiveValue->ValueId() == css::ValueId::Reverse)
      {
        modifierValue = primitiveValue;
      }
      else if (primitiveValue->IsAngle())
      {
        angleValue = primitiveValue;
      }
    }

    Maybe<css::keywords::Auto> autoKeyword;
    auto angleInDegrees = 0.0f;

    if (angleValue)
    {
      angleInDegrees = angleValue->ResolveAsAngle<float>(state.CssToLengthConversionData());
    }

    if (modifierValue)
    {
      switch (modifierValue->ValueId())
      {
        case css::ValueId::Auto: autoKeyword = css::keywords::Auto {}; break;
        case css::ValueId::Reverse:
          autoKeyword = css::keywords::Auto {};
          angleInDegrees += 180.0f;
          break;
        default: krys_debug_assert_msg(false, "Unreachable");
      }
    }

    return OffsetRotate {autoKeyword, OffsetRotate::Angle {angleInDegrees}};
  }

  // MARK: - Blending

  bool Blending<OffsetRotate>::CanBlend(const OffsetRotate &from, const OffsetRotate &to) noexcept
  {
    return from.HasAuto() == to.HasAuto();
  }

  OffsetRotate Blending<OffsetRotate>::Blend(const OffsetRotate &from, const OffsetRotate &to,
                                             const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      return context.Progress ? to : from;
    }

    krys_debug_assert(CanBlend(from, to));
    return OffsetRotate {from.AutoKeyword(),
                         krys::boo::style::Blend(from.GetAngle(), to.GetAngle(), context)};
  }

  // MARK: - Evaluation
}
