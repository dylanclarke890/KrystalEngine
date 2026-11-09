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

#include "Krystal.Booey/Graphics/Adwaita/SliderThumbAdwaita.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/Paths/Path.hpp"

namespace krys::boo
{
  using namespace adwaita;

  SliderThumbAdwaita::SliderThumbAdwaita(ControlPart &part, ControlFactoryAdwaita &controlFactory) noexcept
      : ControlAdwaita(part, controlFactory)
  {
  }

  void SliderThumbAdwaita::Draw(GraphicsContext &graphicsContext, const FloatRoundedRect &borderRect,
                                float /*deviceScaleFactor*/, const ControlStyle &style) noexcept
  {
    GraphicsContextStateSaver stateSaver(graphicsContext);

    SRGBA<uint8> sliderThumbBackgroundColor;
    SRGBA<uint8> sliderThumbBackgroundHoveredColor;
    SRGBA<uint8> sliderThumbBackgroundDisabledColor;
    SRGBA<uint8> sliderThumbBorderColor;

    if (style.States.Contains(ControlStyle::State::DarkAppearance))
    {
      sliderThumbBackgroundColor = SliderThumbBackgroundColorDark;
      sliderThumbBackgroundHoveredColor = SliderThumbBackgroundHoveredColorDark;
      sliderThumbBackgroundDisabledColor = SliderThumbBackgroundDisabledColorDark;
      sliderThumbBorderColor = SliderThumbBorderColorDark;
    }
    else
    {
      sliderThumbBackgroundColor = SliderThumbBackgroundColorLight;
      sliderThumbBackgroundHoveredColor = SliderThumbBackgroundHoveredColorLight;
      sliderThumbBackgroundDisabledColor = SliderThumbBackgroundDisabledColorLight;
      sliderThumbBorderColor = SliderThumbBorderColorLight;
    }

    FloatRect fieldRect = borderRect.Rect();
    Path path;
    path.AddEllipseInRect(fieldRect);
    fieldRect.Inflate(-SliderThumbBorderSize);
    path.AddEllipseInRect(fieldRect);
    graphicsContext.SetFillRule(WindRule::EvenOdd);
    if (style.States.Contains(ControlStyle::State::Enabled)
        && style.States.Contains(ControlStyle::State::Pressed))
    {
      graphicsContext.SetFillColor(AccentColor(style));
    }
    else
    {
      graphicsContext.SetFillColor(sliderThumbBorderColor);
    }
    graphicsContext.FillPath(path);
    path.Clear();

    path.AddEllipseInRect(fieldRect);
    graphicsContext.SetFillRule(WindRule::NonZero);
    if (!style.States.Contains(ControlStyle::State::Enabled))
    {
      graphicsContext.SetFillColor(sliderThumbBackgroundDisabledColor);
    }
    else if (style.States.Contains(ControlStyle::State::Hovered))
    {
      graphicsContext.SetFillColor(sliderThumbBackgroundHoveredColor);
    }
    else
    {
      graphicsContext.SetFillColor(sliderThumbBackgroundColor);
    }
    graphicsContext.FillPath(path);
  }
}
