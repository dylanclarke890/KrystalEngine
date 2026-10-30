#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Core/Maths/Base.hpp"

namespace krys::boo::css
{
  // FIXME: Contexts that allow calc() should not be defined using a closed interval -
  // https://drafts.csswg.org/css-values-4/#calc-range
  // If spring() ever goes further with standardization, the allowable ranges for `mass` and `stiffness`
  // should be reconsidered as the std::nextafter() clamping is non-obvious.

  // <spring()> = spring( <number [>0,∞]> <number [>0,∞]> <number [0,∞]> <number> )
  // Non-standard
  struct SpringEasingParameters
  {
    constexpr static auto NextAfterZero = Limits<double>::denorm_min();
    constexpr static auto Positive = Range {NextAfterZero, Range::Inf};

    Number<Positive> Mass;
    Number<Positive> Stiffness;
    Number<NonNegative> Damping;
    Number<> InitialVelocity;

    KRYS_NODISCARD constexpr bool operator==(const SpringEasingParameters &) const noexcept = default;
  };

  using SpringEasingFunction = FunctionNotation<ValueId::Spring, SpringEasingParameters>;

  template <size_t I>
  KRYS_NODISCARD constexpr const auto &get(const SpringEasingParameters &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Mass;
    }
    else if constexpr (I == 1)
    {
      return value.Stiffness;
    }
    else if constexpr (I == 2)
    {
      return value.Damping;
    }
    else if constexpr (I == 3)
    {
      return value.InitialVelocity;
    }
  }
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::css::SpringEasingParameters, 4)
