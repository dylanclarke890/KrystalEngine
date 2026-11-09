/*
 * Copyright (C) 2022-2023 Apple Inc. All rights reserved.
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
