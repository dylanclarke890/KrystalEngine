#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::style
{
  // <single-animation-duration> = auto | <time [0s,∞]>
  // https://www.w3.org/TR/css-animations-2/#propdef-animation-duration
  struct SingleAnimationDuration : css::ValueOrKeyword<Time<css::NonNegative>, css::keywords::Auto>
  {
    using Base::Base;
    using Time = Base::Value;

    SingleAnimationDuration(css::ValueLiteral<css::TimeUnit::S> literal) noexcept
        : SingleAnimationDuration {Time {literal}}
    {
    }

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return IsKeyword();
    }
    KRYS_NODISCARD bool IsTime() const noexcept
    {
      return IsValue();
    }
    KRYS_NODISCARD Maybe<Time> TryTime() const noexcept
    {
      return TryValue();
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SingleAnimationDuration)
