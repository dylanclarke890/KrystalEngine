#include "Krystal.Booey/Platform/Scrollbar.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Platform/ScrollableArea.hpp"
#include "Krystal.Booey/Platform/ScrollbarTheme.hpp"

namespace krys::boo
{
  Ref<Scrollbar> Scrollbar::CreateNativeScrollbar(ScrollableArea &scrollableArea,
                                                  ScrollbarOrientation orientation,
                                                  ScrollbarWidth width) noexcept
  {
    return AdoptRef(*new Scrollbar(scrollableArea, orientation, width));
  }

  int32 Scrollbar::PixelsPerLineStep(int32) noexcept
  {
    return PixelsPerLineStep();
  }

  int32 Scrollbar::MaxOverlapBetweenPages() noexcept
  {
    static int32 maxOverlapBetweenPages = ScrollbarTheme::Theme().MaxOverlapBetweenPages();
    return maxOverlapBetweenPages;
  }

  Scrollbar::Scrollbar(ScrollableArea &scrollableArea, ScrollbarOrientation orientation,
                       ScrollbarWidth widthStyle) noexcept
      : _scrollableArea(CreateWeakPtr(&scrollableArea)), _orientation(orientation), _widthStyle(widthStyle),
        _theme(ScrollbarTheme::Theme()), _scrollTimer(*this, &Scrollbar::AutoscrollTimerFired)
  {
    // FIXME: This is ugly and would not be necessary if we fix cross-platform code to actually query for
    // scrollbar thickness and use it when sizing scrollbars (rather than leaving one dimension of the
    // scrollbar alone when sizing).
    int32 thickness = Theme().ScrollbarThickness(widthStyle);
    SetFrameRect(IntRect(0, 0, thickness, thickness));

    _currentPos = static_cast<float>(OffsetForOrientation(scrollableArea.GetScrollOffset(), _orientation));
  }

  Scrollbar::~Scrollbar() noexcept = default;

  ScrollableArea &Scrollbar::GetScrollableArea() const noexcept
  {
    return *_scrollableArea;
  }

  int32 Scrollbar::OccupiedWidth() const noexcept
  {
    return IsOverlayScrollbar() ? 0 : Width();
  }

  int32 Scrollbar::OccupiedHeight() const noexcept
  {
    return IsOverlayScrollbar() ? 0 : Height();
  }

  void Scrollbar::OffsetDidChange() noexcept
  {
    float position =
      static_cast<float>(OffsetForOrientation(GetScrollableArea().GetScrollOffset(), _orientation));
    if (position == _currentPos)
    {
      return;
    }

    int32 oldThumbPosition = Theme().ThumbPosition(*this);
    _currentPos = position;
    UpdateThumb();
    if (_pressedPart == ThumbPart)
    {
      SetPressedPos(_pressedPos + Theme().ThumbPosition(*this) - oldThumbPosition);
    }
  }

  void Scrollbar::SetProportion(int32 visibleSize, int32 totalSize) noexcept
  {
    if (visibleSize == _visibleSize && totalSize == _totalSize)
    {
      return;
    }

    _visibleSize = visibleSize;
    _totalSize = totalSize;

    UpdateThumb();
  }

  void Scrollbar::SetSteps(int32 lineStep, int32 pageStep, int32 pixelsPerStep) noexcept
  {
    _lineStep = lineStep;
    _pageStep = pageStep;
    _pixelStep = 1.0f / static_cast<float>(pixelsPerStep);
  }

  void Scrollbar::SetFrameRect(const IntRect &rect) noexcept
  {
    _frameRect = rect;
    if (_scrollableArea)
    {
      _scrollableArea->ScrollbarFrameRectChanged(*this);
    }
  }

  void Scrollbar::SetEnabled(bool enabled) noexcept
  {
    if (_enabled == enabled)
    {
      return;
    }
    _enabled = enabled;
    // FIXME: The theme's and the scrollbars controller's enabled state, once there are any.
    Invalidate();
  }

  bool Scrollbar::IsOverlayScrollbar() const noexcept
  {
    return Theme().UsesOverlayScrollbars();
  }

