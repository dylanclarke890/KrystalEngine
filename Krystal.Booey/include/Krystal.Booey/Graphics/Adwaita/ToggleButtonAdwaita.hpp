#pragma once

#include "Krystal.Booey/Graphics/Adwaita/ControlAdwaita.hpp"

namespace krys::boo
{
  /// @brief A checkbox or radio button: an outline, or filled with the accent colour and a check, dash or
  /// dot.
  class ToggleButtonAdwaita final : public ControlAdwaita
  {
  public:
    ToggleButtonAdwaita(ControlPart &, ControlFactoryAdwaita &) noexcept;

    void Draw(GraphicsContext &, const FloatRoundedRect &borderRect, float deviceScaleFactor,
              const ControlStyle &) noexcept override;

  private:
    void DrawCheckbox(GraphicsContext &, const FloatRoundedRect &borderRect, float deviceScaleFactor,
                      const ControlStyle &) noexcept;
    void DrawRadio(GraphicsContext &, const FloatRoundedRect &borderRect, float deviceScaleFactor,
                   const ControlStyle &) noexcept;
  };
}
