#include "Krystal.Booey/Platform/ScrollableArea.hpp"
#include "Krystal.Booey/Platform/PlatformWheelEvent.hpp"
#include "Krystal.Booey/Platform/Scrollbar.hpp"
#include "Krystal.Booey/Platform/ScrollbarTheme.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  bool ScrollableArea::ScrollToPositionWithoutAnimation(const IntPoint &position,
                                                        ScrollClamping clamping) noexcept
  {
    // FIXME: The scroll animator holds the current position and stops animated scrolls, once there is one.
    auto adjustedPosition = clamping == ScrollClamping::Clamped
                              ? position.ConstrainedBetween(MinimumScrollPosition(), MaximumScrollPosition())
                              : position;

    if (adjustedPosition == GetScrollPosition() && !ScrollOriginChanged())
    {
      return false;
    }

    // An early return here if the position hasn't changed breaks an iOS RTL scrolling test:
    // webkit.org/b/230450.
    auto scrollType = CurrentScrollType();
    auto positionClamping =
      scrollType == ScrollType::User ? ScrollClamping::Unclamped : ScrollClamping::Clamped;
    if (!RequestScrollToPosition(adjustedPosition, {scrollType, positionClamping, ScrollIsAnimated::No}))
    {
      ScrollPositionChanged(adjustedPosition);
    }
    ScrollDidEnd();
    return true;
  }

  void ScrollableArea::ScrollToOffsetWithoutAnimation(const IntPoint &offset,
                                                      ScrollClamping clamping) noexcept
  {
    auto position = ScrollPositionFromOffset(offset, ToIntSize(ScrollOrigin()));
    (void)ScrollToPositionWithoutAnimation(position, clamping);
  }

  void ScrollableArea::NotifyScrollPositionChanged(const ScrollPosition &position) noexcept
  {
    ScrollPositionChanged(position);
  }

  void ScrollableArea::ScrollPositionChanged(const ScrollPosition &position) noexcept
  {
    // Tell the derived class to scroll its contents.
    SetScrollOffset(ScrollOffsetFromPosition(position));

    // Tell the scrollbars to update their thumb postions.
    // FIXME: Invalidate overlay scrollbars, and notify the scrollbars controller, scroll anchoring and anchor
    // positioning, once there are any.
    if (auto *horizontalScrollbar = this->HorizontalScrollbar())
    {
      horizontalScrollbar->OffsetDidChange();
    }
    if (auto *verticalScrollbar = this->VerticalScrollbar())
    {
      verticalScrollbar->OffsetDidChange();
    }
  }

  void ScrollableArea::SetScrollOrigin(const IntPoint &origin) noexcept
  {
    if (_scrollOrigin != origin)
    {
      _scrollOrigin = origin;
      _scrollOriginChanged = true;
    }
  }

  bool ScrollableArea::AllowsHorizontalScrolling() const noexcept
  {
    auto *horizontalScrollbar = this->HorizontalScrollbar();
    return horizontalScrollbar && horizontalScrollbar->Enabled();
  }

  bool ScrollableArea::AllowsVerticalScrolling() const noexcept
  {
    auto *verticalScrollbar = this->VerticalScrollbar();
    return verticalScrollbar && verticalScrollbar->Enabled();
  }

  bool ScrollableArea::HasOverlayScrollbars() const noexcept
  {
    return (VerticalScrollbar() && VerticalScrollbar()->IsOverlayScrollbar())
           || (HorizontalScrollbar() && HorizontalScrollbar()->IsOverlayScrollbar());
  }

  bool ScrollableArea::CanShowNonOverlayScrollbars() const noexcept
  {
    return CanHaveScrollbars() && !ScrollbarTheme::Theme().UsesOverlayScrollbars();
  }

  int32 ScrollableArea::HorizontalScrollbarIntrusion() const noexcept
  {
    return VerticalScrollbar() ? VerticalScrollbar()->OccupiedWidth() : 0;
  }

  int32 ScrollableArea::VerticalScrollbarIntrusion() const noexcept
  {
    return HorizontalScrollbar() ? HorizontalScrollbar()->OccupiedHeight() : 0;
  }

  IntSize ScrollableArea::ScrollbarIntrusion() const noexcept
  {
    return {HorizontalScrollbarIntrusion(), VerticalScrollbarIntrusion()};
  }

  ScrollOffset ScrollableArea::GetScrollOffset() const noexcept
  {
    return ScrollOffsetFromPosition(GetScrollPosition());
  }

  ScrollPosition ScrollableArea::MinimumScrollPosition() const noexcept
  {
    return ScrollPositionFromOffset(ScrollPosition());
  }

  ScrollPosition ScrollableArea::MaximumScrollPosition() const noexcept
  {
    return ScrollPositionFromOffset(ScrollPosition(TotalContentsSize() - VisibleSize()));
  }

  ScrollOffset ScrollableArea::MaximumScrollOffset() const noexcept
  {
    return ScrollOffset(TotalContentsSize() - VisibleSize());
  }

  ScrollPosition ScrollableArea::ScrollPositionFromOffset(ScrollOffset offset) const noexcept
  {
    return ScrollPositionFromOffset(offset, ToIntSize(_scrollOrigin));
  }

  ScrollOffset ScrollableArea::ScrollOffsetFromPosition(ScrollPosition position) const noexcept
  {
    return ScrollOffsetFromPosition(position, ToIntSize(_scrollOrigin));
  }

  bool ScrollableArea::ScrolledToTop() const noexcept
  {
    return GetScrollPosition().Y() <= MinimumScrollPosition().Y();
  }

  bool ScrollableArea::ScrolledToBottom() const noexcept
  {
    return GetScrollPosition().Y() >= MaximumScrollPosition().Y();
  }

  bool ScrollableArea::ScrolledToLeft() const noexcept
  {
    return GetScrollPosition().X() <= MinimumScrollPosition().X();
  }

  bool ScrollableArea::ScrolledToRight() const noexcept
  {
    return GetScrollPosition().X() >= MaximumScrollPosition().X();
  }

  IntSize ScrollableArea::TotalContentsSize() const noexcept
  {
    IntSize totalContentsSize = ContentsSize();
    totalContentsSize.SetHeight(totalContentsSize.Height() + HeaderHeight() + FooterHeight());
    return totalContentsSize;
  }

  IntSize ScrollableArea::ReachableTotalContentsSize() const noexcept
  {
    return TotalContentsSize();
  }

  IntRect ScrollableArea::VisibleContentRect() const noexcept
  {
    return VisibleContentRectInternal(VisibleContentRectIncludesScrollbars::No);
  }

  IntRect ScrollableArea::VisibleContentRectIncludingScrollbars() const noexcept
  {
    return VisibleContentRectInternal(VisibleContentRectIncludesScrollbars::Yes);
  }

  IntRect ScrollableArea::VisibleContentRectInternal(
    VisibleContentRectIncludesScrollbars scrollbarInclusion) const noexcept
  {
    int32 verticalScrollbarWidth = 0;
    int32 horizontalScrollbarHeight = 0;

    if (scrollbarInclusion == VisibleContentRectIncludesScrollbars::Yes)
    {
      if (Scrollbar *verticalBar = VerticalScrollbar())
      {
        verticalScrollbarWidth = verticalBar->OccupiedWidth();
      }
      if (Scrollbar *horizontalBar = HorizontalScrollbar())
      {
        horizontalScrollbarHeight = horizontalBar->OccupiedHeight();
      }
    }

    return IntRect(GetScrollPosition().X(), GetScrollPosition().Y(),
                   std::max(0, VisibleWidth() + verticalScrollbarWidth),
                   std::max(0, VisibleHeight() + horizontalScrollbarHeight));
  }

  void ScrollableArea::ScrollToOffsetWithoutAnimation(ScrollbarOrientation orientation, float offset) noexcept
  {
    auto currentOffset = ScrollOffsetFromPosition(GetScrollPosition());
    if (orientation == ScrollbarOrientation::Horizontal)
    {
      ScrollToOffsetWithoutAnimation(IntPoint(static_cast<int32>(offset), currentOffset.Y()));
    }
    else
    {
      ScrollToOffsetWithoutAnimation(IntPoint(currentOffset.X(), static_cast<int32>(offset)));
    }
  }

  bool ScrollableArea::Scroll(ScrollDirection direction, ScrollGranularity granularity,
                              uint32 stepCount) noexcept
  {
    RefPtr<Scrollbar> scrollbar = ShareRefPtr(ScrollbarForDirection(direction));
    if (!scrollbar)
    {
      return false;
    }

    float step = 0;
    switch (granularity)
    {
      case ScrollGranularity::Line:     step = static_cast<float>(scrollbar->LineStep()); break;
      case ScrollGranularity::Page:     step = static_cast<float>(scrollbar->PageStep()); break;
      case ScrollGranularity::Document: step = static_cast<float>(scrollbar->TotalSize()); break;
      case ScrollGranularity::Pixel:    step = scrollbar->PixelStep(); break;
    }

    auto axis = AxisFromDirection(direction);

    if (granularity == ScrollGranularity::Page && axis == ScrollEventAxis::Vertical)
    {
      step = AdjustVerticalPageScrollStepForFixedContent(step);
    }

    auto scrollDelta = step * static_cast<float>(stepCount);

    if (direction == ScrollDirection::ScrollUp || direction == ScrollDirection::ScrollLeft)
    {
      scrollDelta = -scrollDelta;
    }

    return SingleAxisScroll(axis, scrollDelta);
  }

  bool ScrollableArea::SingleAxisScroll(ScrollEventAxis axis, float delta) noexcept
  {
    // The scroll animator's singleAxisScroll, without animation or snapping.
    // FIXME: Scroll snapping and animated scrolls (scrollAnimatorEnabled), once there are those.
    auto position = GetScrollPosition();
    if (axis == ScrollEventAxis::Horizontal)
    {
      position.SetX(position.X() + static_cast<int32>(delta));
    }
    else
    {
      position.SetY(position.Y() + static_cast<int32>(delta));
    }

    auto oldScrollType = CurrentScrollType();
    SetCurrentScrollType(ScrollType::User);
    bool scrolled = ScrollToPositionWithoutAnimation(position);
    SetCurrentScrollType(oldScrollType);
    return scrolled;
  }

  bool ScrollableArea::HandleWheelEventForScrolling(const PlatformWheelEvent &wheelEvent) noexcept
  {
    if (!IsScrollableOrRubberbandable())
    {
      return false;
    }

    // The scroll animator's and the scrolling effects controller's handleWheelEvent, without animation,
    // snapping or kinetic scrolling.
    auto scrollOffset = GetScrollOffset();
    float deltaX = AllowsHorizontalScrolling() ? wheelEvent.DeltaX() : 0;
    float deltaY = AllowsVerticalScrolling() ? wheelEvent.DeltaY() : 0;
    auto minPosition = MinimumScrollOffset();
    auto maxPosition = MaximumScrollOffset();

    if ((deltaX < 0 && scrollOffset.X() >= maxPosition.X())
        || (deltaX > 0 && scrollOffset.X() <= minPosition.X()))
    {
      deltaX = 0;
    }
    if ((deltaY < 0 && scrollOffset.Y() >= maxPosition.Y())
        || (deltaY > 0 && scrollOffset.Y() <= minPosition.Y()))
    {
      deltaY = 0;
    }

    if (wheelEvent.Granularity() == PlatformWheelEventGranularity::ScrollByPageWheelEvent)
    {
      // A page of what's in view, as a click in the scrollbar's track scrolls.
      auto visibleSize = VisibleSize();
      if (deltaX)
      {
        bool negative = deltaX < 0;
        deltaX = Scrollbar::PageStepDelta(visibleSize.Width());
        if (negative)
        {
          deltaX = -deltaX;
        }
      }
      if (deltaY)
      {
        bool negative = deltaY < 0;
        deltaY = Scrollbar::PageStepDelta(visibleSize.Height());
        if (negative)
        {
          deltaY = -deltaY;
        }
      }
    }

    deltaX = -deltaX;
    deltaY = -deltaY;

    if (!deltaX && !deltaY)
    {
      return false;
    }

    // The scroll animator's immediateScrollBy.
    auto position = GetScrollPosition();
    position.Move(static_cast<int32>(std::lround(deltaX)), static_cast<int32>(std::lround(deltaY)));
    auto oldScrollType = CurrentScrollType();
    SetCurrentScrollType(ScrollType::User);
    (void)ScrollToPositionWithoutAnimation(position);
    SetCurrentScrollType(oldScrollType);
    return true;
  }

  IntPoint ScrollableArea::ConvertFromContainingViewToScrollbar(const Scrollbar &scrollbar,
                                                                const IntPoint &parentPoint) const noexcept
  {
    return parentPoint - ToIntSize(scrollbar.Location());
  }

  bool ScrollableArea::IsPinnedOnSide(BoxSide side) const noexcept
  {
    switch (side)
    {
      case BoxSide::Top:
        if (!AllowsVerticalScrolling())
        {
          return true;
        }
        return GetScrollPosition().Y() <= MinimumScrollPosition().Y();
      case BoxSide::Bottom:
        if (!AllowsVerticalScrolling())
        {
          return true;
        }
        return GetScrollPosition().Y() >= MaximumScrollPosition().Y();
      case BoxSide::Left:
        if (!AllowsHorizontalScrolling())
        {
          return true;
        }
        return GetScrollPosition().X() <= MinimumScrollPosition().X();
      case BoxSide::Right:
        if (!AllowsHorizontalScrolling())
        {
          return true;
        }
        return GetScrollPosition().X() >= MaximumScrollPosition().X();
    }
    return false;
  }

  Maybe<BoxSide> ScrollableArea::TargetSideForScrollDelta(FloatSize delta, ScrollEventAxis axis) noexcept
  {
    switch (axis)
    {
      case ScrollEventAxis::Horizontal:
        if (delta.Width() < 0)
        {
          return BoxSide::Left;
        }
        if (delta.Width() > 0)
        {
          return BoxSide::Right;
        }
        break;
      case ScrollEventAxis::Vertical:
        if (delta.Height() < 0)
        {
          return BoxSide::Top;
        }
        if (delta.Height() > 0)
        {
          return BoxSide::Bottom;
        }
        break;
    }

    return null;
  }

  FloatSize ScrollableArea::DeltaForPropagation(const FloatSize &biasedDelta) const noexcept
  {
    auto filteredDelta = biasedDelta;
    if (HorizontalOverscrollBehaviorPreventsPropagation())
    {
      filteredDelta.SetWidth(0);
    }
    if (VerticalOverscrollBehaviorPreventsPropagation())
    {
      filteredDelta.SetHeight(0);
    }
    return filteredDelta;
  }

  bool ScrollableArea::ShouldBlockScrollPropagation(const FloatSize &biasedDelta) const noexcept
  {
    bool preventsHorizontalPropagation = HorizontalOverscrollBehaviorPreventsPropagation();
    bool preventsVerticalPropagation = VerticalOverscrollBehaviorPreventsPropagation();
    if (preventsHorizontalPropagation && preventsVerticalPropagation)
    {
      return true;
    }

    if (preventsHorizontalPropagation)
    {
      return !biasedDelta.Height();
    }

    if (preventsVerticalPropagation)
    {
      return !biasedDelta.Width();
    }

    return false;
  }
}
