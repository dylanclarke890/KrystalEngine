#include "Krystal.Booey/Style/Values/Transforms/Translate.hpp"

#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  TransformFunctionSizeDependencies Translate::ComputeSizeDependencies() const noexcept
  {
    if (const auto &protectedValue = _value)
    {
      return protectedValue->ComputeSizeDependencies();
    }
    return {};
  }

  void Translate::Apply(TransformationMatrix &transform, const FloatSize &size) const noexcept
  {
    if (const auto &protectedValue = _value)
    {
      protectedValue->Apply(transform, size);
    }
  }

  // MARK: - Conversion

  Translate CSSValueConversion<Translate>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#propdef-translate
    // none | <length-percentage> [ <length-percentage> <length>? ]?

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

    auto type = list->size() > 2 ? TransformFunctionType::Translate3D : TransformFunctionType::Translate;
    auto tx = ConvertCSSValueToStyle<TranslateTransformFunction::LengthPercentage>(state, list->item(0));
    auto ty = list->size() > 1
                ? ConvertCSSValueToStyle<TranslateTransformFunction::LengthPercentage>(state, list->item(1))
                : TranslateTransformFunction::LengthPercentage {0_css_px};
    auto tz = list->size() > 2
                ? ConvertCSSValueToStyle<TranslateTransformFunction::Length>(state, list->item(2))
                : TranslateTransformFunction::Length {0_css_px};

    return TranslateTransformFunction::Create(krys::move(tx), krys::move(ty), krys::move(tz), type);
  }

  // MARK: - Blending

  Translate Blending<Translate>::Blend(const Translate &from, const Translate &to,
                                       const BlendingContext &context) noexcept
  {
    const auto &fromTranslate = from._value;
    const auto &toTranslate = to._value;

    if (!fromTranslate && !toTranslate)
    {
      return css::keywords::None {};
    }

    auto identity = [&](const auto &other)
    {
      return TranslateTransformFunction::Create(0_css_px, 0_css_px, 0_css_px, other.GetType());
    };

    auto fromFunction =
      fromTranslate ? Ref<const TranslateTransformFunction> {fromTranslate} : identity(*toTranslate);
    auto toFunction =
      toTranslate ? Ref<const TranslateTransformFunction> {toTranslate} : identity(*fromTranslate);

    // Ensure the two transforms have the same type.
    if (!fromFunction->IsSameType(*toFunction))
    {
      RefPtr<const TranslateTransformFunction> normalizedFrom;
      RefPtr<const TranslateTransformFunction> normalizedTo;
      if (fromFunction->Is3DOperation() || toFunction->Is3DOperation())
      {
        normalizedFrom = TranslateTransformFunction::Create(
          fromFunction->X(), fromFunction->Y(), fromFunction->Z(), TransformFunctionType::Translate3D);
        normalizedTo = TranslateTransformFunction::Create(toFunction->X(), toFunction->Y(), toFunction->Z(),
                                                          TransformFunctionType::Translate3D);
      }
      else
      {
        normalizedFrom = TranslateTransformFunction::Create(fromFunction->X(), fromFunction->Y(),
                                                            TransformFunctionType::Translate);
        normalizedTo = TranslateTransformFunction::Create(toFunction->X(), toFunction->Y(),
                                                          TransformFunctionType::Translate);
      }
      return Blend(Translate {Ref<const TranslateTransformFunction> {krys::move(normalizedFrom)}},
                   Translate {Ref<const TranslateTransformFunction> {krys::move(normalizedTo)}}, context);
    }

    if (auto blendedFunction = toFunction->Blend(fromFunction.get(), context);
        const auto *translate = DynamicDowncast<TranslateTransformFunction>(*blendedFunction))
    {
      return TranslateTransformFunction::Create(translate->X(), translate->Y(), translate->Z(),
                                                translate->GetType());
    }

    return css::keywords::None {};
  }

  // MARK: - Platform

  RefPtr<TransformOperation> ToPlatform<Translate>::operator()(const Translate &value,
                                                               const FloatSize &size) noexcept
  {
    if (const auto &function = value._value)
    {
      return function->ToPlatform(size);
    }
    return nullptr;
  }
}
