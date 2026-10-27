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

  /// @brief Consumes @p character from the front of @p buffer if it starts with it.
  /// @return Whether it was consumed.
  constexpr bool SkipExactly(utf8_stringview &buffer, char8_t character) noexcept
  {
    if (buffer.empty() || buffer.front() != character)
    {
      return false;
    }
    buffer.remove_prefix(1);
    return true;
  }

  /// @brief Consumes characters from the front of @p buffer up to, but not including, the first @p delimiter
  /// (or all of them if there's none).
  constexpr void SkipUntil(utf8_stringview &buffer, char8_t delimiter) noexcept
  {
    auto position = buffer.find(delimiter);
    buffer.remove_prefix(position == utf8_stringview::npos ? buffer.size() : position);
  }
}
