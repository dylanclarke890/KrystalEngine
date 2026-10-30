#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/HashMap.hpp"

namespace krys::boo
{
  /// @brief The `counter-reset`, `counter-increment` and `counter-set` values for one counter.
  struct CounterDirectives
  {
    Maybe<int32> ResetValue;
    Maybe<int32> IncrementValue;
    Maybe<int32> SetValue;

    KRYS_NODISCARD constexpr bool operator==(const CounterDirectives &) const noexcept = default;
  };

  /// @brief The counter directives of an element, by counter name.
  struct CounterDirectiveMap
  {
    HashMap<css::CSSOMStringAtom, CounterDirectives> Map;

    KRYS_NODISCARD bool operator==(const CounterDirectiveMap &) const noexcept = default;
  };
}
