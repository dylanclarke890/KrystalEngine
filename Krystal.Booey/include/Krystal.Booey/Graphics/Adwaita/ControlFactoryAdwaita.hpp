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
