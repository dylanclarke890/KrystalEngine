#include "Krystal.Booey/Style/NameScope.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  NameScope CSSValueConversion<NameScope>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(&value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::None: return css::keywords::None {};
        case css::ValueId::All:  return {css::keywords::All {}, state.StyleScopeOrdinal()};
        case css::ValueId::Invalid:
          return {css::CommaSeparatedListHashSet<css::CustomIdentifier> {
                    {css::CSSOMStringAtom {primitiveValue->StringValue()}}},
                  state.StyleScopeOrdinal()};
        default: state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::None {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::None {};
    }

    css::CommaSeparatedListHashSet<css::CustomIdentifier> names;
    for (const auto &item : *list)
    {
      names.Value.add({css::CSSOMStringAtom {item.StringValue()}});
    }

    return {krys::move(names), state.StyleScopeOrdinal()};
  }
}
