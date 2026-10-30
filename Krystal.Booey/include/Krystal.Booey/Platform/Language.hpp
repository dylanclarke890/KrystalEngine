#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief The user's preferred languages, most preferred first, as BCP 47 tags: the host's, or the
  /// system's default language.
  KRYS_NODISCARD List<utf8_string> UserPreferredLanguages() noexcept;

  /// @brief Sets the user's preferred languages (empty for the system's), and tells the observers.
  void OverrideUserPreferredLanguages(List<utf8_string> &&languages) noexcept;

  /// @brief The languages the host set; empty if it set none.
  KRYS_NODISCARD const List<utf8_string> &UserPreferredLanguagesOverride() noexcept;

  /// @brief Calls @p observer, keyed by @p context, when the preferred languages change.
  void AddLanguageChangeObserver(const void *context, Func<void()> &&observer) noexcept;
  void RemoveLanguageChangeObserver(const void *context) noexcept;
}
