#include "Krystal.Booey/Platform/Language.hpp"
#include "Krystal.Booey/Core/PlatformLocale.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD List<utf8_string> &PreferredLanguagesOverride() noexcept
    {
      static List<utf8_string> languages;
      return languages;
    }

    KRYS_NODISCARD List<Pair<const void *, Func<void()>>> &Observers() noexcept
    {
      static List<Pair<const void *, Func<void()>>> observers;
      return observers;
    }

    void LanguageDidChange() noexcept
    {
      // An observer may remove itself.
      auto observers = Observers();
      for (auto &[context, observer] : observers)
      {
        observer();
      }
    }
  }

  List<utf8_string> UserPreferredLanguages() noexcept
  {
    auto &override = PreferredLanguagesOverride();
    if (!override.empty())
    {
      return override;
    }
    return {DefaultLanguage()};
  }

  void OverrideUserPreferredLanguages(List<utf8_string> &&languages) noexcept
  {
    PreferredLanguagesOverride() = krys::move(languages);
    LanguageDidChange();
  }

  const List<utf8_string> &UserPreferredLanguagesOverride() noexcept
  {
    return PreferredLanguagesOverride();
  }

  void AddLanguageChangeObserver(const void *context, Func<void()> &&observer) noexcept
  {
    Observers().emplace_back(context, krys::move(observer));
  }

  void RemoveLanguageChangeObserver(const void *context) noexcept
  {
    std::erase_if(Observers(), [context](auto &entry) { return entry.first == context; });
  }
}
