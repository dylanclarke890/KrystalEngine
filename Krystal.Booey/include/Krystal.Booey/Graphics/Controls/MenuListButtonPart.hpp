#pragma once

#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"

namespace krys::boo
{
  /// @brief A drop-down select styled by the page, with only its arrow drawn by the theme.
  class MenuListButtonPart final : public ControlPart
  {
  public:
    KRYS_NODISCARD static Ref<MenuListButtonPart> Create() noexcept
    {
      return AdoptRef(*new MenuListButtonPart());
    }

  private:
    MenuListButtonPart() noexcept : ControlPart(StyleAppearance::MenulistButton)
    {
    }

    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformControl() noexcept final
    {
      return GetControlFactory().CreatePlatformMenuListButton(*this);
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::MenuListButtonPart)
  KRYS_NODISCARD static bool IsType(const krys::boo::ControlPart &part) noexcept
  {
    return part.GetType() == krys::boo::StyleAppearance::MenulistButton;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
