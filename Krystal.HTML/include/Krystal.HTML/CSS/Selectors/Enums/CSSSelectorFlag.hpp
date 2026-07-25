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
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::CSSSelectorFlag, 5uz);