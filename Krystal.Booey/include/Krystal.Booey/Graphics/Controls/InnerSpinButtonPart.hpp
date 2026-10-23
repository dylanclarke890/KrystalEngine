#pragma once

#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"

namespace krys::boo
{
  /// @brief A number field's spin button.
  class InnerSpinButtonPart final : public ControlPart
  {
  public:
    KRYS_NODISCARD static Ref<InnerSpinButtonPart> Create() noexcept
    {
      return AdoptRef(*new InnerSpinButtonPart());
    }

  private:
    InnerSpinButtonPart() noexcept : ControlPart(StyleAppearance::InnerSpinButton)
    {
    }

    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformControl() noexcept final
    {
      return GetControlFactory().CreatePlatformInnerSpinButton(*this);
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_CONTROL_PART(InnerSpinButton)
