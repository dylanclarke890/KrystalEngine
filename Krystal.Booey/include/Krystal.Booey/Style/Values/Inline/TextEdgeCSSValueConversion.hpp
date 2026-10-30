#pragma once

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Inline/TextEdge.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"

namespace krys::boo::style
{
  template <TextEdgeDerived T>
  struct CSSValueConversion<T>
  {
    KRYS_NODISCARD T operator()(BuilderState &state, const css::Value &value) noexcept
    {
      using Keyword = typename T::Keyword;

      if (Is<css::PrimitiveValue>(value))
      {
        switch (value.ValueId())
        {
          case Keyword::Value:            return Keyword {};
          case css::ValueId::Text:        return {TextEdgeOver::Text, TextEdgeUnder::Text};
          case css::ValueId::Ideographic: return {TextEdgeOver::Ideographic, TextEdgeUnder::Ideographic};
          case css::ValueId::IdeographicInk:
            return {TextEdgeOver::IdeographicInk, TextEdgeUnder::IdeographicInk};
          case css::ValueId::Cap: return {TextEdgeOver::Cap, TextEdgeUnder::Text};
          case css::ValueId::Ex:  return {TextEdgeOver::Ex, TextEdgeUnder::Text};
          default:                break;
        }

        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return Keyword {};
      }

      auto pair = RequiredPairDowncast<css::PrimitiveValue>(state, value);
      if (!pair)
      {
        return Keyword {};
      }

      return {
        ConvertCSSValueToStyle<TextEdgeOver>(state, *pair->first),
        ConvertCSSValueToStyle<TextEdgeUnder>(state, *pair->second),
      };
    }
  };
}
