#include "Krystal.Booey/Style/Values/Text/TextIndent.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  TextIndent CSSValueConversion<TextIndent>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      return ConvertCSSValueToStyle<TextIndentLength>(state, *primitiveValue);
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return 0_css_px;
    }

    Maybe<TextIndentLength> length;
    Maybe<css::keywords::Hanging> hanging;
    Maybe<css::keywords::EachLine> eachLine;

    for (const auto &primitiveValue : *list)
    {
      if (primitiveValue.ValueId() == css::ValueId::Hanging)
      {
        hanging = css::keywords::Hanging {};
      }
      else if (primitiveValue.ValueId() == css::ValueId::EachLine)
      {
        eachLine = css::keywords::EachLine {};
      }
      else
      {
        length = ConvertCSSValueToStyle<TextIndentLength>(state, primitiveValue);
      }
    }

    if (!length)
    {
      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return 0_css_px;
    }

    return TextIndent {krys::move(*length), hanging, eachLine};
  }

  // MARK: - Blending

  bool Blending<TextIndent>::CanBlend(const TextIndent &a, const TextIndent &b) noexcept
  {
    return a.Hanging == b.Hanging && a.EachLine == b.EachLine && style::CanBlend(a.Length, b.Length);
  }

  TextIndent Blending<TextIndent>::Blend(const TextIndent &a, const TextIndent &b,
                                         const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    krys_debug_assert(a.Hanging == b.Hanging);
    krys_debug_assert(a.EachLine == b.EachLine);
    return TextIndent {style::Blend(a.Length, b.Length, context), a.Hanging, a.EachLine};
  }
}
