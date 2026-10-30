#include "Krystal.Booey/Graphics/Adwaita/ControlAdwaita.hpp"

namespace krys::boo
{
  using namespace adwaita;

  ControlAdwaita::ControlAdwaita(ControlPart &owningPart, ControlFactoryAdwaita &) noexcept
      : PlatformControl(owningPart)
  {
  }

  Color ControlAdwaita::AccentColor(const ControlStyle &style) noexcept
  {
    if (style.AccentColor.IsValid())
    {
      return style.AccentColor;
    }
    return SystemAccentColor();
  }
}
