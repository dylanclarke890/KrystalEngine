#pragma once

#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"

namespace krys::boo
{
  /// @brief A checkbox or radio button.
  class ToggleButtonPart final : public ControlPart
  {
  public:
    KRYS_NODISCARD static Ref<ToggleButtonPart> Create(StyleAppearance type) noexcept
    {
      return AdoptRef(*new ToggleButtonPart(type));
    }

  private:
    explicit ToggleButtonPart(StyleAppearance type) noexcept : ControlPart(type)
    {
      krys_debug_assert(type == StyleAppearance::Checkbox || type == StyleAppearance::Radio);
    }

    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformControl() noexcept final
    {
      return GetControlFactory().CreatePlatformToggleButton(*this);
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ToggleButtonPart)
  KRYS_NODISCARD static bool IsType(const krys::boo::ControlPart &part) noexcept
  {
    return part.GetType() == krys::boo::StyleAppearance::Checkbox
           || part.GetType() == krys::boo::StyleAppearance::Radio;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