  bool Scrollbar::ShouldParticipateInHitTesting() const noexcept
  {
    // Non-overlay scrollbars should always participate in hit testing.
    // FIXME: Ask the scrollbars controller about overlay scrollbars, once there is one.
    return true;
  }

  void Scrollbar::UpdateThumb() noexcept
  {
    Theme().InvalidateParts(*this, ForwardTrackPart | BackTrackPart | ThumbPart);
  }

  void Scrollbar::Paint(GraphicsContext &context, const IntRect &damageRect) noexcept
  {
    // FIXME: Invalidating control tints, once there are control tints.
    if (!_frameRect.Intersects(damageRect))
    {
      return;
    }

    (void)Theme().Paint(*this, context, damageRect);
  }

  bool Scrollbar::ThumbUnderMouse() const noexcept
  {
    int32 thumbPos = Theme().TrackPosition(const_cast<Scrollbar &>(*this))
                     + Theme().ThumbPosition(const_cast<Scrollbar &>(*this));
    int32 thumbLength = Theme().ThumbLength(const_cast<Scrollbar &>(*this));
    return PressedPos() >= thumbPos && PressedPos() < thumbPos + thumbLength;
  }

  void Scrollbar::AutoscrollTimerFired() noexcept
  {
    AutoscrollPressedPart(Theme().AutoscrollTimerDelay());
  }

  void Scrollbar::AutoscrollPressedPart(MonotonicTime::duration delay) noexcept
  {
    // Don't do anything for the thumb or if nothing was pressed.
    if (_pressedPart == ThumbPart || _pressedPart == NoPart)
    {
      return;
    }

    // Handle the track.
    if ((_pressedPart == BackTrackPart || _pressedPart == ForwardTrackPart) && ThumbUnderMouse())
    {
      Theme().InvalidatePart(*this, _pressedPart);
      SetHoveredPart(ThumbPart);
      return;
    }

    // Handle the arrows and track.
    if (GetScrollableArea().Scroll(PressedPartScrollDirection(), PressedPartScrollGranularity()))
    {
      StartTimerIfNeeded(delay);
    }
  }

  void Scrollbar::StartTimerIfNeeded(MonotonicTime::duration delay) noexcept
  {
    // Don't do anything for the thumb.
    if (_pressedPart == ThumbPart)
    {
      return;
    }

    // Handle the track.  We halt track scrolling once the thumb is level
    // with us.
    if ((_pressedPart == BackTrackPart || _pressedPart == ForwardTrackPart) && ThumbUnderMouse())
    {
      Theme().InvalidatePart(*this, _pressedPart);
      SetHoveredPart(ThumbPart);
      return;
    }

    // We can't scroll if we've hit the beginning or end.
    ScrollDirection dir = PressedPartScrollDirection();
    if (dir == ScrollDirection::ScrollUp || dir == ScrollDirection::ScrollLeft)
    {
      if (_currentPos == 0)
      {
        return;
      }
    }
    else
    {
      if (_currentPos == static_cast<float>(Maximum()))
      {
        return;
      }
    }

    _scrollTimer.StartOneShot(delay);
  }

  void Scrollbar::StopTimerIfNeeded() noexcept
  {
    if (_scrollTimer.IsActive())
    {
      _scrollTimer.Stop();
    }
  }

  ScrollDirection Scrollbar::PressedPartScrollDirection() const noexcept
  {
    if (_orientation == ScrollbarOrientation::Horizontal)
    {
      if (_pressedPart == BackButtonStartPart || _pressedPart == BackButtonEndPart
          || _pressedPart == BackTrackPart)
      {
        return ScrollDirection::ScrollLeft;
      }
      return ScrollDirection::ScrollRight;
    }
    if (_pressedPart == BackButtonStartPart || _pressedPart == BackButtonEndPart
        || _pressedPart == BackTrackPart)
    {
      return ScrollDirection::ScrollUp;
    }
    return ScrollDirection::ScrollDown;
  }

  ScrollGranularity Scrollbar::PressedPartScrollGranularity() const noexcept
  {
    if (_pressedPart == BackButtonStartPart || _pressedPart == BackButtonEndPart
        || _pressedPart == ForwardButtonStartPart || _pressedPart == ForwardButtonEndPart)
    {
      return ScrollGranularity::Line;
    }
    return ScrollGranularity::Page;
  }

