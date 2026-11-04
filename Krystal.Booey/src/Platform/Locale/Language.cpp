#include "Krystal.Booey/Platform/Locale/Language.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD List<utf8_string> &PreferredLanguagesOverride() noexcept
    {
      static List<utf8_string> languages;
      return languages;
    }
  }

  void OverrideUserPreferredLanguages(List<utf8_string> languages) noexcept
  {
    PreferredLanguagesOverride() = krys::move(languages);
  }

  const List<utf8_string> &UserPreferredLanguagesOverride() noexcept
  {
    return PreferredLanguagesOverride();
  }
}
