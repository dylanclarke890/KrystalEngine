#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::style
{
  // <single-animation-iteration-count> = infinite | <number [0,∞]>
  // https://www.w3.org/TR/css-animations-1/#typedef-single-animation-iteration-count
  struct SingleAnimationIterationCount
      : css::ValueOrKeyword<Number<css::NonNegative>, css::keywords::Infinite>
  {
    using Base::Base;
    using Number = Base::Value;

    SingleAnimationIterationCount(double count) noexcept : Base {Number {count}}
    {
    }

    KRYS_NODISCARD bool IsInfinite() const noexcept
    {
      return IsKeyword();
    }
    KRYS_NODISCARD bool IsNumber() const noexcept
    {
      return IsValue();
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SingleAnimationIterationCount)
