#pragma once

#include "Krystal.Booey/Platform/ScrollbarTheme.hpp"

namespace krys::boo
{
  /// @brief A theme that builds scrollbars from parts (buttons, the track around the thumb, the thumb): their
  /// geometry, hit testing, and painting part by part.
  class ScrollbarThemeComposite : public ScrollbarTheme
  {
  public:
    bool Paint(Scrollbar &, GraphicsContext &, const IntRect &damageRect) noexcept override;
    KRYS_NODISCARD ScrollbarPart HitTest(Scrollbar &, const IntPoint &) noexcept override;
    void InvalidatePart(Scrollbar &, ScrollbarPart) noexcept override;
    KRYS_NODISCARD int32 ThumbPosition(Scrollbar &) noexcept override;
    KRYS_NODISCARD int32 ThumbLength(Scrollbar &) noexcept override;
    KRYS_NODISCARD int32 TrackPosition(Scrollbar &) noexcept override;
    KRYS_NODISCARD int32 TrackLength(Scrollbar &) noexcept override;

    KRYS_NODISCARD virtual bool HasButtons(Scrollbar &) noexcept = 0;
    KRYS_NODISCARD virtual bool HasThumb(Scrollbar &) noexcept = 0;

    KRYS_NODISCARD virtual IntRect BackButtonRect(Scrollbar &, ScrollbarPart,
                                                  bool painting = false) noexcept = 0;
    KRYS_NODISCARD virtual IntRect ForwardButtonRect(Scrollbar &, ScrollbarPart,
                                                     bool painting = false) noexcept = 0;
    KRYS_NODISCARD virtual IntRect TrackRect(Scrollbar &, bool painting = false) noexcept = 0;
    KRYS_NODISCARD virtual IntRect ThumbRect(Scrollbar &) noexcept;

    virtual void SplitTrack(Scrollbar &, const IntRect &track, IntRect &startTrack, IntRect &thumb,
                            IntRect &endTrack) noexcept;

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
    virtual void PaintTrackBackground(GraphicsContext &, Scrollbar &, const IntRect &) noexcept
    {
    }
    virtual void PaintTrackPiece(GraphicsContext &, Scrollbar &, const IntRect &, ScrollbarPart) noexcept
    {
    }
    virtual void PaintButton(GraphicsContext &, Scrollbar &, const IntRect &, ScrollbarPart) noexcept
    {
    }
    virtual void PaintThumb(GraphicsContext &, Scrollbar &, const IntRect &) noexcept
    {
    }

    KRYS_NODISCARD virtual IntRect ConstrainTrackRectToTrackPieces(Scrollbar &, const IntRect &rect) noexcept
    {
      return rect;
    }
  };
}
