/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Controls/ControlStyle.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, ControlStyleState state) noexcept
  {
    switch (state)
    {
      case ControlStyleState::Hovered:                  ts << "hovered"; break;
      case ControlStyleState::Pressed:                  ts << "pressed"; break;
      case ControlStyleState::Focused:                  ts << "focused"; break;
      case ControlStyleState::Enabled:                  ts << "enabled"; break;
      case ControlStyleState::Checked:                  ts << "checked"; break;
      case ControlStyleState::Default:                  ts << "default"; break;
      case ControlStyleState::WindowActive:             ts << "window-active"; break;
      case ControlStyleState::Indeterminate:            ts << "indeterminate"; break;
      case ControlStyleState::SpinUp:                   ts << "spin-up"; break;
      case ControlStyleState::Presenting:               ts << "presenting"; break;
      case ControlStyleState::FormSemanticContext:      ts << "form-semantic-context"; break;
      case ControlStyleState::DarkAppearance:           ts << "dark-appearance"; break;
      case ControlStyleState::InlineFlippedWritingMode: ts << "inline-flipped-writing-mode"; break;
      case ControlStyleState::LargeControls:            ts << "large-controls"; break;
      case ControlStyleState::ReadOnly:                 ts << "read-only"; break;
      case ControlStyleState::ListButton:               ts << "list-button"; break;
      case ControlStyleState::ListButtonPressed:        ts << "list-button-pressed"; break;
      case ControlStyleState::VerticalWritingMode:      ts << "vertical-writing-mode"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, const ControlStyle &style) noexcept
  {
    ts.DumpProperty(u8"states", style.States);
    ts.DumpProperty(u8"font-size", style.FontSize);
    ts.DumpProperty(u8"zoom-factor", style.ZoomFactor);
    ts.DumpProperty(u8"accent-color", style.AccentColor);
    return ts;
  }
}
