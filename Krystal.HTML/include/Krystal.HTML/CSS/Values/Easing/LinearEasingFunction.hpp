#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  // <linear()> = linear( [ <number> && <percentage>{0,2} ]# )
  // https://drafts.csswg.org/css-easing-2/#funcdef-linear
  struct LinearEasingParameters
  {
    struct Stop
    {
      struct Length
      {
        Percentage<> input;
        Markable<Percentage<>> extra;

        constexpr bool operator==(const Length &) const noexcept = default;
      };

      Number<> output;
      Maybe<Length> input;

      constexpr bool operator==(const Stop &) const = default;
    };

    CommaSeparatedList<Stop> stops;

    bool operator==(const LinearEasingParameters &) const noexcept = default;
  };

  using LinearEasingFunction = FunctionNotation<CSSValueId::Linear, LinearEasingParameters>;

  DEFINE_TYPE_WRAPPER_GET(LinearEasingParameters, stops);

  template <size_t I>
  KRYS_NODISCARD constexpr const auto &get(const LinearEasingParameters::Stop &value) noexcept
  {
    if constexpr (!I)
    {
      return value.output;
    }
    else if constexpr (I == 1)
    {
      return value.input;
    }
  }

  template <size_t I>
  KRYS_NODISCARD constexpr const auto &get(const LinearEasingParameters::Stop::Length &value) noexcept
  {
    if constexpr (!I)
    {
      return value.input;
    }
    else if constexpr (I == 1)
    {
      return value.extra;
    }
  }
}

DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::LinearEasingParameters, 1)
DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(Krys::HTML::LinearEasingParameters::Stop, 2)
DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(Krys::HTML::LinearEasingParameters::Stop::Length, 2)