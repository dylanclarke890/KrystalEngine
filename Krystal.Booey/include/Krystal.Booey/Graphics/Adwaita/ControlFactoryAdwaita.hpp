#pragma once

#include "Krystal.Booey/Graphics/Controls/ControlFactory.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief Makes the Adwaita drawing of each control part.
  class ControlFactoryAdwaita final : public ControlFactory
  {
  public:
    KRYS_NODISCARD static ControlFactoryAdwaita &Singleton() noexcept;

  private:
    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformButton(ButtonPart &) noexcept final;
    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformColorWell(ColorWellPart &) noexcept final;
    KRYS_NODISCARD UniquePtr<PlatformControl>
      CreatePlatformInnerSpinButton(InnerSpinButtonPart &) noexcept final;
    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformMenuList(MenuListPart &) noexcept final;
    KRYS_NODISCARD UniquePtr<PlatformControl>
      CreatePlatformMenuListButton(MenuListButtonPart &) noexcept final;
    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformMeter(MeterPart &) noexcept final;
    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformProgressBar(ProgressBarPart &) noexcept final;
    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformSearchField(SearchFieldPart &) noexcept final;
    KRYS_NODISCARD UniquePtr<PlatformControl>
      CreatePlatformSearchFieldCancelButton(SearchFieldCancelButtonPart &) noexcept final;
    KRYS_NODISCARD UniquePtr<PlatformControl>
      CreatePlatformSearchFieldResults(SearchFieldResultsPart &) noexcept final;
    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformSliderThumb(SliderThumbPart &) noexcept final;
    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformSliderTrack(SliderTrackPart &) noexcept final;
    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformTextArea(TextAreaPart &) noexcept final;
    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformTextField(TextFieldPart &) noexcept final;
    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformToggleButton(ToggleButtonPart &) noexcept final;

    KRYS_NODISCARD Type GetType() const noexcept final
    {
      return Type::Adwaita;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ControlFactoryAdwaita)
  KRYS_NODISCARD static bool IsType(const krys::boo::ControlFactory &factory) noexcept
  {
    return factory.GetType() == krys::boo::ControlFactory::Type::Adwaita;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
