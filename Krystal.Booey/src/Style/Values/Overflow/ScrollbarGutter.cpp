#include "Krystal.Booey/Style/Values/Overflow/ScrollbarGutter.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  ScrollbarGutter CSSValueConversion<ScrollbarGutter>::operator()(BuilderState &state,
                                                                  const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      if (primitiveValue->IsValueId())
      {
        switch (primitiveValue->ValueId())
        {
          case css::ValueId::Auto:   return css::keywords::Auto {};
          case css::ValueId::Stable: return css::keywords::Stable {};
          default:                   break;
        }
      }

      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::Auto {};
    }

    auto pair = RequiredPairDowncast<css::PrimitiveValue>(state, value);
    if (!pair)
    {
      return css::keywords::Auto {};
    }

    if (css::IsValueId(pair->first.get(), css::ValueId::Stable)
        && css::IsValueId(pair->second.get(), css::ValueId::BothEdges))
    {
      return {css::keywords::Stable {}, css::keywords::BothEdges {}};
    }

    state.SetCurrentPropertyInvalidAtComputedValueTime();
    return css::keywords::Auto {};
  }
}
