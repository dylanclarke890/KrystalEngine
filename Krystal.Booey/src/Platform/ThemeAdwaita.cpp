#include "Krystal.Booey/Platform/ThemeAdwaita.hpp"

namespace krys::boo
{
  Theme &Theme::Singleton() noexcept
  {
    static ThemeAdwaita theme;
    return theme;
  }

  // FIXME: The system's reduced motion and high contrast settings, once the host reports them.

  void ThemeAdwaita::SetAccentColor(const Color &color) noexcept
  {
    if (_accentColor == color)
    {
      return;
    }

    _accentColor = color;
    PlatformColorsDidChange();
  }

  Color ThemeAdwaita::AccentColor() const noexcept
  {
    return _accentColor;
  }

  bool ThemeAdwaita::UserPrefersContrast() const noexcept
  {
    return _prefersContrast;
  }

  bool ThemeAdwaita::UserPrefersReducedMotion() const noexcept
  {
    return _prefersReducedMotion;
  }
}
