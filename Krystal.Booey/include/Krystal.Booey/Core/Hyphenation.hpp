#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"

// FIXME: There are no hyphenation dictionaries: nothing can be hyphenated automatically (`hyphens: auto`).

namespace krys::boo
{
  KRYS_NODISCARD constexpr bool EnoughWidthForHyphenation(float availableWidth, float fontSize) noexcept
  {
    // If the maximum width available for the prefix before the hyphen is small, then it is very unlikely
    // that an hyphenation opportunity exists, so do not bother to look for it.
    return availableWidth > fontSize * 5 / 4;
  }

  /// @brief Whether words in @p localeIdentifier's language can be hyphenated automatically.
  KRYS_NODISCARD bool CanHyphenate(const StringAtom &localeIdentifier) noexcept;

  /// @brief The byte offset of the last hyphenation opportunity in @p string before @p beforeIndex, or 0.
  KRYS_NODISCARD size_t LastHyphenLocation(utf8_stringview string, size_t beforeIndex,
                                           const StringAtom &localeIdentifier) noexcept;
}
