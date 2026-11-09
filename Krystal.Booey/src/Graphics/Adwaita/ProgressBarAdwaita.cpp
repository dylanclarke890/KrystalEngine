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

#include "Krystal.Booey/Graphics/Adwaita/ProgressBarAdwaita.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/Paths/Path.hpp"
#include <cmath>

namespace krys::boo
{
  using namespace adwaita;

  namespace
  {
    KRYS_NODISCARD double CurrentAnimationProgress(MonotonicTime::duration animationStartTime) noexcept
    {
      using Seconds = std::chrono::duration<double>;
      auto duration = std::chrono::duration_cast<Seconds>(ProgressAnimationDuration).count();
      auto elapsed = std::chrono::duration_cast<Seconds>(MonotonicTime::Now() - animationStartTime).count();
      return std::fmod(elapsed, duration) / duration;
    }
  }

  ProgressBarAdwaita::ProgressBarAdwaita(ControlPart &part, ControlFactoryAdwaita &controlFactory) noexcept
      : ControlAdwaita(part, controlFactory)
  {
  }

  void ProgressBarAdwaita::Draw(GraphicsContext &graphicsContext, const FloatRoundedRect &borderRect,
                                float /*deviceScaleFactor*/, const ControlStyle &style) noexcept
  {
    GraphicsContextStateSaver stateSaver(graphicsContext);

    SRGBA<uint8> progressBarBackgroundColor;
    if (style.States.Contains(ControlStyle::State::DarkAppearance))
    {
      progressBarBackgroundColor = ProgressBarBackgroundColorDark;
    }
    else
    {
      progressBarBackgroundColor = ProgressBarBackgroundColorLight;
    }

    FloatRect fieldRect = borderRect.Rect();
    FloatSize corner(3, 3);
    Path path;
    path.AddRoundedRect(fieldRect, corner);
    graphicsContext.SetFillRule(WindRule::NonZero);
    graphicsContext.SetFillColor(progressBarBackgroundColor);
    graphicsContext.FillPath(path);
    path.Clear();

    bool isVertical = style.States.Contains(ControlStyle::State::VerticalWritingMode);
    auto setPrimarySize = [&fieldRect, isVertical](float size)
    {
      if (isVertical)
      {
        fieldRect.SetHeight(size);
      }
      else
      {
        fieldRect.SetWidth(size);
      }
    };
    auto getPrimarySize = [&fieldRect, isVertical]() -> float
    {
      return isVertical ? fieldRect.Height() : fieldRect.Width();
    };
    auto getBordersPrimarySize = [&borderRect, isVertical]() -> float
    {
      return isVertical ? borderRect.Rect().Height() : borderRect.Rect().Width();
    };
    auto movePrimaryAxis = [&fieldRect, isVertical](float offset)
    {
      if (isVertical)
      {
        fieldRect.Move(0, offset);
      }
      else
      {
        fieldRect.Move(offset, 0);
      }
    };

    auto &progressBarPart = OwningProgressBarPart();
    bool isDeterminate = progressBarPart.Position() >= 0;
    if (isDeterminate)
    {
      auto progressSize = getPrimarySize() * static_cast<float>(progressBarPart.Position());
      if (style.States.Contains(ControlStyle::State::InlineFlippedWritingMode))
      {
        movePrimaryAxis(getPrimarySize() - progressSize);
      }
      setPrimarySize(progressSize);
    }
    else
    {
      double animationProgress = CurrentAnimationProgress(progressBarPart.AnimationStartTime());

      // Never let the progress rect shrink smaller than 2 pixels.
      setPrimarySize(std::max<float>(2, getPrimarySize() / ProgressActivityBlocks));
      auto movableSize = getBordersPrimarySize() - getPrimarySize();

      // We want the first 0.5 units of the animation progress to represent the
      // forward motion and the second 0.5 units to represent the backward motion,
      // thus we multiply by two here to get the full sweep of the progress bar with
      // each direction.
      if (animationProgress < 0.5)
      {
        movePrimaryAxis(static_cast<float>(animationProgress * 2 * movableSize));
      }
      else
      {
        movePrimaryAxis(static_cast<float>((1.0 - animationProgress) * 2 * movableSize));
      }
    }

    path.AddRoundedRect(fieldRect, corner);
    graphicsContext.SetFillRule(WindRule::NonZero);
    graphicsContext.SetFillColor(AccentColor(style));
    graphicsContext.FillPath(path);
  }
}
