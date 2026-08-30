#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  // FIXME: Contexts that allow calc() should not be defined using a closed interval -
  // https://drafts.csswg.org/css-values-4/#calc-range
  // If spring() ever goes further with standardization, the allowable ranges for `mass` and `stiffness`
  // should be reconsidered as the std::nextafter() clamping is non-obvious.

  // <spring()> = spring( <number [>0,∞]> <number [>0,∞]> <number [0,∞]> <number> )
  // Non-standard
  struct SpringEasingParameters
  {
    constexpr static auto NextAfterZero = std::numeric_limits<double>::denorm_min();
    constexpr static auto Positive = CSSRange {NextAfterZero, CSSRange::Inf};

    Number<Positive> mass;
    Number<Positive> stiffness;
    Number<NonNegative> damping;
    Number<> initialVelocity;

    constexpr bool operator==(const SpringEasingParameters &) const noexcept = default;
  };

  using SpringEasingFunction = FunctionNotation<CSSValueId::Spring, SpringEasingParameters>;

  template <size_t I>
  KRYS_NODISCARD constexpr const auto &get(const SpringEasingParameters &value) noexcept
  {
    if constexpr (!I)
    {
      return value.mass;
    }
    else if constexpr (I == 1)
    {
      return value.stiffness;
    }
    else if constexpr (I == 2)
    {
      return value.damping;
    }
    else if constexpr (I == 3)
    {
      return value.initialVelocity;
    }
  }
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(Krys::HTML::SpringEasingParameters, 4)
