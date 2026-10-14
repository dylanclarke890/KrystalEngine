#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <counter-style> = <custom-ident excluding=none>
  // https://drafts.csswg.org/css-counter-styles-3/#typedef-counter-style
  struct CounterStyle
  {
    css::CustomIdentifier Identifier;

    KRYS_NODISCARD bool operator==(const CounterStyle &) const noexcept = default;

    KRYS_NODISCARD bool operator==(const css::CustomIdentifier &other) const noexcept
    {
      return Identifier == other;
    }

    KRYS_NODISCARD bool operator==(const css::CSSOMStringAtom &other) const noexcept
    {
      return Identifier.Value == other;
    }

    KRYS_NODISCARD bool operator==(css::ValueId other) const noexcept
    {
      return Identifier.Value == css::CSSOMStringAtom {css::ToString(other)};
    }
  };
  DEFINE_TYPE_WRAPPER_GET(CounterStyle, Identifier)

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<CounterStyle>
  {
    KRYS_NODISCARD CounterStyle operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::CounterStyle)
