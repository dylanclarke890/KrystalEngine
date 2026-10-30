#pragma once

#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"

namespace krys::boo
{
  /// @brief A drop-down select.
  class MenuListPart final : public ControlPart
  {
  public:
    KRYS_NODISCARD static Ref<MenuListPart> Create() noexcept
    {
      return AdoptRef(*new MenuListPart());
    }

  private:
    MenuListPart() noexcept : ControlPart(StyleAppearance::Menulist)
    {
    }

    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformControl() noexcept final
    {
      return GetControlFactory().CreatePlatformMenuList(*this);
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::MenuListPart)
  KRYS_NODISCARD static bool IsType(const krys::boo::ControlPart &part) noexcept
  {
    return part.GetType() == krys::boo::StyleAppearance::Menulist;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
