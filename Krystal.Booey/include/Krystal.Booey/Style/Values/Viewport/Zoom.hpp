#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::style
{
  // <'zoom'> = <number [0,∞]> | <percentage [0,∞]> | normal
  // NOTE: `normal` is non-standard and gets resolved to `1` at style building time.
  // https://drafts.csswg.org/css-viewport/#propdef-zoom
  struct Zoom
  {
    using Representation = NumberOrPercentageResolvedToNumber<css::NonNegative, css::NonNegative, float>;
    using Number = Representation::Number;
    using Percentage = Representation::Percentage;

    Representation Value;

    constexpr Zoom(Representation value) noexcept : Value {value}
    {
    }
    constexpr Zoom(Number number) noexcept : Value {number}
    {
    }
    constexpr Zoom(Percentage percentage) noexcept : Value {percentage}
    {
    }
    constexpr Zoom(typename Number::ResolvedValueType value) noexcept : Value {value}
    {
    }
    constexpr Zoom(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept : Value {literal}
    {
    }
    constexpr Zoom(css::ValueLiteral<css::PercentageUnit::Percentage> literal) noexcept : Value {literal}
    {
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return Value.IsZero();
    }

    KRYS_NODISCARD constexpr bool operator==(const Zoom &) const noexcept = default;
    KRYS_NODISCARD constexpr bool operator==(typename Number::ResolvedValueType other) const noexcept
    {
      return Value == other;
    }
  };
  DEFINE_TYPE_WRAPPER_GET(Zoom, Value)
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::Zoom)
