#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <opacity-value> = <number [0,1]@(allow-both-at-parse)> | <percentage [0,100]@(allow-both-at-parse,
  // converted-to-number)> https://drafts.csswg.org/css-color-4/#typedef-opacity-opacity-value
  struct Opacity
  {
    using Number = style::Number<css::ClosedUnitRange, float>;

    Number Value;

    constexpr Opacity(Number number) noexcept : Value {number}
    {
    }

    constexpr Opacity(float literal) noexcept : Value {literal}
    {
    }

    constexpr Opacity(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept : Value {literal}
    {
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return Value == 0;
    }

    KRYS_NODISCARD constexpr bool IsTransparent() const noexcept
    {
      return Value == 0;
    }

    KRYS_NODISCARD constexpr bool IsOpaque() const noexcept
    {
      return Value == 1;
    }

    KRYS_NODISCARD constexpr bool operator==(const Opacity &) const noexcept = default;
    KRYS_NODISCARD constexpr auto operator<=>(const Opacity &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(Opacity, Value);

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<Opacity>
  {
    KRYS_NODISCARD Opacity operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::Opacity)
