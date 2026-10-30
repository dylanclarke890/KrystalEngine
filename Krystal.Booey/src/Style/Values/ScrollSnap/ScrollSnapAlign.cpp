#include "Krystal.Booey/Style/Values/ScrollSnap/ScrollSnapAlign.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"

namespace krys::boo::style
{
  ScrollSnapAlign CSSValueConversion<ScrollSnapAlign>::operator()(BuilderState &state,
                                                                  const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      return {ConvertCSSValueToStyle<ScrollSnapAxisAlignType>(state, *primitiveValue)};
    }

    auto pair = RequiredPairDowncast<css::PrimitiveValue>(state, value);
    if (!pair)
    {
      return css::keywords::None {};
    }

    return {
      ConvertCSSValueToStyle<ScrollSnapAxisAlignType>(state, *pair->first),
      ConvertCSSValueToStyle<ScrollSnapAxisAlignType>(state, *pair->second),
    };
  }
}
