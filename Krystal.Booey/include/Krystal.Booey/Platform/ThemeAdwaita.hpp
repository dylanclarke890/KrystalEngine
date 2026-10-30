#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Platform/Theme.hpp"

namespace krys::boo
{
  /// @brief GNOME's Adwaita theme: its accent colour, which controls, selections and focus rings use.
  class ThemeAdwaita : public Theme
  {
  private:
    Color _accentColor {SRGBA<uint8> {52, 132, 228}};
    bool _prefersReducedMotion {false};
    bool _prefersContrast {false};

  public:
    ThemeAdwaita() noexcept = default;

    virtual void PlatformColorsDidChange() noexcept
    {
    }

    KRYS_NODISCARD bool UserPrefersContrast() const noexcept final;
    KRYS_NODISCARD bool UserPrefersReducedMotion() const noexcept final;

    void SetAccentColor(const Color &) noexcept;
    KRYS_NODISCARD Color AccentColor() const noexcept;
  };
}
