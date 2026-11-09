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

#include "Krystal.Booey/Graphics/Adwaita/ButtonControlAdwaita.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/Paths/Path.hpp"

namespace krys::boo
{
  using namespace adwaita;

  ButtonControlAdwaita::ButtonControlAdwaita(ControlPart &part,
                                             ControlFactoryAdwaita &controlFactory) noexcept
      : ControlAdwaita(part, controlFactory)
  {
  }

  void ButtonControlAdwaita::Draw(GraphicsContext &graphicsContext, const FloatRoundedRect &borderRect,
                                  float deviceScaleFactor, const ControlStyle &style) noexcept
  {
    DrawButton(graphicsContext, borderRect, deviceScaleFactor, style);
  }

  void ButtonControlAdwaita::DrawButton(GraphicsContext &graphicsContext, const FloatRoundedRect &borderRect,
                                        float /*deviceScaleFactor*/, const ControlStyle &style) noexcept
  {
    GraphicsContextStateSaver stateSaver(graphicsContext);

    SRGBA<uint8> buttonBorderColor;
    SRGBA<uint8> buttonBackgroundColor;
    SRGBA<uint8> buttonBackgroundHoveredColor;
    SRGBA<uint8> buttonBackgroundPressedColor;

    if (style.States.Contains(ControlStyle::State::DarkAppearance))
    {
      buttonBorderColor = ButtonBorderColorDark;
      buttonBackgroundColor = ButtonBackgroundColorDark;
      buttonBackgroundHoveredColor = ButtonBackgroundHoveredColorDark;
      buttonBackgroundPressedColor = ButtonBackgroundPressedColorDark;
    }
    else
    {
      buttonBorderColor = ButtonBorderColorLight;
      buttonBackgroundColor = ButtonBackgroundColorLight;
      buttonBackgroundHoveredColor = ButtonBackgroundHoveredColorLight;
      buttonBackgroundPressedColor = ButtonBackgroundPressedColorLight;
    }

    if (!style.States.Contains(ControlStyle::State::Enabled))
    {
      graphicsContext.BeginTransparencyLayer(static_cast<float>(DisabledOpacity));
    }

    FloatRect fieldRect = borderRect.Rect();
    FloatSize corner(5, 5);
    Path path;
    path.AddRoundedRect(fieldRect, corner);
    fieldRect.Inflate(-ButtonBorderSize);
    corner.Expand(-ButtonBorderSize, -ButtonBorderSize);
    path.AddRoundedRect(fieldRect, corner);
    graphicsContext.SetFillRule(WindRule::EvenOdd);
    graphicsContext.SetFillColor(buttonBorderColor);
    graphicsContext.FillPath(path);
    path.Clear();

    path.AddRoundedRect(fieldRect, corner);
    graphicsContext.SetFillRule(WindRule::NonZero);
    if (style.States.Contains(ControlStyle::State::Pressed))
    {
      graphicsContext.SetFillColor(buttonBackgroundPressedColor);
    }
    else if (style.States.Contains(ControlStyle::State::Enabled)
             && style.States.Contains(ControlStyle::State::Hovered))
    {
      graphicsContext.SetFillColor(buttonBackgroundHoveredColor);
    }
    else
    {
      graphicsContext.SetFillColor(buttonBackgroundColor);
    }
    graphicsContext.FillPath(path);

    if (style.States.Contains(ControlStyle::State::Focused))
    {
      PaintFocus(graphicsContext, borderRect.Rect(), ButtonFocusOffset, SystemFocusRingColor());
    }

    if (!style.States.Contains(ControlStyle::State::Enabled))
    {
      graphicsContext.EndTransparencyLayer();
    }
  }
}
