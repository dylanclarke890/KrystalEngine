#pragma once

#include "Krystal.Booey/Graphics/Adwaita/ControlAdwaita.hpp"
#include "Krystal.Booey/Graphics/Controls/SliderTrackPart.hpp"

namespace krys::boo
{
  /// @brief A slider's track: a rounded trough filled with the accent colour up to the thumb.
  class SliderTrackAdwaita final : public ControlAdwaita
  {
  public:
    SliderTrackAdwaita(ControlPart &, ControlFactoryAdwaita &) noexcept;

    void Draw(GraphicsContext &, const FloatRoundedRect &borderRect, float deviceScaleFactor,
              const ControlStyle &) noexcept override;

  private:
    KRYS_NODISCARD const SliderTrackPart &OwningSliderTrackPart() const noexcept
    {
      return Downcast<SliderTrackPart>(_owningPart);
    }
  };
}
