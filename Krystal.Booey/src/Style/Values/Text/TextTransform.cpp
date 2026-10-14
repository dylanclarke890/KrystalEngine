#include "Krystal.Booey/Style/Values/Text/TextTransform.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"

namespace krys::boo::style
{
  TextTransform CSSValueConversion<TextTransform>::operator()(BuilderState &state,
                                                              const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::None:         return css::keywords::None {};
        case css::ValueId::Capitalize:   return {TextTransformValue::Capitalize};
        case css::ValueId::Uppercase:    return {TextTransformValue::Uppercase};
        case css::ValueId::Lowercase:    return {TextTransformValue::Lowercase};
        case css::ValueId::FullWidth:    return {TextTransformValue::FullWidth};
        case css::ValueId::FullSizeKana: return {TextTransformValue::FullSizeKana};
        case css::ValueId::MathAuto:     return {TextTransformValue::MathAuto};
        default:                         state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::None {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::None {};
    }

    TextTransformValueEnumSet result;
    for (const auto &item : *list)
    {
      switch (item.ValueId())
      {
        case css::ValueId::Capitalize:
          if (result.Value.ContainsAny({TextTransformValue::Uppercase, TextTransformValue::Lowercase}))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::None {};
          }
          result.Value.Add(TextTransformValue::Capitalize);
          break;
        case css::ValueId::Uppercase:
          if (result.Value.ContainsAny({TextTransformValue::Capitalize, TextTransformValue::Lowercase}))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::None {};
          }
          result.Value.Add(TextTransformValue::Uppercase);
          break;
        case css::ValueId::Lowercase:
          if (result.Value.ContainsAny({TextTransformValue::Capitalize, TextTransformValue::Uppercase}))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::None {};
          }
          result.Value.Add(TextTransformValue::Lowercase);
          break;
        case css::ValueId::FullWidth:    result.Value.Add(TextTransformValue::FullWidth); break;
        case css::ValueId::FullSizeKana: result.Value.Add(TextTransformValue::FullSizeKana); break;
        default:                         state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::None {};
      }
    }
    return result;
  }
}
