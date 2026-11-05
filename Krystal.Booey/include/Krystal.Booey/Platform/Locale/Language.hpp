#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief Sets the user's preferred languages, most preferred first, as BCP 47 tags (empty for the
  /// system's).
  void OverrideUserPreferredLanguages(List<utf8_string> languages) noexcept;

  /// @brief The languages the host set; empty if it set none.
  KRYS_NODISCARD const List<utf8_string> &UserPreferredLanguagesOverride() noexcept;
}
