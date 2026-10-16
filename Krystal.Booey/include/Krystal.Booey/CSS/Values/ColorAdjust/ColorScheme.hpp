#pragma once

#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  // <'color-scheme'> = normal | [ light | dark | <custom-ident> ]+ && only?
  // https://drafts.csswg.org/css-color-adjust/#propdef-color-scheme
  struct ColorScheme
  {
    SpaceSeparatedList<CustomIdentifier> Schemes;
    Maybe<keywords::Only> Only;

    // As an optimization, if `schemes` is empty, that indicates the
    // entire value should be considered `normal`.
    KRYS_NODISCARD bool IsNormal() const noexcept
    {
      return Schemes.empty();
    }

    KRYS_NODISCARD bool operator==(const ColorScheme &) const noexcept = default;
  };

  template <>
  struct Serialiser<ColorScheme>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const ColorScheme &value) noexcept;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const ColorScheme &colorScheme) noexcept
  {
    if constexpr (!I)
    {
      return colorScheme.Schemes;
    }
    else if constexpr (I == 1)
    {
      return colorScheme.Only;
    }
  }
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::ColorScheme, 2uz)
