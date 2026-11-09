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

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Geometry/BoxExtents.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief A state a theme draws a form control in.
  enum class ControlStyleState : uint8
  {
    Hovered,
    Pressed,
    Focused,
    Enabled,
    Checked,
    Default,
    WindowActive,
    Indeterminate,
    SpinUp, // Sub-state for HoverState and PressedState.
    Presenting,
    FormSemanticContext,
    DarkAppearance,
    InlineFlippedWritingMode,
    LargeControls,
    ReadOnly,
    ListButton,
    ListButtonPressed,
    VerticalWritingMode,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::ControlStyleState, 18uz);

namespace krys::boo
{
  /// @brief The states and style a theme draws a form control with.
  struct ControlStyle
  {
    using State = ControlStyleState;

    EnumSet<State> States;
    float FontSize {12};
    float ZoomFactor {1};
    Color AccentColor;
    Color TextColor;
    FloatBoxExtent BorderWidth;
  };

  TextStream &operator<<(TextStream &ts, ControlStyleState state) noexcept;
  TextStream &operator<<(TextStream &ts, const ControlStyle &style) noexcept;
}
