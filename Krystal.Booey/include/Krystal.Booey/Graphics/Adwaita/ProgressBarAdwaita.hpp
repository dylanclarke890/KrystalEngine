#pragma once

#include "Krystal.Booey/Graphics/Adwaita/ControlAdwaita.hpp"
#include "Krystal.Booey/Graphics/Controls/ProgressBarPart.hpp"

namespace krys::boo
{
  /// @brief A progress bar: a rounded trough filled with the accent colour, or a block sweeping back and
  /// forth when indeterminate.
  class ProgressBarAdwaita final : public ControlAdwaita
  {
  public:
    ProgressBarAdwaita(ControlPart &, ControlFactoryAdwaita &) noexcept;

    void Draw(GraphicsContext &, const FloatRoundedRect &borderRect, float deviceScaleFactor,
              const ControlStyle &) noexcept override;

  private:
    KRYS_NODISCARD const ProgressBarPart &OwningProgressBarPart() const noexcept
    {
      return Downcast<ProgressBarPart>(_owningPart);
    }
  };
}
