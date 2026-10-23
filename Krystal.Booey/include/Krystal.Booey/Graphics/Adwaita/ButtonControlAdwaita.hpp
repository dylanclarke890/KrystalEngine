#pragma once

#include "Krystal.Booey/Graphics/Adwaita/ControlAdwaita.hpp"

namespace krys::boo
{
  /// @brief A push button: a rounded, bordered face that lightens when hovered and darkens when pressed.
  class ButtonControlAdwaita final : public ControlAdwaita
  {
  public:
    ButtonControlAdwaita(ControlPart &, ControlFactoryAdwaita &) noexcept;

    void Draw(GraphicsContext &, const FloatRoundedRect &borderRect, float deviceScaleFactor,
              const ControlStyle &) noexcept override;
    static void DrawButton(GraphicsContext &, const FloatRoundedRect &borderRect, float deviceScaleFactor,
                           const ControlStyle &) noexcept;
  };
}
