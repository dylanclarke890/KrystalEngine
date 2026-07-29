#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class CSSSelectorFlag : uint8
  {
    None = 0,
    /// @brief Set when an attribute selector has a case-insensitive matching flag (e.g. `[attr=value i]`).
    CaseInsensitiveAttributeValueMatching = 1 << 0,
    /// @brief Set when a selector is the last selector in a selector list.
    IsLastInSelectorList = 1 << 1,
    /// @brief Set when a selector is the first component of a complex selector (e.g. `div` in `div > p`).
    IsFirstInComplexSelector = 1 << 2,
    /// @brief Set when a selector is the last component of a complex selector (e.g. `p` in `div > p`).
    IsLastInComplexSelector = 1 << 3,
    /// @brief Set when a selector has rare data.
    HasRareData = 1 << 4,
    /// @brief Set when the type selector is for a namespace rule (e.g. `@namespace svg url(http://www.w3.org/2000/svg);`).
    TagIsForNamespaceRule = 1 << 5,
    /// @brief Set when a selector is implicit (it isn't written by a UA or author but is implied by the
    /// CSSOM, e.g. `:root`).
    IsImplicit = 1 << 6,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::CSSSelectorFlag, 8uz);