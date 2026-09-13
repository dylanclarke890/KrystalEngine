#pragma once

#include "Krystal.Booey/CSS/Values/CSSValueTypes.hpp"

namespace krys::boo::css
{
  // <'color-scheme'> = normal | [ light | dark | <custom-ident> ]+ && only?
  // https://drafts.csswg.org/css-color-adjust/#propdef-color-scheme
  struct ColorScheme
  {
    SpaceSeparatedList<CustomIdentifier> schemes;
    Maybe<keywords::Only> only;

    // As an optimization, if `schemes` is empty, that indicates the
    // entire value should be considered `normal`.
    KRYS_NODISCARD bool IsNormal() const noexcept
    {
      return schemes.empty();
    }

    KRYS_NODISCARD bool operator==(const ColorScheme &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const ColorScheme &colorScheme) noexcept
  {
    if constexpr (!I)
    {
      return colorScheme.schemes;
    }
    else if constexpr (I == 1)
    {
      return colorScheme.only;
    }
  }
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::ColorScheme, 2uz)