/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Controls/SliderTrackPart.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include <cmath>

namespace krys::boo
{
  Ref<SliderTrackPart> SliderTrackPart::Create(StyleAppearance type) noexcept
  {
    return AdoptRef(*new SliderTrackPart(type, {}, {}, {}, 0));
  }

  Ref<SliderTrackPart> SliderTrackPart::Create(StyleAppearance type, const IntSize &thumbSize,
                                               const IntRect &trackBounds, List<double> &&tickRatios,
                                               double thumbPosition) noexcept
  {
    return AdoptRef(
      *new SliderTrackPart(type, thumbSize, trackBounds, krys::move(tickRatios), thumbPosition));
  }

  SliderTrackPart::SliderTrackPart(StyleAppearance type, const IntSize &thumbSize, const IntRect &trackBounds,
                                   List<double> &&tickRatios, double thumbPosition) noexcept
      : ControlPart(type), _thumbSize(thumbSize), _trackBounds(trackBounds),
        _tickRatios(krys::move(tickRatios)), _thumbPosition(thumbPosition)
  {
    krys_debug_assert(type == StyleAppearance::SliderHorizontal || type == StyleAppearance::SliderVertical);
  }

  void SliderTrackPart::DrawTicks(GraphicsContext &context, const FloatRect &rect,
                                  const ControlStyle &style) const noexcept
  {
    if (_tickRatios.empty())
    {
      return;
    }

    constexpr FloatSize sliderTickSize = {1, 3};
    constexpr int32 sliderTickOffsetFromTrackCenter = -9;

    bool isHorizontal = GetType() == StyleAppearance::SliderHorizontal;

    auto trackBounds = _trackBounds;
    trackBounds.MoveBy(IntPoint(rect.Location()));

    auto tickSize = isHorizontal ? sliderTickSize : sliderTickSize.TransposedSize();
    tickSize.Scale(style.ZoomFactor);

    FloatPoint tickLocation;
    float tickRegionMargin = 0;
    float tickRegionWidth = 0;

    float offsetFromTrackCenter = sliderTickOffsetFromTrackCenter * style.ZoomFactor;

    if (isHorizontal)
    {
      tickLocation = {0, rect.Center().Y() + offsetFromTrackCenter};
      tickRegionMargin = trackBounds.X() + (_thumbSize.Width() - tickSize.Width()) / 2.0f;
      tickRegionWidth = static_cast<float>(trackBounds.Width() - _thumbSize.Width());
    }
    else
    {
      tickLocation = {rect.Center().X() + offsetFromTrackCenter, 0};
      tickRegionMargin = trackBounds.Y() + (_thumbSize.Height() - tickSize.Height()) / 2.0f;
      tickRegionWidth = static_cast<float>(trackBounds.Height() - _thumbSize.Height());
    }

    auto tickRect = FloatRect {tickLocation, tickSize};

    GraphicsContextStateSaver stateSaver(context);
    context.SetFillColor(style.TextColor);

    bool isVerticalWritingMode = style.States.Contains(ControlStyle::State::VerticalWritingMode);
    bool isInlineFlippedWritingMode = style.States.Contains(ControlStyle::State::InlineFlippedWritingMode);
    bool isInlineFlipped = (!isHorizontal && !isVerticalWritingMode) || isInlineFlippedWritingMode;

    for (auto tickRatio : _tickRatios)
    {
      double value = isInlineFlipped ? 1.0 - tickRatio : tickRatio;
      double tickPosition = std::round(tickRegionMargin + tickRegionWidth * value);
      if (isHorizontal)
      {
        tickRect.SetX(static_cast<float>(tickPosition));
      }
      else
      {
        tickRect.SetY(static_cast<float>(tickPosition));
      }
      context.FillRect(tickRect);
    }
  }

  UniquePtr<PlatformControl> SliderTrackPart::CreatePlatformControl() noexcept
  {
    return GetControlFactory().CreatePlatformSliderTrack(*this);
  }
}
