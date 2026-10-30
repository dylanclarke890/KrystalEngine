#include "Krystal.Booey/Style/Values/ScrollSnap/ScrollSnapType.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"

namespace krys::boo::style
{
  ScrollSnapType CSSValueConversion<ScrollSnapType>::operator()(BuilderState &state,
                                                                const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::None: return css::keywords::None {};
        default:                 break;
      }

      return FromCSSValue<ScrollSnapAxis>(*primitiveValue);
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::None {};
    }

    auto axis = FromCSSValue<ScrollSnapAxis>(list->item(0));

    if (list->size() == 1)
    {
      return {axis};
    }

    return {axis, FromCSSValue<ScrollSnapStrictness>(list->item(1))};
  }
}
