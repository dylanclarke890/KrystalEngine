#include "Krystal.Booey/Platform/ScrollableArea.hpp"
#include "Krystal.Booey/Platform/ScrollbarTheme.hpp"
#include <algorithm>

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
}
