#include "Krystal.Booey/Graphics/SystemFontDatabase.hpp"
#include "Krystal.Booey/CSS/WebKitFontFamilyNames.hpp"

namespace krys::boo
{
  SystemFontDatabase &SystemFontDatabase::Singleton() noexcept
  {
    struct Database : SystemFontDatabase
    {
    };
    static Database database;
    return database;
  }

  auto SystemFontDatabase::GetSystemFontShorthandInfo(FontShorthand fontShorthand) noexcept
    -> const SystemFontShorthandInfo &
  {
    auto &entry = _systemFontShorthandCache[static_cast<size_t>(fontShorthand)];
    if (!entry)
    {
      entry = PlatformSystemFontShorthandInfo(fontShorthand);
    }
    return *entry;
  }

  const StringAtom &SystemFontDatabase::SystemFontShorthandFamily(FontShorthand fontShorthand) noexcept
  {
    return GetSystemFontShorthandInfo(fontShorthand).Family;
  }

  float SystemFontDatabase::SystemFontShorthandSize(FontShorthand fontShorthand) noexcept
  {
    return GetSystemFontShorthandInfo(fontShorthand).Size;
  }

  FontSelectionValue SystemFontDatabase::SystemFontShorthandWeight(FontShorthand fontShorthand) noexcept
  {
    return GetSystemFontShorthandInfo(fontShorthand).Weight;
  }

  void SystemFontDatabase::Invalidate() noexcept
  {
    for (auto &item : _systemFontShorthandCache)
    {
      item.reset();
    }
    PlatformInvalidate();
  }

  auto SystemFontDatabase::PlatformSystemFontShorthandInfo(FontShorthand) noexcept -> SystemFontShorthandInfo
  {
    // FIXME: Ask the platform for its system fonts, once there's a font backend. Until then every system font
    // is the standard family at 16px and normal weight.
    return {css::WebKitFontFamilyNames::StandardFamily(), 16, NormalWeightValue()};
  }

  void SystemFontDatabase::PlatformInvalidate() noexcept
  {
  }
}
