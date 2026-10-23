#pragma once

#include "Krystal.Booey/Graphics/Adwaita/Adwaita.hpp"
#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"
#include "Krystal.Booey/Graphics/Controls/PlatformControl.hpp"

namespace krys::boo
{
  class ControlFactoryAdwaita;

  /// @brief A control part drawn in GNOME's Adwaita style.
  class ControlAdwaita : public PlatformControl
  {
  public:
    ControlAdwaita(ControlPart &, ControlFactoryAdwaita &) noexcept;

  protected:
    KRYS_NODISCARD static Color AccentColor(const ControlStyle &) noexcept;
  };
}
