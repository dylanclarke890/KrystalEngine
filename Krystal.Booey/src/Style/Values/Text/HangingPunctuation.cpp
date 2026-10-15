#include "Krystal.Booey/Style/Values/Text/HangingPunctuation.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"

namespace krys::boo::style
{
  HangingPunctuation CSSValueConversion<HangingPunctuation>::operator()(BuilderState &state,
                                                                        const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::None:     return css::keywords::None {};
        case css::ValueId::First:    return {HangingPunctuationValue::First};
        case css::ValueId::ForceEnd: return {HangingPunctuationValue::ForceEnd};
        case css::ValueId::AllowEnd: return {HangingPunctuationValue::AllowEnd};
        case css::ValueId::Last:     return {HangingPunctuationValue::Last};
        default:                     state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::None {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::None {};
    }

    HangingPunctuationValueEnumSet result;
    for (const auto &item : *list)
    {
      switch (item.ValueId())
      {
        case css::ValueId::First: result.Value.Add(HangingPunctuationValue::First); break;
        case css::ValueId::ForceEnd:
          if (result.Value.Contains(HangingPunctuationValue::AllowEnd))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::None {};
          }
          result.Value.Add(HangingPunctuationValue::ForceEnd);
          break;
        case css::ValueId::AllowEnd:
          if (result.Value.Contains(HangingPunctuationValue::ForceEnd))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::None {};
          }
          result.Value.Add(HangingPunctuationValue::AllowEnd);
          break;
        case css::ValueId::Last: result.Value.Add(HangingPunctuationValue::Last); break;
        default:                 state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::None {};
      }
    }
    return result;
  }
}
