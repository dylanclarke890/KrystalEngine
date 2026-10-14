#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Ratio.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  /// @brief A computed `<ratio>`.
  // <ratio> = <number [0,∞]> [ / <number [0,∞]> ]?
  // https://drafts.csswg.org/css-values-4/#ratio-value
  struct Ratio
  {
    Number<css::NonNegative> Numerator;
    Number<css::NonNegative> Denominator;

    KRYS_NODISCARD constexpr bool operator==(const Ratio &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD constexpr const auto &get(const Ratio &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Numerator;
    }
    else if constexpr (I == 1)
    {
      return value.Denominator;
    }
  }

  DEFINE_TYPE_MAPPING(css::Ratio, Ratio)

  // MARK: Conversion

  // `Ratio` is special-cased to return a `css::RatioValue`.
  template <>
  struct CSSValueCreation<Ratio>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &, const Ratio &) noexcept;
  };
}

DEFINE_SLASH_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::Ratio, 2)
