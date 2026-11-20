#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Strings/String.hpp"

namespace krys::text
{
  /// @brief @p character's simple uppercase mapping (one code point to one), or itself if it has none.
  KRYS_NODISCARD char32 SimpleUppercase(char32 character) noexcept;

  /// @brief @p character's simple titlecase mapping (one code point to one), or itself if it has none.
  KRYS_NODISCARD char32 SimpleTitlecase(char32 character) noexcept;

  /// @brief @p character's simple default case folding, for caseless matching.
  KRYS_NODISCARD char32 SimpleCaseFold(char32 character) noexcept;

  /// @brief @p character's full titlecase mapping, which may be several code points (`ß` → `Ss`).
  KRYS_NODISCARD utf32_string ToTitlecase(char32 character) noexcept;

  /// @brief @p text's full uppercase mapping, with @p locale's rules (a BCP 47 tag, or empty for the root
  /// locale's); @p text unchanged if it can't be mapped.
  KRYS_NODISCARD utf8_string ToUppercase(utf8_stringview text, utf8_stringview locale) noexcept;

  /// @brief @p text's full lowercase mapping, with @p locale's rules (a BCP 47 tag, or empty for the root
  /// locale's); @p text unchanged if it can't be mapped.
  KRYS_NODISCARD utf8_string ToLowercase(utf8_stringview text, utf8_stringview locale) noexcept;
}
