#include "Krystal.Booey/Style/Values/Box/MarginTrim.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"

namespace krys::boo::style
{
  MarginTrim CSSValueConversion<MarginTrim>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    // See if value is "block" or "inline" before trying to parse a list
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Block: return {style::MarginTrimSide::BlockStart, style::MarginTrimSide::BlockEnd};
        case css::ValueId::Inline:
          return {style::MarginTrimSide::InlineStart, style::MarginTrimSide::InlineEnd};
        case css::ValueId::BlockStart:  return {style::MarginTrimSide::BlockStart};
        case css::ValueId::BlockEnd:    return {style::MarginTrimSide::BlockEnd};
        case css::ValueId::InlineStart: return {style::MarginTrimSide::InlineStart};
        case css::ValueId::InlineEnd:   return {style::MarginTrimSide::InlineEnd};
        default:                        state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::None {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::None {};
    }

    MarginTrimSideEnumSet result;
    for (const auto &item : *list)
    {
      switch (item.ValueId())
      {
        case css::ValueId::Block:
          result.Value.Add({style::MarginTrimSide::BlockStart, style::MarginTrimSide::BlockEnd});
          break;
        case css::ValueId::Inline:
          result.Value.Add({style::MarginTrimSide::InlineStart, style::MarginTrimSide::InlineEnd});
          break;
        case css::ValueId::BlockStart:  result.Value.Add(style::MarginTrimSide::BlockStart); break;
        case css::ValueId::BlockEnd:    result.Value.Add(style::MarginTrimSide::BlockEnd); break;
        case css::ValueId::InlineStart: result.Value.Add(style::MarginTrimSide::InlineStart); break;
        case css::ValueId::InlineEnd:   result.Value.Add(style::MarginTrimSide::InlineEnd); break;
        default:                        state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::None {};
      }
    }
    return result;
  }
}
