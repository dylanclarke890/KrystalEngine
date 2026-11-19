#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Strings/String.hpp"

namespace krys::text
{
  /// @brief The process's default locale for text processing (ICU's, such as `en_US`), which break iteration
  /// and locale formatting fall back to.
  KRYS_NODISCARD utf8_string DefaultLocale() noexcept;

  /// @brief Sets the default locale for text processing.
  /// @note Not thread-safe: set it before text processing starts, or while nothing else uses it.
  void SetDefaultLocale(utf8_stringview locale) noexcept;
}
