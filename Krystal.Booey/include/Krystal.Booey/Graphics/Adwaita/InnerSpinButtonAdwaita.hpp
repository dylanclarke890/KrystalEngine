#pragma once

#include "Krystal.Booey/Graphics/Adwaita/ControlAdwaita.hpp"

namespace krys::boo
{
  /// @brief A number field's spin button: up and down arrows, one above the other.
  class InnerSpinButtonAdwaita final : public ControlAdwaita
  {
  public:
    InnerSpinButtonAdwaita(ControlPart &, ControlFactoryAdwaita &) noexcept;

    void Draw(GraphicsContext &, const FloatRoundedRect &borderRect, float deviceScaleFactor,
              const ControlStyle &) noexcept override;
  };
}
