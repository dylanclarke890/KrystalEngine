#include "Krystal.Booey/Platform/ScrollbarThemeComposite.hpp"
#include "Krystal.Booey/Platform/Scrollbar.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  namespace
  {
    // Returns the size represented by track taking into account scrolling past the end of the document.
    KRYS_NODISCARD float UsedTotalSize(Scrollbar &scrollbar) noexcept
    {
      float overhangAtStart = -scrollbar.CurrentPos();
      float overhangAtEnd = scrollbar.CurrentPos() + static_cast<float>(scrollbar.VisibleSize())
                            - static_cast<float>(scrollbar.TotalSize());
      float overhang = std::max(0.0f, std::max(overhangAtStart, overhangAtEnd));
      return static_cast<float>(scrollbar.TotalSize()) + overhang;
    }
  }

  bool ScrollbarThemeComposite::Paint(Scrollbar &scrollbar, GraphicsContext &graphicsContext,
                                      const IntRect &damageRect) noexcept
  {
    // Create the ScrollbarControlPartMask based on the damageRect
    ScrollbarControlPartMask scrollMask = NoPart;

    IntRect backButtonStartPaintRect;
    IntRect backButtonEndPaintRect;
    IntRect forwardButtonStartPaintRect;
    IntRect forwardButtonEndPaintRect;
    if (HasButtons(scrollbar))
    {
      backButtonStartPaintRect = BackButtonRect(scrollbar, BackButtonStartPart, true);
      if (damageRect.Intersects(backButtonStartPaintRect))
      {
        scrollMask |= BackButtonStartPart;
      }
      backButtonEndPaintRect = BackButtonRect(scrollbar, BackButtonEndPart, true);
      if (damageRect.Intersects(backButtonEndPaintRect))
      {
        scrollMask |= BackButtonEndPart;
      }
      forwardButtonStartPaintRect = ForwardButtonRect(scrollbar, ForwardButtonStartPart, true);
      if (damageRect.Intersects(forwardButtonStartPaintRect))
      {
        scrollMask |= ForwardButtonStartPart;
      }
      forwardButtonEndPaintRect = ForwardButtonRect(scrollbar, ForwardButtonEndPart, true);
      if (damageRect.Intersects(forwardButtonEndPaintRect))
      {
        scrollMask |= ForwardButtonEndPart;
      }
    }

    IntRect startTrackRect;
    IntRect thumbRect;
    IntRect endTrackRect;
    IntRect trackPaintRect = TrackRect(scrollbar, true);
    if (damageRect.Intersects(trackPaintRect))
    {
      scrollMask |= TrackBGPart;
    }
    bool thumbPresent = HasThumb(scrollbar);
    if (thumbPresent)
    {
      IntRect track = TrackRect(scrollbar);
      SplitTrack(scrollbar, track, startTrackRect, thumbRect, endTrackRect);
      if (damageRect.Intersects(thumbRect))
      {
        scrollMask |= ThumbPart;
      }
      if (damageRect.Intersects(startTrackRect))
      {
        scrollMask |= BackTrackPart;
      }
      if (damageRect.Intersects(endTrackRect))
      {
        scrollMask |= ForwardTrackPart;
      }
    }

    WillPaintScrollbar(graphicsContext, scrollbar);

    // Paint the scrollbar background (only used by custom CSS scrollbars).
    PaintScrollbarBackground(graphicsContext, scrollbar);

    // Paint the back and forward buttons.
    if (scrollMask & BackButtonStartPart)
    {
      PaintButton(graphicsContext, scrollbar, backButtonStartPaintRect, BackButtonStartPart);
    }
    if (scrollMask & BackButtonEndPart)
    {
      PaintButton(graphicsContext, scrollbar, backButtonEndPaintRect, BackButtonEndPart);
    }
    if (scrollMask & ForwardButtonStartPart)
    {
      PaintButton(graphicsContext, scrollbar, forwardButtonStartPaintRect, ForwardButtonStartPart);
    }
    if (scrollMask & ForwardButtonEndPart)
    {
      PaintButton(graphicsContext, scrollbar, forwardButtonEndPaintRect, ForwardButtonEndPart);
    }

    if (scrollMask & TrackBGPart)
    {
      PaintTrackBackground(graphicsContext, scrollbar, trackPaintRect);
    }

    if ((scrollMask & ForwardTrackPart) || (scrollMask & BackTrackPart))
    {
      // Paint the track pieces above and below the thumb.
      if (scrollMask & BackTrackPart)
      {
        PaintTrackPiece(graphicsContext, scrollbar, startTrackRect, BackTrackPart);
      }
      if (scrollMask & ForwardTrackPart)
      {
        PaintTrackPiece(graphicsContext, scrollbar, endTrackRect, ForwardTrackPart);
      }

      // FIXME: The tickmarks (find-in-page matches), once there's find-in-page.
    }

    // Paint the thumb.
    if (scrollMask & ThumbPart)
    {
      PaintThumb(graphicsContext, scrollbar, thumbRect);
    }

    DidPaintScrollbar(graphicsContext, scrollbar);
    return true;
  }

  ScrollbarPart ScrollbarThemeComposite::HitTest(Scrollbar &scrollbar, const IntPoint &position) noexcept
  {
    ScrollbarPart result = NoPart;
    if (!scrollbar.Enabled())
    {
      return result;
    }

    IntPoint testPosition = scrollbar.ConvertFromContainingWindow(position);
    testPosition.Move(scrollbar.X(), scrollbar.Y());

    if (!scrollbar.FrameRect().Contains(testPosition))
    {
      return NoPart;
    }

    result = ScrollbarBGPart;

    IntRect track = TrackRect(scrollbar);
    if (track.Contains(testPosition))
    {
      IntRect beforeThumbRect;
      IntRect thumbRect;
      IntRect afterThumbRect;
      SplitTrack(scrollbar, track, beforeThumbRect, thumbRect, afterThumbRect);
      if (thumbRect.Contains(testPosition))
      {
        result = ThumbPart;
      }
      else if (beforeThumbRect.Contains(testPosition))
      {
        result = BackTrackPart;
      }
      else if (afterThumbRect.Contains(testPosition))
      {
        result = ForwardTrackPart;
      }
      else
      {
        result = TrackBGPart;
      }
    }
    else if (BackButtonRect(scrollbar, BackButtonStartPart).Contains(testPosition))
    {
      result = BackButtonStartPart;
    }
    else if (BackButtonRect(scrollbar, BackButtonEndPart).Contains(testPosition))
    {
      result = BackButtonEndPart;
    }
    else if (ForwardButtonRect(scrollbar, ForwardButtonStartPart).Contains(testPosition))
    {
      result = ForwardButtonStartPart;
    }
    else if (ForwardButtonRect(scrollbar, ForwardButtonEndPart).Contains(testPosition))
    {
      result = ForwardButtonEndPart;
    }
    return result;
  }

  void ScrollbarThemeComposite::InvalidatePart(Scrollbar &scrollbar, ScrollbarPart part) noexcept
  {
    if (part == NoPart)
    {
      return;
    }

    IntRect result;
    switch (part)
    {
      case BackButtonStartPart:    result = BackButtonRect(scrollbar, BackButtonStartPart, true); break;
      case BackButtonEndPart:      result = BackButtonRect(scrollbar, BackButtonEndPart, true); break;
      case ForwardButtonStartPart: result = ForwardButtonRect(scrollbar, ForwardButtonStartPart, true); break;
      case ForwardButtonEndPart:   result = ForwardButtonRect(scrollbar, ForwardButtonEndPart, true); break;
      case TrackBGPart:            result = TrackRect(scrollbar, true); break;
      case ScrollbarBGPart:        result = scrollbar.FrameRect(); break;
      default:
      {
        IntRect beforeThumbRect, thumbRect, afterThumbRect;
        SplitTrack(scrollbar, TrackRect(scrollbar), beforeThumbRect, thumbRect, afterThumbRect);
        if (part == BackTrackPart)
        {
          result = beforeThumbRect;
        }
        else if (part == ForwardTrackPart)
        {
          result = afterThumbRect;
        }
        else
        {
          result = thumbRect;
        }
      }
    }
    result.MoveBy(-scrollbar.Location());
    scrollbar.InvalidateRect(result);
  }

  void ScrollbarThemeComposite::SplitTrack(Scrollbar &scrollbar, const IntRect &unconstrainedTrackRect,
                                           IntRect &beforeThumbRect, IntRect &thumbRect,
                                           IntRect &afterThumbRect) noexcept
  {
    // This function won't even get called unless we're big enough to have some combination of these three
    // rects where at least one of them is non-empty.
    IntRect trackRect = ConstrainTrackRectToTrackPieces(scrollbar, unconstrainedTrackRect);
    int32 thickness =
      scrollbar.Orientation() == ScrollbarOrientation::Horizontal ? scrollbar.Height() : scrollbar.Width();
    int32 thumbPos = ThumbPosition(scrollbar);
    if (scrollbar.Orientation() == ScrollbarOrientation::Horizontal)
    {
      thumbRect = IntRect(trackRect.X() + thumbPos, trackRect.Y() + (trackRect.Height() - thickness) / 2,
                          ThumbLength(scrollbar), thickness);
      beforeThumbRect =
        IntRect(trackRect.X(), trackRect.Y(), thumbPos + thumbRect.Width() / 2, trackRect.Height());
      afterThumbRect = IntRect(trackRect.X() + beforeThumbRect.Width(), trackRect.Y(),
                               trackRect.MaxX() - beforeThumbRect.MaxX(), trackRect.Height());
    }
    else
    {
      thumbRect = IntRect(trackRect.X() + (trackRect.Width() - thickness) / 2, trackRect.Y() + thumbPos,
                          thickness, ThumbLength(scrollbar));
      beforeThumbRect =
        IntRect(trackRect.X(), trackRect.Y(), trackRect.Width(), thumbPos + thumbRect.Height() / 2);
      afterThumbRect = IntRect(trackRect.X(), trackRect.Y() + beforeThumbRect.Height(), trackRect.Width(),
                               trackRect.MaxY() - beforeThumbRect.MaxY());
    }
  }

  int32 ScrollbarThemeComposite::ThumbPosition(Scrollbar &scrollbar) noexcept
  {
    if (scrollbar.Enabled())
    {
      float size = UsedTotalSize(scrollbar) - static_cast<float>(scrollbar.VisibleSize());
      // Avoid doing a floating point divide by zero and return 1 when usedTotalSize == visibleSize.
      if (!size)
      {
        return 1;
      }
      float pos = std::max(0.0f, scrollbar.CurrentPos())
                  * static_cast<float>(TrackLength(scrollbar) - ThumbLength(scrollbar)) / size;
      return (pos < 1 && pos > 0) ? 1 : static_cast<int32>(pos);
    }
    return 0;
  }

  int32 ScrollbarThemeComposite::ThumbLength(Scrollbar &scrollbar) noexcept
  {
    if (!scrollbar.Enabled())
    {
      return 0;
    }

    float proportion = static_cast<float>(scrollbar.VisibleSize()) / UsedTotalSize(scrollbar);
    int32 trackLen = TrackLength(scrollbar);
    int32 length = static_cast<int32>(std::round(proportion * static_cast<float>(trackLen)));
    length = std::max(length, MinimumThumbLength(scrollbar));
    if (length > trackLen)
    {
      length =
        0; // Once the thumb is below the track length, it just goes away (to make more room for the track).
    }
    return length;
  }

  int32 ScrollbarThemeComposite::MinimumThumbLength(Scrollbar &scrollbar) noexcept
  {
    return ScrollbarThickness(scrollbar.WidthStyle());
  }

  int32 ScrollbarThemeComposite::TrackPosition(Scrollbar &scrollbar) noexcept
  {
    IntRect constrainedTrackRect = ConstrainTrackRectToTrackPieces(scrollbar, TrackRect(scrollbar));
    return scrollbar.Orientation() == ScrollbarOrientation::Horizontal
             ? constrainedTrackRect.X() - scrollbar.X()
             : constrainedTrackRect.Y() - scrollbar.Y();
  }

  int32 ScrollbarThemeComposite::TrackLength(Scrollbar &scrollbar) noexcept
  {
    IntRect constrainedTrackRect = ConstrainTrackRectToTrackPieces(scrollbar, TrackRect(scrollbar));
    return scrollbar.Orientation() == ScrollbarOrientation::Horizontal ? constrainedTrackRect.Width()
                                                                       : constrainedTrackRect.Height();
  }

  IntRect ScrollbarThemeComposite::ThumbRect(Scrollbar &scrollbar) noexcept
  {
    if (!HasThumb(scrollbar))
    {
      return IntRect();
    }

    IntRect track = TrackRect(scrollbar);
    IntRect startTrackRect;
    IntRect thumbRect;
    IntRect endTrackRect;
    SplitTrack(scrollbar, track, startTrackRect, thumbRect, endTrackRect);

    return thumbRect;
  }
}
