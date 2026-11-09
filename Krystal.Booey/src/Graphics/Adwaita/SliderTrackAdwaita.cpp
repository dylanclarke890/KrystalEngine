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

#include "Krystal.Booey/Graphics/Adwaita/SliderTrackAdwaita.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/Paths/Path.hpp"

namespace krys::boo
{
  using namespace adwaita;

  SliderTrackAdwaita::SliderTrackAdwaita(ControlPart &part, ControlFactoryAdwaita &controlFactory) noexcept
      : ControlAdwaita(part, controlFactory)
  {
  }

  void SliderTrackAdwaita::Draw(GraphicsContext &graphicsContext, const FloatRoundedRect &borderRect,
                                float /*deviceScaleFactor*/, const ControlStyle &style) noexcept
  {
    auto &sliderTrackPart = OwningSliderTrackPart();

    GraphicsContextStateSaver stateSaver(graphicsContext);

    FloatRect rect = borderRect.Rect();
    FloatRect fieldRect = rect;
    bool isHorizontal = sliderTrackPart.GetType() == StyleAppearance::SliderHorizontal;
    if (isHorizontal)
    {
      fieldRect.Move(0, rect.Height() / 2 - (SliderTrackSize / 2));
      fieldRect.SetHeight(SliderTrackSize);
    }
    else
    {
      fieldRect.Move(rect.Width() / 2 - (SliderTrackSize / 2), 0);
      fieldRect.SetWidth(SliderTrackSize);
    }

    SRGBA<uint8> sliderTrackBackgroundColor;
    if (style.States.Contains(ControlStyle::State::DarkAppearance))
    {
      sliderTrackBackgroundColor = SliderTrackBackgroundColorDark;
    }
    else
    {
      sliderTrackBackgroundColor = SliderTrackBackgroundColorLight;
    }

    if (!style.States.Contains(ControlStyle::State::Enabled))
    {
      graphicsContext.BeginTransparencyLayer(static_cast<float>(DisabledOpacity));
    }

    FloatSize corner(3, 3);
    Path path;

    path.AddRoundedRect(fieldRect, corner);
    graphicsContext.SetFillRule(WindRule::NonZero);
    graphicsContext.SetFillColor(sliderTrackBackgroundColor);
    graphicsContext.FillPath(path);
    path.Clear();

    FloatRect rangeRect = fieldRect;
    FloatRoundedRect::Radii corners;
    if (isHorizontal)
    {
      float offset = rangeRect.Width() * static_cast<float>(sliderTrackPart.ThumbPosition());
      if (style.States.Contains(ControlStyle::State::InlineFlippedWritingMode))
      {
        rangeRect.Move(rangeRect.Width() - offset, 0);
        rangeRect.SetWidth(offset);
        corners.SetTopRight(corner);
        corners.SetBottomRight(corner);
      }
      else
      {
        rangeRect.SetWidth(offset);
        corners.SetTopLeft(corner);
        corners.SetBottomLeft(corner);
      }
    }
    else
    {
      float offset = rangeRect.Height() * static_cast<float>(sliderTrackPart.ThumbPosition());
      if (style.States.Contains(ControlStyle::State::VerticalWritingMode))
      {
        rangeRect.SetHeight(offset);
        corners.SetTopLeft(corner);
        corners.SetTopRight(corner);
      }
      else
      {
        rangeRect.Move(0, rangeRect.Height() - offset);
        rangeRect.SetHeight(offset);
        corners.SetBottomLeft(corner);
        corners.SetBottomRight(corner);
      }
    }

    path.AddRoundedRect(FloatRoundedRect(rangeRect, corners));
    graphicsContext.SetFillRule(WindRule::NonZero);
    graphicsContext.SetFillColor(AccentColor(style));
    graphicsContext.FillPath(path);

    sliderTrackPart.DrawTicks(graphicsContext, borderRect.Rect(), style);

    if (style.States.Contains(ControlStyle::State::Focused))
    {
      // Sliders support accent-color, so we want to color their focus rings too
      Color focusRingColor =
        AccentColor(style).ColorWithAlphaMultipliedBy(static_cast<float>(FocusRingOpacity));
      PaintFocus(graphicsContext, fieldRect, SliderTrackFocusOffset, focusRingColor, PaintRounded::Yes);
    }

    if (!style.States.Contains(ControlStyle::State::Enabled))
    {
      graphicsContext.EndTransparencyLayer();
    }
  }
}
