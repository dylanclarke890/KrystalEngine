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

#pragma once

#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"
#include "Krystal.Booey/Graphics/Geometry/IntRect.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  /// @brief A slider's track: where its thumb is, and the ticks of its data list's suggestions.
  class SliderTrackPart : public ControlPart
  {
  private:
    IntSize _thumbSize;
    IntRect _trackBounds;
    List<double> _tickRatios;
    double _thumbPosition;

  public:
    KRYS_NODISCARD static Ref<SliderTrackPart> Create(StyleAppearance) noexcept;
    KRYS_NODISCARD static Ref<SliderTrackPart> Create(StyleAppearance, const IntSize &thumbSize,
                                                      const IntRect &trackBounds, List<double> &&tickRatios,
                                                      double thumbPosition) noexcept;

    KRYS_NODISCARD IntSize ThumbSize() const noexcept
    {
      return _thumbSize;
    }

    void SetThumbSize(IntSize thumbSize) noexcept
    {
      _thumbSize = thumbSize;
    }

    KRYS_NODISCARD IntRect TrackBounds() const noexcept
    {
      return _trackBounds;
    }

    void SetTrackBounds(IntRect trackBounds) noexcept
    {
      _trackBounds = trackBounds;
    }

    KRYS_NODISCARD const List<double> &TickRatios() const noexcept
    {
      return _tickRatios;
    }

    void SetTickRatios(List<double> &&tickRatios) noexcept
    {
      _tickRatios = krys::move(tickRatios);
    }

    KRYS_NODISCARD double ThumbPosition() const noexcept
    {
      return _thumbPosition;
    }

    void SetThumbPosition(double thumbPosition) noexcept
    {
      _thumbPosition = thumbPosition;
    }

    void DrawTicks(GraphicsContext &, const FloatRect &, const ControlStyle &) const noexcept;

  private:
    SliderTrackPart(StyleAppearance, const IntSize &thumbSize, const IntRect &trackBounds,
                    List<double> &&tickRatios, double thumbPosition) noexcept;

    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformControl() noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::SliderTrackPart)
  KRYS_NODISCARD static bool IsType(const krys::boo::ControlPart &part) noexcept
  {
    return part.GetType() == krys::boo::StyleAppearance::SliderHorizontal
           || part.GetType() == krys::boo::StyleAppearance::SliderVertical;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
