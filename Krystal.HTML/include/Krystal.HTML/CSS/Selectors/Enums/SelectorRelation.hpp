#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  /// @brief The relationship between two selectors in a CSSSelectorList. From the perspective of the second
  /// (rightmost) selector, so the relationship of `p` in the selector `div > p` is `SelectorRelation::Child`,
  /// and the relationship of `p` in the selector `div p` is `SelectorRelation::Descendant`.
  enum class SelectorRelation : uint8
  {
    // @brief The selector forms a compound selector with the preceding selector. (e.g. `div.class`).
    Compounding,
    /// @brief The selector is a direct child of the preceding selector. (e.g. `div > p`).
    Child,
    /// @brief The selector is a descendant of the preceding selector. (e.g. `div p`).
    Descendant,
    /// @brief The selector is the direct next sibling of the preceding selector. (e.g. `div + p`).
    NextSibling,
    /// @brief The selector is one of the following siblings of the preceding selector. (e.g. `div ~ p`).
    SubsequentSibling,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::SelectorRelation, 5uz);