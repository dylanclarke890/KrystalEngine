#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueAggregates.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
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

  /// @brief `Ratio` is special-cased to return a `CSSRatioValue`.
  template <>
  struct CSSValueCreation<Ratio>
  {
    KRYS_NODISCARD Ref<CSSValue> operator()(CSSValuePool &, const Ratio &) noexcept;
  };
}

DEFINE_SLASH_SEPARATED_TUPLE_LIKE_CONFORMANCE(Krys::HTML::Ratio, 2)
