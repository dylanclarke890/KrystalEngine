#pragma once

#include "Krystal.Booey/Graphics/Adwaita/ControlAdwaita.hpp"

namespace krys::boo
{
  /// @brief A text field, text area or search field: a rounded border that takes the focus ring colour when
  /// focused.
  class TextFieldAdwaita final : public ControlAdwaita
  {
  public:
    TextFieldAdwaita(ControlPart &, ControlFactoryAdwaita &) noexcept;

    void Draw(GraphicsContext &, const FloatRoundedRect &borderRect, float deviceScaleFactor,
              const ControlStyle &) noexcept override;
  };
}
