#pragma once

#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"

namespace krys::boo
{
  /// @brief A colour input's well.
  class ColorWellPart final : public ControlPart
  {
  public:
    KRYS_NODISCARD static Ref<ColorWellPart> Create() noexcept
    {
      return AdoptRef(*new ColorWellPart());
    }

  private:
    ColorWellPart() noexcept : ControlPart(StyleAppearance::ColorWell)
    {
    }

    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformControl() noexcept final
    {
      return GetControlFactory().CreatePlatformColorWell(*this);
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_CONTROL_PART(ColorWell)
