#include "Krystal.Web/Style/Values/Text/HyphenateLimitChars.hpp"

#include "Krystal.Web/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Web/Style/Builder/BuilderState.hpp"
#include "Krystal.Web/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Web/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::web::style
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
    auto word = ConvertCSSValueToStyle<HyphenateLimitEdge>(state, (*list)[0]);
    auto before = list->size() > 1 ? ConvertCSSValueToStyle<HyphenateLimitEdge>(state, (*list)[1])
                                   : HyphenateLimitEdge {css::keywords::Auto {}};
    auto after = list->size() > 2 ? ConvertCSSValueToStyle<HyphenateLimitEdge>(state, (*list)[2]) : before;
    return {word, before, after};
  }
}
