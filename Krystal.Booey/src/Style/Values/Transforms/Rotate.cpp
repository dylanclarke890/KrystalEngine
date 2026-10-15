#include "Krystal.Booey/Style/Values/Transforms/Rotate.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  void Rotate::Apply(TransformationMatrix &transform, const FloatSize &size) const noexcept
  {
    if (const auto &protectedValue = _value)
    {
      protectedValue->Apply(transform, size);
    }
  }

  // MARK: - Conversion

  Rotate CSSValueConversion<Rotate>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#propdef-rotate
    // none | <angle> | [ x | y | z | <number>{3} ] && <angle>

    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      krys_debug_assert(primitiveValue->ValueId() == css::ValueId::None);
      return css::keywords::None {};
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::None {};
    }

    // Only an angle was specified.
    if (list->size() == 1)
    {
      return RotateTransformFunction::Create(ConvertCSSValueToStyle<Angle<>>(state, list->item(0)),
                                             TransformFunctionBase::Type::Rotate);
    }

    // An axis identifier and angle were specified.
    if (list->size() == 2)
    {
      auto axis = list->item(0).ValueId();
      auto angle = ConvertCSSValueToStyle<Angle<>>(state, list->item(1));

      switch (axis)
      {
        case css::ValueId::X:
          return RotateTransformFunction::Create(1_css_number, 0_css_number, 0_css_number, angle,
                                                 TransformFunctionBase::Type::RotateX);
        case css::ValueId::Y:
          return RotateTransformFunction::Create(0_css_number, 1_css_number, 0_css_number, angle,
                                                 TransformFunctionBase::Type::RotateY);
        case css::ValueId::Z:
          return RotateTransformFunction::Create(0_css_number, 0_css_number, 1_css_number, angle,
                                                 TransformFunctionBase::Type::RotateZ);
        default: break;
      }
      krys_debug_assert_msg(false, "Unreachable");
      return RotateTransformFunction::Create(angle, TransformFunctionType::Rotate);
    }

    krys_debug_assert(list->size() == 4);

    // An axis vector and angle were specified.
    auto x = ConvertCSSValueToStyle<Number<>>(state, list->item(0));
    auto y = ConvertCSSValueToStyle<Number<>>(state, list->item(1));
    auto z = ConvertCSSValueToStyle<Number<>>(state, list->item(2));
    auto angle = ConvertCSSValueToStyle<Angle<>>(state, list->item(3));

    return RotateTransformFunction::Create(x, y, z, angle, TransformFunctionType::Rotate3D);
  }

  // MARK: - Blending

  Rotate Blending<Rotate>::Blend(const Rotate &from, const Rotate &to,
                                 const BlendingContext &context) noexcept
  {
    const auto &fromRotate = from._value;
    const auto &toRotate = to._value;

    if (!fromRotate && !toRotate)
    {
      return css::keywords::None {};
    }

    auto identity = [&](const auto &other)
    {
      return RotateTransformFunction::Create(0_css_deg, other.GetType());
    };

    auto fromFunction = fromRotate ? Ref<const RotateTransformFunction> {fromRotate} : identity(*toRotate);
    auto toFunction = toRotate ? Ref<const RotateTransformFunction> {toRotate} : identity(*fromRotate);

    // Ensure the two transforms have the same type.
    if (!fromFunction->IsSameType(*toFunction))
    {
      RefPtr<const RotateTransformFunction> normalizedFrom;
      RefPtr<const RotateTransformFunction> normalizedTo;
      if (fromFunction->Is3DOperation() || toFunction->Is3DOperation())
      {
        normalizedFrom =
          RotateTransformFunction::Create(fromFunction->X(), fromFunction->Y(), fromFunction->Z(),
                                          fromFunction->GetAngle(), TransformFunctionType::Rotate3D);
        normalizedTo =
          RotateTransformFunction::Create(toFunction->X(), toFunction->Y(), toFunction->Z(),
                                          toFunction->GetAngle(), TransformFunctionType::Rotate3D);
      }
      else
      {
        normalizedFrom =
          RotateTransformFunction::Create(fromFunction->GetAngle(), TransformFunctionType::Rotate);
        normalizedTo = RotateTransformFunction::Create(toFunction->GetAngle(), TransformFunctionType::Rotate);
      }
      return Blend(Rotate {Ref<const RotateTransformFunction> {krys::move(normalizedFrom)}},
                   Rotate {Ref<const RotateTransformFunction> {krys::move(normalizedTo)}}, context);
    }

    if (auto blendedFunction = toFunction->Blend(fromFunction.get(), context);
        const auto *rotate = DynamicDowncast<RotateTransformFunction>(*blendedFunction))
    {
      return RotateTransformFunction::Create(rotate->X(), rotate->Y(), rotate->Z(), rotate->GetAngle(),
                                             rotate->GetType());
    }

    return css::keywords::None {};
  }

  // MARK: - Platform

  RefPtr<TransformOperation> ToPlatform<Rotate>::operator()(const Rotate &value,
                                                            const FloatSize &size) noexcept
  {
    if (const auto &function = value._value)
    {
      return function->ToPlatform(size);
    }
    return nullptr;
  }
}
