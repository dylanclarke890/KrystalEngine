#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  // <linear()> = linear( [ <number> && <percentage>{0,2} ]# )
  // https://drafts.csswg.org/css-easing-2/#funcdef-linear
  struct LinearEasingParameters
  {
    struct Stop
    {
      struct Length
      {
        Percentage<> Input;
        Maybe<Percentage<>> Extra;

        KRYS_NODISCARD constexpr bool operator==(const Length &) const noexcept = default;
      };

      Number<> Output;
      Maybe<Length> Input;

      KRYS_NODISCARD constexpr bool operator==(const Stop &) const noexcept = default;
    };

    CommaSeparatedList<Stop> Stops;

    KRYS_NODISCARD constexpr bool operator==(const LinearEasingParameters &) const noexcept = default;
  };

  using LinearEasingFunction = FunctionNotation<ValueId::Linear, LinearEasingParameters>;

  DEFINE_TYPE_WRAPPER_GET(LinearEasingParameters, Stops)

  template <size_t I>
  KRYS_NODISCARD constexpr const auto &get(const LinearEasingParameters::Stop &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Output;
    }
    else if constexpr (I == 1)
    {
      return value.Input;
    }
  }

  template <size_t I>
  KRYS_NODISCARD constexpr const auto &get(const LinearEasingParameters::Stop::Length &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Input;
    }
    else if constexpr (I == 1)
    {
      return value.Extra;
    }
  }
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::LinearEasingParameters, 1)
DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::css::LinearEasingParameters::Stop, 2)
DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::css::LinearEasingParameters::Stop::Length, 2)
