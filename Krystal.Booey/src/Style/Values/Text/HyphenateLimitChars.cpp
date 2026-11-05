#include "Krystal.Booey/Style/Values/Text/HyphenateLimitChars.hpp"

#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  HyphenateLimitChars CSSValueConversion<HyphenateLimitChars>::operator()(BuilderState &state,
                                                                          const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      auto word = ConvertCSSValueToStyle<HyphenateLimitEdge>(state, *primitiveValue);
      return {word, css::keywords::Auto {}, css::keywords::Auto {}};
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::Auto {};
    }

    // A missing third value is the same as the second.
    auto word = ConvertCSSValueToStyle<HyphenateLimitEdge>(state, list->item(0));
    auto before = list->size() > 1 ? ConvertCSSValueToStyle<HyphenateLimitEdge>(state, list->item(1))
                                   : HyphenateLimitEdge {css::keywords::Auto {}};
    auto after = list->size() > 2 ? ConvertCSSValueToStyle<HyphenateLimitEdge>(state, list->item(2)) : before;
    return {word, before, after};
  }
}
