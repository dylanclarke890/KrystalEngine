#pragma once

#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumeric.hpp"

namespace krys::boo::style
{
  // <'stroke-miterlimit'> = <number [0,∞]>
  // FIXME: The current spec grammar is `<number [1,∞]>`, though this does not match what SVG specified.
  // https://drafts.fxtf.org/fill-stroke-3/#propdef-stroke-miterlimit
  struct StrokeMiterlimit
  {
    using Number = style::Number<css::NonNegative, float>;

    Number Value;

    constexpr StrokeMiterlimit(Number number) noexcept : Value {number}
    {
    }
    constexpr StrokeMiterlimit(Number::ResolvedValueType literal) noexcept : Value {literal}
    {
    }
    constexpr StrokeMiterlimit(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept : Value {literal}
    {
    }

    KRYS_NODISCARD constexpr bool operator==(const StrokeMiterlimit &) const noexcept = default;
    KRYS_NODISCARD constexpr auto operator<=>(const StrokeMiterlimit &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(StrokeMiterlimit, Value)
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::StrokeMiterlimit)
