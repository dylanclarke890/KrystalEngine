#pragma once

#include "Krystal.Booey/Graphics/Adwaita/ControlAdwaita.hpp"

namespace krys::boo
{
  /// @brief A drop-down select: a button with a down arrow.
  class MenuListAdwaita final : public ControlAdwaita
  {
  public:
    MenuListAdwaita(ControlPart &, ControlFactoryAdwaita &) noexcept;

    void Draw(GraphicsContext &, const FloatRoundedRect &borderRect, float deviceScaleFactor,
              const ControlStyle &) noexcept override;
  };
}
