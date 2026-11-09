/*
 * Copyright (C) 2024 Sony Interactive Entertainment Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
