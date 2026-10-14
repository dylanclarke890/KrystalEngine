#include "Krystal.Booey/Style/Values/Inline/WebKitInitialLetter.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  WebkitInitialLetter CSSValueConversion<WebkitInitialLetter>::operator()(BuilderState &state,
                                                                          const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      if (primitiveValue->ValueId() == css::ValueId::Normal)
      {
        return css::keywords::Normal {};
      }
      return {ConvertCSSValueToStyle<Number<css::NonNegative, float>>(state, *primitiveValue)};
    }

    auto pair = RequiredPairDowncast<css::PrimitiveValue>(state, value);
    if (!pair)
    {
      return css::keywords::Normal {};
    }

    const auto &first = *pair->first;
    const auto &second = *pair->second;

    return {
      ConvertCSSValueToStyle<Number<css::NonNegative, float>>(state, first),
      ConvertCSSValueToStyle<Number<css::NonNegative, float>>(state, second),
    };
  }
}
