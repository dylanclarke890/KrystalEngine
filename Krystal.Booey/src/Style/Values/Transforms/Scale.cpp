#include "Krystal.Booey/Style/Values/Transforms/Scale.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  void Scale::Apply(TransformationMatrix &transform, const FloatSize &size) const noexcept
  {
    if (const auto &protectedValue = _value)
    {
      protectedValue->Apply(transform, size);
    }
  }

  // MARK: - Conversion

  Scale CSSValueConversion<Scale>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#propdef-scale
    // none | [ <number> | <percentage> ]{1,3}

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

    auto sx = ConvertCSSValueToStyle<NumberOrPercentageResolvedToNumber<>>(state, list->item(0));
    auto sy = list->size() > 1
                ? ConvertCSSValueToStyle<NumberOrPercentageResolvedToNumber<>>(state, list->item(1))
                : sx;
    auto sz = list->size() > 2
                ? ConvertCSSValueToStyle<NumberOrPercentageResolvedToNumber<>>(state, list->item(2))
                : NumberOrPercentageResolvedToNumber<> {1_css_number};

    return ScaleTransformFunction::Create(sx, sy, sz, TransformFunctionType::Scale);
  }

  // MARK: - Blending

  Scale Blending<Scale>::Blend(const Scale &from, const Scale &to, const BlendingContext &context) noexcept
  {
    const auto &fromScale = from._value;
    const auto &toScale = to._value;

    if (!fromScale && !toScale)
    {
      return css::keywords::None {};
    }

    auto identity = [&](const auto &other)
    {
      return ScaleTransformFunction::Create(1_css_number, 1_css_number, 1_css_number, other.GetType());
    };

    auto fromFunction = fromScale ? Ref<const ScaleTransformFunction> {fromScale} : identity(*toScale);
    auto toFunction = toScale ? Ref<const ScaleTransformFunction> {toScale} : identity(*fromScale);

    // Ensure the two transforms have the same type.
    if (!fromFunction->IsSameType(*toFunction))
    {
      RefPtr<const ScaleTransformFunction> normalizedFrom;
      RefPtr<const ScaleTransformFunction> normalizedTo;
      if (fromFunction->Is3DOperation() || toFunction->Is3DOperation())
      {
        normalizedFrom = ScaleTransformFunction::Create(fromFunction->X(), fromFunction->Y(),
                                                        fromFunction->Z(), TransformFunctionType::Scale3D);
        normalizedTo = ScaleTransformFunction::Create(toFunction->X(), toFunction->Y(), toFunction->Z(),
                                                      TransformFunctionType::Scale3D);
      }
      else
      {
        normalizedFrom =
          ScaleTransformFunction::Create(fromFunction->X(), fromFunction->Y(), TransformFunctionType::Scale);
        normalizedTo =
          ScaleTransformFunction::Create(toFunction->X(), toFunction->Y(), TransformFunctionType::Scale);
      }
      return Blend(Scale {Ref<const ScaleTransformFunction> {krys::move(normalizedFrom)}},
                   Scale {Ref<const ScaleTransformFunction> {krys::move(normalizedTo)}}, context);
    }

    if (auto blendedFunction = toFunction->Blend(fromFunction.get(), context);
        const auto *scale = DynamicDowncast<ScaleTransformFunction>(*blendedFunction))
    {
      return ScaleTransformFunction::Create(scale->X(), scale->Y(), scale->Z(), scale->GetType());
    }

    return css::keywords::None {};
  }

  // MARK: - Platform

  RefPtr<TransformOperation> ToPlatform<Scale>::operator()(const Scale &value, const FloatSize &size) noexcept
  {
    if (const auto &function = value._value)
    {
      return function->ToPlatform(size);
    }
    return nullptr;
  }
}
