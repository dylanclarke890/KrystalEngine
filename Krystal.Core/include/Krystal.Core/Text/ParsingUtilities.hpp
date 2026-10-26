#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::text
{
  /// @brief Consumes @p characters from the front of @p buffer if it starts with them.
  /// @return Whether they were consumed.
  constexpr bool SkipCharactersExactly(utf8_stringview &buffer, utf8_stringview characters) noexcept
  {
    if (!buffer.starts_with(characters))
    {
      return false;
    }
    buffer.remove_prefix(characters.size());
    return true;
  }
}
