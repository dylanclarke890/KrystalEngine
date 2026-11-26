/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
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

#include "Krystal.Web/Platform/Scrolling/ScrollbarTheme.hpp"

namespace krys::web
{
  /// @brief A theme that builds scrollbars from parts (buttons, the track around the thumb, the thumb): their
  /// geometry, hit testing, and painting part by part.
  class ScrollbarThemeComposite : public ScrollbarTheme
  {
  public:
    bool Paint(Scrollbar &, GraphicsContext &, const RectI &damageRect) noexcept override;

    KRYS_NODISCARD ScrollbarPart HitTest(Scrollbar &, const PointI &) noexcept override;

    void InvalidatePart(Scrollbar &, ScrollbarPart) noexcept override;

    KRYS_NODISCARD int32 ThumbPosition(Scrollbar &) noexcept override;

    KRYS_NODISCARD int32 ThumbLength(Scrollbar &) noexcept override;

    KRYS_NODISCARD int32 TrackPosition(Scrollbar &) noexcept override;

    KRYS_NODISCARD int32 TrackLength(Scrollbar &) noexcept override;

    KRYS_NODISCARD virtual bool HasButtons(Scrollbar &) noexcept = 0;

    KRYS_NODISCARD virtual bool HasThumb(Scrollbar &) noexcept = 0;

    KRYS_NODISCARD virtual RectI BackButtonRect(Scrollbar &, ScrollbarPart,
                                                bool painting = false) noexcept = 0;

    KRYS_NODISCARD virtual RectI ForwardButtonRect(Scrollbar &, ScrollbarPart,
                                                   bool painting = false) noexcept = 0;

    KRYS_NODISCARD virtual RectI TrackRect(Scrollbar &, bool painting = false) noexcept = 0;

    KRYS_NODISCARD virtual RectI ThumbRect(Scrollbar &) noexcept;

    virtual void SplitTrack(Scrollbar &, const RectI &unconstrainedTrackRect, RectI &beforeThumbRect,
                            RectI &thumb, RectI &afterThumbRect) noexcept;

    KRYS_NODISCARD virtual int32 MinimumThumbLength(Scrollbar &) noexcept;

    virtual void WillPaintScrollbar(GraphicsContext &, Scrollbar &) noexcept
    {
    }

    virtual void DidPaintScrollbar(GraphicsContext &, Scrollbar &) noexcept
    {
    }

    virtual void PaintScrollbarBackground(GraphicsContext &, Scrollbar &) noexcept
    {
    }

    virtual void PaintTrackBackground(GraphicsContext &, Scrollbar &, const RectI &) noexcept
    {
    }

    virtual void PaintTrackPiece(GraphicsContext &, Scrollbar &, const RectI &, ScrollbarPart) noexcept
    {
    }

    virtual void PaintButton(GraphicsContext &, Scrollbar &, const RectI &, ScrollbarPart) noexcept
    {
    }

    virtual void PaintThumb(GraphicsContext &, Scrollbar &, const RectI &) noexcept
    {
    }

    KRYS_NODISCARD virtual RectI ConstrainTrackRectToTrackPieces(Scrollbar &, const RectI &rect) noexcept
    {
      return rect;
    }
  };
}
