#pragma once

#include "Krystal.HTML/CSS/Values/Color/Color.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueAggregates.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  // <text-shadow> = <color>? && [<length>{2} <length [0,∞]>?]
  // https://drafts.csswg.org/css-text-decor-3/#propdef-text-shadow
  struct TextShadow
  {
    Maybe<Color> color;
    SpaceSeparatedPoint<Length<AllUnzoomed>> location;
    Maybe<Length<NonNegativeUnzoomed>> blur;

    bool operator==(const TextShadow &) const = default;
  };

  // <`text-shadow`> = none | <text-shadow>#
  // https://drafts.csswg.org/css-text-decor-3/#propdef-text-shadow
  using TextShadowProperty = ListOrNone<CommaSeparatedList<TextShadow>>;

  template <size_t I>
  const auto &get(const TextShadow &value)
  {
    if constexpr (!I)
    {
      return value.color;
    }
    else if constexpr (I == 1)
    {
      return value.location;
    }
    else if constexpr (I == 2)
    {
      return value.blur;
    }
  }
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(Krys::HTML::TextShadow, 3)