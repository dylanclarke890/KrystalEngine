#include "Krystal.Booey/Style/Values/TextDecoration/TextUnderlinePosition.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"

namespace krys::boo::style
{
  TextUnderlinePosition
    CSSValueConversion<TextUnderlinePosition>::operator()(BuilderState &state,
                                                          const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Auto:     return css::keywords::Auto {};
        case css::ValueId::FromFont: return {TextUnderlinePositionValue::FromFont};
        case css::ValueId::Under:    return {TextUnderlinePositionValue::Under};
        case css::ValueId::Left:     return {TextUnderlinePositionValue::Left};
        case css::ValueId::Right:    return {TextUnderlinePositionValue::Right};
        default:                     state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Auto {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::Auto {};
    }

    TextUnderlinePositionValueEnumSet result;
    for (const auto &item : *list)
    {
      switch (item.ValueId())
      {
        case css::ValueId::FromFont:
          if (result.Contains(TextUnderlinePositionValue::Under))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::Auto {};
          }
          result.Value.Add(TextUnderlinePositionValue::FromFont);
          break;
        case css::ValueId::Under:
          if (result.Contains(TextUnderlinePositionValue::FromFont))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::Auto {};
          }
          result.Value.Add(TextUnderlinePositionValue::Under);
          break;
        case css::ValueId::Left:
          if (result.Contains(TextUnderlinePositionValue::Right))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::Auto {};
          }
          result.Value.Add(TextUnderlinePositionValue::Left);
          break;
        case css::ValueId::Right:
          if (result.Contains(TextUnderlinePositionValue::Left))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::Auto {};
          }
          result.Value.Add(TextUnderlinePositionValue::Right);
          break;
        default: state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Auto {};
      }
    }
    return result;
  }
}
