#pragma once

#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"

namespace krys::boo
{
  /// @brief A push button.
  class ButtonPart final : public ControlPart
  {
  public:
    KRYS_NODISCARD static Ref<ButtonPart> Create(StyleAppearance type) noexcept
    {
      return AdoptRef(*new ButtonPart(type));
    }

  private:
    explicit ButtonPart(StyleAppearance type) noexcept : ControlPart(type)
    {
      krys_debug_assert(type == StyleAppearance::PushButton || type == StyleAppearance::SquareButton
                        || type == StyleAppearance::Button || type == StyleAppearance::DefaultButton);
    }

    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformControl() noexcept final
    {
      return GetControlFactory().CreatePlatformButton(*this);
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ButtonPart)
  KRYS_NODISCARD static bool IsType(const krys::boo::ControlPart &part) noexcept
  {
    return part.GetType() == krys::boo::StyleAppearance::PushButton
           || part.GetType() == krys::boo::StyleAppearance::SquareButton
           || part.GetType() == krys::boo::StyleAppearance::Button
           || part.GetType() == krys::boo::StyleAppearance::DefaultButton;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
