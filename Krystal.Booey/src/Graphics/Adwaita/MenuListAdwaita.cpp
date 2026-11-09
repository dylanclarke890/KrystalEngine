/*
 * Copyright (C) 2015, 2020 Igalia S.L.
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Adwaita/MenuListAdwaita.hpp"
#include "Krystal.Booey/Graphics/Adwaita/ButtonControlAdwaita.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"

namespace krys::boo
{
  using namespace adwaita;

  MenuListAdwaita::MenuListAdwaita(ControlPart &part, ControlFactoryAdwaita &controlFactory) noexcept
      : ControlAdwaita(part, controlFactory)
  {
  }

  void MenuListAdwaita::Draw(GraphicsContext &graphicsContext, const FloatRoundedRect &borderRect,
                             float deviceScaleFactor, const ControlStyle &style) noexcept
  {
    GraphicsContextStateSaver stateSaver(graphicsContext);

    ButtonControlAdwaita::DrawButton(graphicsContext, borderRect, deviceScaleFactor, style);

    auto zoomedArrowSize = MenuListButtonArrowSize * style.ZoomFactor;
    FloatRect fieldRect = borderRect.Rect();
    auto borderWidth = fieldRect.Width();
    auto borderHeight = fieldRect.Height();
    fieldRect.SetSize({zoomedArrowSize, zoomedArrowSize});
    if (!style.States.Contains(ControlStyle::State::VerticalWritingMode))
    {
      auto centerY = (borderHeight / 2) - (zoomedArrowSize / 2);
      if (style.States.Contains(ControlStyle::State::InlineFlippedWritingMode))
      {
        fieldRect.Move(MenuListButtonPadding, centerY);
      }
      else
      {
        fieldRect.Move(borderWidth - (zoomedArrowSize + MenuListButtonPadding), centerY);
      }
    }
    else
    {
      auto centerX = (borderWidth / 2) - (zoomedArrowSize / 2);
      fieldRect.Move(centerX, borderHeight - (zoomedArrowSize + MenuListButtonPadding));
    }
    PaintArrow(graphicsContext, fieldRect, ArrowDirection::Down,
               style.States.Contains(ControlStyle::State::DarkAppearance));

    if (style.States.Contains(ControlStyle::State::Focused))
    {
      PaintFocus(graphicsContext, borderRect.Rect(), MenuListButtonFocusOffset, SystemFocusRingColor());
    }
  }
}
