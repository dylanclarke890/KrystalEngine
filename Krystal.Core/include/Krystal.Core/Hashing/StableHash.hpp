#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Strings/String.hpp"

namespace krys
{
  // FIXME: A `StableHash<T>` trait, beside `FastHash<T>`, over a pinned run-time algorithm (XXH3), once
  // something stores hashes.

  /// @brief The 32-bit FNV-1a hash of @p text, usable at compile time.
  /// @note Pinned: its results may be stored or compiled in (`StringId`), so it must never change.
  /// @see http://www.isthe.com/chongo/tech/comp/fnv/
  KRYS_NODISCARD constexpr uint32 Fnv1a32(stringview text) noexcept
  {
    uint32 hash = 2'166'136'261u;
    for (char character : text)
    {
      hash ^= static_cast<uint8>(character);
      hash *= 16'777'619u;
    }
    return hash;
  }

  /// @brief The same, for UTF-8 text.
  KRYS_NODISCARD constexpr uint32 Fnv1a32(utf8_stringview text) noexcept
  {
    uint32 hash = 2'166'136'261u;
    for (char8 character : text)
    {
      hash ^= static_cast<uint8>(character);
      hash *= 16'777'619u;
    }
    return hash;
  }
}
