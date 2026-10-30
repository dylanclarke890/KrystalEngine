#pragma once

#include "Krystal.Booey/Graphics/Adwaita/ControlAdwaita.hpp"

namespace krys::boo
{
  /// @brief A slider's thumb: a bordered circle.
  class SliderThumbAdwaita final : public ControlAdwaita
  {
  public:
    SliderThumbAdwaita(ControlPart &, ControlFactoryAdwaita &) noexcept;

    void Draw(GraphicsContext &, const FloatRoundedRect &borderRect, float deviceScaleFactor,
              const ControlStyle &) noexcept override;
  };
}
