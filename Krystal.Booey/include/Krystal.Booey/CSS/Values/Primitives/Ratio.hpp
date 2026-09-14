#pragma once

#include "Krystal.Booey/CSS/Values/Aggregates.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo::css
{
  // <ratio> = <number [0,∞]> [ / <number [0,∞]> ]?
  // https://drafts.csswg.org/css-values-4/#ratio-value
  struct Ratio
  {
    Number<NonNegative> numerator;
    Number<NonNegative> denominator {1_css_number};

    KRYS_NODISCARD bool operator==(const Ratio &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Ratio &value) noexcept
  {
    if constexpr (!I)
    {
      return value.numerator;
    }
    else if constexpr (I == 1)
    {
      return value.denominator;
    }
  }

  /// @brief `Ratio` is special-cased to return a `RatioValue`.
  template <>
  struct CSSValueCreation<Ratio>
  {
    KRYS_NODISCARD Ref<Value> operator()(ValuePool &, const Ratio &) noexcept;
  };
}

DEFINE_SLASH_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Ratio, 2)
