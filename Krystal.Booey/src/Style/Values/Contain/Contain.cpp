#include "Krystal.Booey/Style/Values/Contain/Contain.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"

namespace krys::boo::style
{
  Contain CSSValueConversion<Contain>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::None:       return css::keywords::None {};
        case css::ValueId::Strict:     return css::keywords::Strict {};
        case css::ValueId::Content:    return css::keywords::Content {};
        case css::ValueId::Size:       return {ContainValue::Size};
        case css::ValueId::InlineSize: return {ContainValue::InlineSize};
        case css::ValueId::Layout:     return {ContainValue::Layout};
        case css::ValueId::Style:      return {ContainValue::Style};
        case css::ValueId::Paint:      return {ContainValue::Paint};
        default:                       state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::None {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::None {};
    }

    ContainValueEnumSet result;
    for (const auto &item : *list)
    {
      switch (item.ValueId())
      {
        case css::ValueId::Size:
          if (result.Contains(ContainValue::InlineSize))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::None {};
          }
          result.Value.Add(ContainValue::Size);
          break;
        case css::ValueId::InlineSize:
          if (result.Contains(ContainValue::Size))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::None {};
          }
          result.Value.Add(ContainValue::InlineSize);
          break;
        case css::ValueId::Layout: result.Value.Add(ContainValue::Layout); break;
        case css::ValueId::Style:  result.Value.Add(ContainValue::Style); break;
        case css::ValueId::Paint:  result.Value.Add(ContainValue::Paint); break;
        default:                   state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::None {};
      }
    }
    return result;
  }
}