  void Scrollbar::MoveThumb(int32 pos, bool draggingDocument) noexcept
  {
    int32 delta = pos - _pressedPos;

    if (draggingDocument)
    {
      if (_draggingDocument)
      {
        delta = pos - _documentDragPos;
      }
      _draggingDocument = true;
      auto currentPosition = GetScrollableArea().GetScrollPosition();
      int32 destinationPosition =
        (_orientation == ScrollbarOrientation::Horizontal ? currentPosition.X() : currentPosition.Y())
        + delta;
      if (delta > 0)
      {
        destinationPosition = std::min(destinationPosition + delta, Maximum());
      }
      else if (delta < 0)
      {
        destinationPosition = std::max(destinationPosition + delta, 0);
      }
      GetScrollableArea().ScrollToOffsetWithoutAnimation(_orientation,
                                                         static_cast<float>(destinationPosition));
      _documentDragPos = pos;
      return;
    }

    if (_draggingDocument)
    {
      delta += _pressedPos - _documentDragPos;
      _draggingDocument = false;
    }

    // Drag the thumb.
    int32 thumbPos = Theme().ThumbPosition(*this);
    int32 thumbLen = Theme().ThumbLength(*this);
    int32 trackLen = Theme().TrackLength(*this);
    int32 maxPos = trackLen - thumbLen;
    if (delta > 0)
    {
      delta = std::min(maxPos - thumbPos, delta);
    }
    else if (delta < 0)
    {
      delta = std::max(-thumbPos, delta);
    }

    if (delta)
    {
      float newOffset = static_cast<float>(thumbPos + delta) * static_cast<float>(Maximum())
                        / static_cast<float>(trackLen - thumbLen);
      GetScrollableArea().ScrollToOffsetWithoutAnimation(_orientation, newOffset);
    }
  }

  void Scrollbar::SetHoveredPart(ScrollbarPart part) noexcept
  {
    if (part == _hoveredPart)
    {
      return;
    }

    if ((_hoveredPart == NoPart || part == NoPart) && Theme().InvalidateOnMouseEnterExit())
    {
      Invalidate(); // Just invalidate the whole scrollbar, since the buttons at either end change anyway.
    }
    else if (_pressedPart == NoPart)
    {
      // When there's a pressed part, we don't draw a hovered state, so there's no reason to invalidate.
      Theme().InvalidatePart(*this, part);
      Theme().InvalidatePart(*this, _hoveredPart);
    }
    _hoveredPart = part;
  }

  void Scrollbar::SetPressedPart(ScrollbarPart part) noexcept
  {
    if (_pressedPart != NoPart)
    {
      Theme().InvalidatePart(*this, _pressedPart);
    }
    _pressedPart = part;
    if (_pressedPart != NoPart)
    {
      Theme().InvalidatePart(*this, _pressedPart);
    }
    else if (_hoveredPart != NoPart)
    {
      // When we no longer have a pressed part, we can start drawing a hovered state on the hovered part.
      Theme().InvalidatePart(*this, _hoveredPart);
    }
  }

  bool Scrollbar::MouseMoved(const PlatformMouseEvent &event) noexcept
  {
    auto position = ConvertFromContainingWindow(FlooredIntPoint(event.Position()));
    if (_pressedPart == ThumbPart)
    {
      if (Theme().ShouldSnapBackToDragOrigin(*this, event))
      {
        GetScrollableArea().ScrollToOffsetWithoutAnimation(_orientation, _dragOrigin);
      }
      else
      {
        MoveThumb(_orientation == ScrollbarOrientation::Horizontal ? position.X() : position.Y(),
                  Theme().ShouldDragDocumentInsteadOfThumb(*this, event));
      }

      return true;
    }

    if (_pressedPart != NoPart)
    {
      _pressedPos = _orientation == ScrollbarOrientation::Horizontal ? position.X() : position.Y();
    }

    ScrollbarPart part = Theme().HitTest(*this, FlooredIntPoint(event.Position()));
    if (part != _hoveredPart)
    {
      if (_pressedPart != NoPart)
      {
        if (part == _pressedPart)
        {
          // The mouse is moving back over the pressed part. We need to start up the timer action again.
          StartTimerIfNeeded(Theme().AutoscrollTimerDelay());
          Theme().InvalidatePart(*this, _pressedPart);
        }
        else if (_hoveredPart == _pressedPart)
        {
          // The mouse is leaving the pressed part. Kill our timer if needed.
          StopTimerIfNeeded();
          Theme().InvalidatePart(*this, _pressedPart);
        }
      }

      SetHoveredPart(part);
    }

    return true;
  }

