#include "Krystal.Booey/Graphics/Adwaita/ControlFactoryAdwaita.hpp"
#include "Krystal.Booey/Graphics/Adwaita/ButtonControlAdwaita.hpp"
#include "Krystal.Booey/Graphics/Adwaita/InnerSpinButtonAdwaita.hpp"
#include "Krystal.Booey/Graphics/Adwaita/MenuListAdwaita.hpp"
#include "Krystal.Booey/Graphics/Adwaita/ProgressBarAdwaita.hpp"
#include "Krystal.Booey/Graphics/Adwaita/SliderThumbAdwaita.hpp"
#include "Krystal.Booey/Graphics/Adwaita/SliderTrackAdwaita.hpp"
#include "Krystal.Booey/Graphics/Adwaita/TextFieldAdwaita.hpp"
#include "Krystal.Booey/Graphics/Adwaita/ToggleButtonAdwaita.hpp"
#include "Krystal.Booey/Graphics/Controls/ButtonPart.hpp"
#include "Krystal.Booey/Graphics/Controls/ColorWellPart.hpp"
#include "Krystal.Booey/Graphics/Controls/InnerSpinButtonPart.hpp"
#include "Krystal.Booey/Graphics/Controls/MenuListButtonPart.hpp"
#include "Krystal.Booey/Graphics/Controls/MenuListPart.hpp"
#include "Krystal.Booey/Graphics/Controls/SearchFieldPart.hpp"
#include "Krystal.Booey/Graphics/Controls/SliderThumbPart.hpp"
#include "Krystal.Booey/Graphics/Controls/TextAreaPart.hpp"
#include "Krystal.Booey/Graphics/Controls/TextFieldPart.hpp"
#include "Krystal.Booey/Graphics/Controls/ToggleButtonPart.hpp"

namespace krys::boo
{
  Ref<ControlFactory> ControlFactory::Create() noexcept
  {
    return AdoptRef(*new ControlFactoryAdwaita());
  }

  ControlFactoryAdwaita &ControlFactoryAdwaita::Singleton() noexcept
  {
    return Downcast<ControlFactoryAdwaita>(ControlFactory::Singleton());
  }

  UniquePtr<PlatformControl> ControlFactoryAdwaita::CreatePlatformButton(ButtonPart &part) noexcept
  {
    return CreateUnique<ButtonControlAdwaita>(part, *this);
  }

  UniquePtr<PlatformControl> ControlFactoryAdwaita::CreatePlatformColorWell(ColorWellPart &part) noexcept
  {
    return CreateUnique<ButtonControlAdwaita>(part, *this);
  }

  UniquePtr<PlatformControl>
    ControlFactoryAdwaita::CreatePlatformInnerSpinButton(InnerSpinButtonPart &part) noexcept
  {
    return CreateUnique<InnerSpinButtonAdwaita>(part, *this);
  }

  UniquePtr<PlatformControl> ControlFactoryAdwaita::CreatePlatformMenuList(MenuListPart &part) noexcept
  {
    return CreateUnique<MenuListAdwaita>(part, *this);
  }

  UniquePtr<PlatformControl>
    ControlFactoryAdwaita::CreatePlatformMenuListButton(MenuListButtonPart &part) noexcept
  {
    return CreateUnique<MenuListAdwaita>(part, *this);
  }

  UniquePtr<PlatformControl> ControlFactoryAdwaita::CreatePlatformMeter(MeterPart &) noexcept
  {
    return nullptr;
  }

  UniquePtr<PlatformControl> ControlFactoryAdwaita::CreatePlatformProgressBar(ProgressBarPart &part) noexcept
  {
    return CreateUnique<ProgressBarAdwaita>(part, *this);
  }

  UniquePtr<PlatformControl> ControlFactoryAdwaita::CreatePlatformSearchField(SearchFieldPart &part) noexcept
  {
    return CreateUnique<TextFieldAdwaita>(part, *this);
  }

  UniquePtr<PlatformControl>
    ControlFactoryAdwaita::CreatePlatformSearchFieldCancelButton(SearchFieldCancelButtonPart &) noexcept
  {
    return nullptr;
  }

  UniquePtr<PlatformControl>
    ControlFactoryAdwaita::CreatePlatformSearchFieldResults(SearchFieldResultsPart &) noexcept
  {
    return nullptr;
  }

  UniquePtr<PlatformControl> ControlFactoryAdwaita::CreatePlatformSliderThumb(SliderThumbPart &part) noexcept
  {
    return CreateUnique<SliderThumbAdwaita>(part, *this);
  }

  UniquePtr<PlatformControl> ControlFactoryAdwaita::CreatePlatformSliderTrack(SliderTrackPart &part) noexcept
  {
    return CreateUnique<SliderTrackAdwaita>(part, *this);
  }

  UniquePtr<PlatformControl> ControlFactoryAdwaita::CreatePlatformTextArea(TextAreaPart &part) noexcept
  {
    return CreateUnique<TextFieldAdwaita>(part, *this);
  }

  UniquePtr<PlatformControl> ControlFactoryAdwaita::CreatePlatformTextField(TextFieldPart &part) noexcept
  {
    return CreateUnique<TextFieldAdwaita>(part, *this);
  }

  UniquePtr<PlatformControl>
    ControlFactoryAdwaita::CreatePlatformToggleButton(ToggleButtonPart &part) noexcept
  {
    return CreateUnique<ToggleButtonAdwaita>(part, *this);
  }
}
