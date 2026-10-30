#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class ButtonPart;
  class ColorWellPart;
  class InnerSpinButtonPart;
  class MeterPart;
  class MenuListButtonPart;
  class MenuListPart;
  class PlatformControl;
  class ProgressBarPart;
  class SearchFieldCancelButtonPart;
  class SearchFieldPart;
  class SearchFieldResultsPart;
  class SliderThumbPart;
  class SliderTrackPart;
  class TextAreaPart;
  class TextFieldPart;
  class ToggleButtonPart;

  /// @brief Makes each control part's platform control: the theme's drawing of it.
  class ControlFactory : public RefCounted<ControlFactory>
  {
  public:
    enum class Type : uint8
    {
      Adwaita,
    };

    virtual ~ControlFactory() noexcept = default;

    KRYS_NODISCARD static Ref<ControlFactory> Create() noexcept;
    KRYS_NODISCARD static ControlFactory &Singleton() noexcept;

    KRYS_NODISCARD virtual UniquePtr<PlatformControl> CreatePlatformButton(ButtonPart &) noexcept = 0;
    KRYS_NODISCARD virtual UniquePtr<PlatformControl> CreatePlatformColorWell(ColorWellPart &) noexcept = 0;
    KRYS_NODISCARD virtual UniquePtr<PlatformControl>
      CreatePlatformInnerSpinButton(InnerSpinButtonPart &) noexcept = 0;
    KRYS_NODISCARD virtual UniquePtr<PlatformControl> CreatePlatformMenuList(MenuListPart &) noexcept = 0;
    KRYS_NODISCARD virtual UniquePtr<PlatformControl>
      CreatePlatformMenuListButton(MenuListButtonPart &) noexcept = 0;
    KRYS_NODISCARD virtual UniquePtr<PlatformControl> CreatePlatformMeter(MeterPart &) noexcept = 0;
    KRYS_NODISCARD virtual UniquePtr<PlatformControl>
      CreatePlatformProgressBar(ProgressBarPart &) noexcept = 0;
    KRYS_NODISCARD virtual UniquePtr<PlatformControl>
      CreatePlatformSearchField(SearchFieldPart &) noexcept = 0;
    KRYS_NODISCARD virtual UniquePtr<PlatformControl>
      CreatePlatformSearchFieldCancelButton(SearchFieldCancelButtonPart &) noexcept = 0;
    KRYS_NODISCARD virtual UniquePtr<PlatformControl>
      CreatePlatformSearchFieldResults(SearchFieldResultsPart &) noexcept = 0;
    KRYS_NODISCARD virtual UniquePtr<PlatformControl>
      CreatePlatformSliderThumb(SliderThumbPart &) noexcept = 0;
    KRYS_NODISCARD virtual UniquePtr<PlatformControl>
      CreatePlatformSliderTrack(SliderTrackPart &) noexcept = 0;
    KRYS_NODISCARD virtual UniquePtr<PlatformControl> CreatePlatformTextArea(TextAreaPart &) noexcept = 0;
    KRYS_NODISCARD virtual UniquePtr<PlatformControl> CreatePlatformTextField(TextFieldPart &) noexcept = 0;
    KRYS_NODISCARD virtual UniquePtr<PlatformControl>
      CreatePlatformToggleButton(ToggleButtonPart &) noexcept = 0;

    KRYS_NODISCARD virtual Type GetType() const noexcept = 0;

  protected:
    ControlFactory() noexcept = default;
  };
}
