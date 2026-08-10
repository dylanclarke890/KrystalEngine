#pragma once

#include "Krystal.HTML/CSS/Properties/Enums/CSSPropertyId.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"
#include "Krystal.Lib/Types/Span.hpp"

namespace Krys::HTML
{
  /// @brief Represents a shorthand property and the longhand properties it expands into.
  /// @example `margin` expands into `margin-top`, `margin-left`, `margin-bottom`, `margin-right`.
  struct CSSPropertyShorthand
  {
    CSSPropertyId ShorthandId : BitCount<CSSPropertyId>() {CSSPropertyId::Invalid};
    Span<const CSSPropertyId> LonghandProperties;
  };

  /// @brief Finds the index of the shorthand property in the given list of shorthands. If the shorthand
  /// property is not found, an assertion will fail.
  KRYS_NODISCARD constexpr size_t
    IndexOfShorthandProperty(CSSPropertyId property, Span<const CSSPropertyShorthand> shorthands) noexcept
  {
    for (size_t i = 0uz; i < shorthands.size(); ++i)
    {
      if (shorthands[i].ShorthandId == property)
      {
        return i;
      }
    }

    assert(false);
    return 0uz;
  }

  using CSSPropertyShorthandList = SmallList<CSSPropertyShorthand, 4uz>;

  /// @brief Gets the list of longhand properties that are set by the given shorthand property.
  KRYS_NODISCARD CSSPropertyShorthand LonghandsForShorthand(CSSPropertyId property) noexcept;

  /// @brief Gets the list of shorthands that set the given longhand property.
  KRYS_NODISCARD CSSPropertyShorthandList MatchingShorthandsForLonghand(CSSPropertyId property) noexcept;
}