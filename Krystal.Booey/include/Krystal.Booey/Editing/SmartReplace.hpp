#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief Whether smart replace adds no space next to @p c: whitespace, CJK, and the punctuation that goes
  /// before (or, if @p isPreviousCharacter is false, after) a word without a space.
  KRYS_NODISCARD bool IsCharacterSmartReplaceExempt(char32 c, bool isPreviousCharacter) noexcept;
}
