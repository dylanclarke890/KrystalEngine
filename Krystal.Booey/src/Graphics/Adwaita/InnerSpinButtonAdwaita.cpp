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

#include "Krystal.Booey/Graphics/Adwaita/InnerSpinButtonAdwaita.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/Paths/Path.hpp"

namespace krys::boo
{
  using namespace adwaita;

  InnerSpinButtonAdwaita::InnerSpinButtonAdwaita(ControlPart &part,
                                                 ControlFactoryAdwaita &controlFactory) noexcept
      : ControlAdwaita(part, controlFactory)
  {
  }

  void InnerSpinButtonAdwaita::Draw(GraphicsContext &graphicsContext, const FloatRoundedRect &borderRect,
                                    float /*deviceScaleFactor*/, const ControlStyle &style) noexcept
  {
    GraphicsContextStateSaver stateSaver(graphicsContext);

    SRGBA<uint8> spinButtonBorderColor;
    SRGBA<uint8> spinButtonBackgroundColor;
    SRGBA<uint8> spinButtonBackgroundHoveredColor;
    SRGBA<uint8> spinButtonBackgroundPressedColor;

    bool useDarkAppearance = style.States.Contains(ControlStyle::State::DarkAppearance);
    if (useDarkAppearance)
    {
      spinButtonBorderColor = SpinButtonBorderColorDark;
      spinButtonBackgroundColor = SpinButtonBackgroundColorDark;
      spinButtonBackgroundHoveredColor = SpinButtonBackgroundHoveredColorDark;
      spinButtonBackgroundPressedColor = SpinButtonBackgroundPressedColorDark;
    }
    else
    {
      spinButtonBorderColor = SpinButtonBorderColorLight;
      spinButtonBackgroundColor = SpinButtonBackgroundColorLight;
      spinButtonBackgroundHoveredColor = SpinButtonBackgroundHoveredColorLight;
      spinButtonBackgroundPressedColor = SpinButtonBackgroundPressedColorLight;
    }

    FloatRect fieldRect = borderRect.Rect();
    FloatSize corner(2, 2);
    Path path;
    path.AddRoundedRect(fieldRect, corner);
    fieldRect.Inflate(-ButtonBorderSize);
    corner.Expand(-ButtonBorderSize, -ButtonBorderSize);
    path.AddRoundedRect(fieldRect, corner);
    graphicsContext.SetFillRule(WindRule::EvenOdd);
    graphicsContext.SetFillColor(spinButtonBorderColor);
    graphicsContext.FillPath(path);
    path.Clear();

    path.AddRoundedRect(fieldRect, corner);
    graphicsContext.SetFillRule(WindRule::NonZero);
    graphicsContext.SetFillColor(spinButtonBackgroundColor);
    graphicsContext.FillPath(path);
    path.Clear();

    FloatRect buttonRect = fieldRect;
    buttonRect.SetHeight(fieldRect.Height() / 2.0f);
    {
      if (style.States.Contains(ControlStyle::State::SpinUp))
      {
        path.AddRoundedRect(FloatRoundedRect(buttonRect, corner, corner, {}, {}));
        if (style.States.Contains(ControlStyle::State::Pressed))
        {
          graphicsContext.SetFillColor(spinButtonBackgroundPressedColor);
        }
        else if (style.States.Contains(ControlStyle::State::Hovered))
        {
          graphicsContext.SetFillColor(spinButtonBackgroundHoveredColor);
        }
        graphicsContext.FillPath(path);
        path.Clear();
      }

      PaintArrow(graphicsContext, buttonRect, ArrowDirection::Up, useDarkAppearance);
    }

    buttonRect.Move(0, buttonRect.Height());
    {
      if (!style.States.Contains(ControlStyle::State::SpinUp))
      {
        path.AddRoundedRect(FloatRoundedRect(buttonRect, {}, {}, corner, corner));
        if (style.States.Contains(ControlStyle::State::Pressed))
        {
          graphicsContext.SetFillColor(spinButtonBackgroundPressedColor);
        }
        else if (style.States.Contains(ControlStyle::State::Hovered))
        {
          graphicsContext.SetFillColor(spinButtonBackgroundHoveredColor);
        }
        else
        {
          graphicsContext.SetFillColor(spinButtonBackgroundColor);
        }
        graphicsContext.FillPath(path);
        path.Clear();
      }

      PaintArrow(graphicsContext, buttonRect, ArrowDirection::Down, useDarkAppearance);
    }
  }
}