  void Scrollbar::MouseEntered() noexcept
  {
    GetScrollableArea().MouseEnteredScrollbar(this);
  }

  bool Scrollbar::MouseExited() noexcept
  {
    GetScrollableArea().MouseExitedScrollbar(this);
    SetHoveredPart(NoPart);
    return true;
  }

  bool Scrollbar::MouseUp(const PlatformMouseEvent &event) noexcept
  {
    SetPressedPart(NoPart);
    _pressedPos = 0;
    _draggingDocument = false;
    StopTimerIfNeeded();

    auto &scrollableArea = GetScrollableArea();
    scrollableArea.MouseIsDownInScrollbar(this, false);

    // _hoveredPart won't be updated until the next mouseMoved or mouseDown, so we have to hit test to really
    // know if the mouse has exited the scrollbar on a mouseUp.
    ScrollbarPart part = Theme().HitTest(*this, FlooredIntPoint(event.Position()));
    if (part == NoPart)
    {
      scrollableArea.MouseExitedScrollbar(this);
    }

    // FIXME: Snapping after moving the thumb (doPostThumbMoveSnapping), once there's scroll snapping.
    return true;
  }

  bool Scrollbar::MouseDown(const PlatformMouseEvent &event) noexcept
  {
    ScrollbarPart pressedPart = Theme().HitTest(*this, FlooredIntPoint(event.Position()));
    auto action = Theme().HandleMousePressEvent(*this, event, pressedPart);
    if (action == ScrollbarButtonPressAction::None)
    {
      return true;
    }

    GetScrollableArea().MouseIsDownInScrollbar(this, true);
    SetPressedPart(pressedPart);

    auto position = ConvertFromContainingWindow(FlooredIntPoint(event.Position()));
    int32 pressedPosition = _orientation == ScrollbarOrientation::Horizontal ? position.X() : position.Y();
    if (action == ScrollbarButtonPressAction::CenterOnThumb)
    {
      SetHoveredPart(ThumbPart);
      SetPressedPart(ThumbPart);
      _dragOrigin = _currentPos;
      // Set the pressed position to the middle of the thumb so that when we do the move, the delta will be
      // from the current pixel position of the thumb to the new desired position for the thumb.
      _pressedPos =
        Theme().TrackPosition(*this) + Theme().ThumbPosition(*this) + Theme().ThumbLength(*this) / 2;
      MoveThumb(pressedPosition);
      return true;
    }

    _pressedPos = pressedPosition;

    if (action == ScrollbarButtonPressAction::StartDrag)
    {
      _dragOrigin = _currentPos;
    }

    if (action == ScrollbarButtonPressAction::Scroll)
    {
      AutoscrollPressedPart(Theme().InitialAutoscrollTimerDelay());
    }

    return true;
  }

  IntPoint Scrollbar::ConvertFromContainingWindow(const IntPoint &windowPoint) const noexcept
  {
    return GetScrollableArea().ConvertFromContainingViewToScrollbar(*this, windowPoint);
  }

  void Scrollbar::InvalidateRect(const IntRect &rect) noexcept
  {
    GetScrollableArea().InvalidateScrollbar(*this, rect);
  }

  void Scrollbar::UpdateScrollbarThickness() noexcept
  {
    _widthStyle = GetScrollableArea().ScrollbarWidthStyle();
    int32 thickness = ScrollbarTheme::Theme().ScrollbarThickness(WidthStyle());
    SetFrameRect(IntRect(0, 0, thickness, thickness));
  }
}
