#include "Krystal.Booey/Style/Values/Speech/SpeakAs.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"

namespace krys::boo::style
{
  SpeakAs CSSValueConversion<SpeakAs>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::None:
        case css::ValueId::Normal:             return css::keywords::Normal {};
        case css::ValueId::SpellOut:           return {SpeakAsValue::SpellOut};
        case css::ValueId::Digits:             return {SpeakAsValue::Digits};
        case css::ValueId::LiteralPunctuation: return {SpeakAsValue::LiteralPunctuation};
        case css::ValueId::NoPunctuation:      return {SpeakAsValue::NoPunctuation};
        default:                               state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::Normal {};
    }

    SpeakAsValueEnumSet result;
    for (const auto &item : *list)
    {
      switch (item.ValueId())
      {
        case css::ValueId::SpellOut: result.Value.Add(SpeakAsValue::SpellOut); break;
        case css::ValueId::Digits:   result.Value.Add(SpeakAsValue::Digits); break;
        case css::ValueId::LiteralPunctuation:
          if (result.Contains(SpeakAsValue::NoPunctuation))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::Normal {};
          }
          result.Value.Add(SpeakAsValue::LiteralPunctuation);
          break;
        case css::ValueId::NoPunctuation:
          if (result.Contains(SpeakAsValue::LiteralPunctuation))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::Normal {};
          }
          result.Value.Add(SpeakAsValue::NoPunctuation);
          break;
        default: state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    }
    return result;
  }
}
