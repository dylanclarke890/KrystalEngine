#pragma once

#include "Krystal.Booey/Core/BoxSides.hpp"
#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Platform/Scrollbar.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class PlatformWheelEvent;
  /// @brief Something that scrolls: a scroll position within its contents, its scroll origin, and its
  /// scrollbars.
  // FIXME: The scroll animator and scrollbars controller, touch scrolling, scroll snapping, scroll anchoring,
  // rubber-banding and compositing layers come with animations, touch, snapping and compositing: scrolls are
  // immediate.
  class ScrollableArea : public CanMakeWeakPtr<ScrollableArea>, public CanMakeCheckedPtr<ScrollableArea>
  {
    KRYS_NON_COPY_MOVABLE(ScrollableArea);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ScrollableArea);

  public:
    enum class VisibleContentRectIncludesScrollbars : bool
    {
      No,
      Yes
    };

    enum class AvailableSizeChangeReason : bool
    {
      ScrollbarsChanged,
      AreaSizeChanged
    };

  private:
    // There are 8 possible combinations of writing mode and direction. Scroll origin will be non-zero in the
    // x or y axis if there is any reversed direction or writing-mode. The combinations are: writing-mode /
    // direction     scrollOrigin.x() set    scrollOrigin.y() set horizontal-tb / ltr          NO NO
    // horizontal-tb / rtl          YES                     NO
    // horizontal-bt / ltr          NO                      YES
    // horizontal-bt / rtl          YES                     YES
    // vertical-lr / ltr            NO                      NO
    // vertical-lr / rtl            NO                      YES
    // vertical-rl / ltr            YES                     NO
    // vertical-rl / rtl            YES                     YES
    IntPoint _scrollOrigin;
    ScrollClamping _scrollClamping {ScrollClamping::Clamped};
    ScrollElasticity _verticalScrollElasticity {ScrollElasticity::None};
    ScrollElasticity _horizontalScrollElasticity {ScrollElasticity::None};
    ScrollType _currentScrollType {ScrollType::User};
    ScrollAnimationStatus _scrollAnimationStatus {ScrollAnimationStatus::NotAnimating};
    bool _scrollOriginChanged {false};
    bool _isAwaitingScrollend {false};

  public:
    KRYS_NODISCARD virtual bool IsScrollView() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsRenderLayer() const noexcept
    {
      return false;
    }

    bool ScrollToPositionWithoutAnimation(const IntPoint &position,
                                          ScrollClamping clamping = ScrollClamping::Clamped) noexcept;
    void ScrollToOffsetWithoutAnimation(const IntPoint &offset,
                                        ScrollClamping clamping = ScrollClamping::Clamped) noexcept;
    void ScrollToOffsetWithoutAnimation(ScrollbarOrientation orientation, float offset) noexcept;

    /// @brief Scrolls by @p stepCount steps of @p granularity (a line, a page, the document or a pixel of the
    /// scrollbar in @p direction).
    /// @return Whether it scrolled: false without a scrollbar in that direction, or at its end.
    virtual bool Scroll(ScrollDirection direction, ScrollGranularity granularity,
                        uint32 stepCount = 1) noexcept;
    /// @brief Scrolls by @p delta pixels on @p axis, as a user scroll.
    /// @return Whether it scrolled.
    bool SingleAxisScroll(ScrollEventAxis axis, float delta) noexcept;

    /// @brief Scrolls by @p wheelEvent's deltas, as a user scroll.
    /// @return Whether it scrolled: false if it can't scroll, or is at its end in the event's direction.
    // FIXME: Animated scrolling, rubber-banding, momentum and scroll snapping, once there are those.
    virtual bool HandleWheelEventForScrolling(const PlatformWheelEvent &wheelEvent) noexcept;
    KRYS_NODISCARD virtual bool IsScrollableOrRubberbandable() const noexcept = 0;

    // Should be called when the scroll position changes externally, for example if the scroll layer position
    // is updated on the scrolling thread and we need to notify the main thread.
    void NotifyScrollPositionChanged(const ScrollPosition &position) noexcept;

    // Allows subclasses to handle scroll position updates themselves. If this member function
    // returns true, the scrollable area won't actually update the scroll position and instead
    // expect it to happen sometime in the future.
    virtual bool RequestScrollToPosition(const ScrollPosition &, const ScrollPositionChangeOptions &) noexcept
    {
      return false;
    }
    virtual void StopAsyncAnimatedScroll() noexcept
    {
    }

    KRYS_NODISCARD ScrollClamping GetScrollClamping() const noexcept
    {
      return _scrollClamping;
    }
    void SetScrollClamping(ScrollClamping clamping) noexcept
    {
      _scrollClamping = clamping;
    }

    void SetVerticalScrollElasticity(ScrollElasticity scrollElasticity) noexcept
    {
      _verticalScrollElasticity = scrollElasticity;
    }
    KRYS_NODISCARD ScrollElasticity VerticalScrollElasticity() const noexcept
    {
      return _verticalScrollElasticity;
    }

    void SetHorizontalScrollElasticity(ScrollElasticity scrollElasticity) noexcept
    {
      _horizontalScrollElasticity = scrollElasticity;
    }
    KRYS_NODISCARD ScrollElasticity HorizontalScrollElasticity() const noexcept
    {
      return _horizontalScrollElasticity;
    }

    KRYS_NODISCARD virtual ScrollbarMode HorizontalScrollbarMode() const noexcept
    {
      return ScrollbarMode::Auto;
    }
    KRYS_NODISCARD virtual ScrollbarMode VerticalScrollbarMode() const noexcept
    {
      return ScrollbarMode::Auto;
    }
    KRYS_NODISCARD bool CanHaveScrollbars() const noexcept
    {
      return HorizontalScrollbarMode() != ScrollbarMode::AlwaysOff
             || VerticalScrollbarMode() != ScrollbarMode::AlwaysOff;
    }

    KRYS_NODISCARD virtual ScrollbarWidth ScrollbarWidthStyle() const noexcept
    {
      return ScrollbarWidth::Auto;
    }

    KRYS_NODISCARD bool AllowsHorizontalScrolling() const noexcept;
    KRYS_NODISCARD bool AllowsVerticalScrolling() const noexcept;

    virtual void DidAddScrollbar(Scrollbar *, ScrollbarOrientation) noexcept
    {
    }
    virtual void WillRemoveScrollbar(Scrollbar &, ScrollbarOrientation) noexcept
    {
    }

    virtual void ContentsResized() noexcept
    {
    }

    virtual void AvailableContentSizeChanged(AvailableSizeChangeReason) noexcept
    {
    }

    // This returns information about existing scrollbars, not scrollbars that may be created in future.
    KRYS_NODISCARD bool HasOverlayScrollbars() const noexcept;
    // Returns true if any scrollbars that might be created would be non-overlay scrollbars.
    KRYS_NODISCARD virtual bool CanShowNonOverlayScrollbars() const noexcept;

    KRYS_NODISCARD virtual bool IsScrollCornerVisible() const noexcept = 0;
    KRYS_NODISCARD virtual IntRect ScrollCornerRect() const noexcept = 0;

    KRYS_NODISCARD int32 HorizontalScrollbarIntrusion() const noexcept;
    KRYS_NODISCARD int32 VerticalScrollbarIntrusion() const noexcept;
    KRYS_NODISCARD IntSize ScrollbarIntrusion() const noexcept;

    KRYS_NODISCARD virtual Scrollbar *HorizontalScrollbar() const noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual Scrollbar *VerticalScrollbar() const noexcept
    {
      return nullptr;
    }

    virtual void ScrollbarFrameRectChanged(const Scrollbar &) const noexcept
    {
    }

    KRYS_NODISCARD Scrollbar *ScrollbarForDirection(ScrollDirection direction) const noexcept
    {
      switch (direction)
      {
        case ScrollDirection::ScrollUp:
        case ScrollDirection::ScrollDown:  return VerticalScrollbar();
        case ScrollDirection::ScrollLeft:
        case ScrollDirection::ScrollRight: return HorizontalScrollbar();
      }
      return nullptr;
    }

    KRYS_NODISCARD const IntPoint &ScrollOrigin() const noexcept
    {
      return _scrollOrigin;
    }
    KRYS_NODISCARD bool ScrollOriginChanged() const noexcept
    {
      return _scrollOriginChanged;
    }

    KRYS_NODISCARD virtual ScrollPosition GetScrollPosition() const noexcept = 0;
    KRYS_NODISCARD virtual ScrollPosition MinimumScrollPosition() const noexcept;
    KRYS_NODISCARD virtual ScrollPosition MaximumScrollPosition() const noexcept;
    KRYS_NODISCARD ScrollPosition ConstrainedScrollPosition(const ScrollPosition &position) const noexcept
    {
      return position.ConstrainedBetween(MinimumScrollPosition(), MaximumScrollPosition());
    }

    KRYS_NODISCARD ScrollOffset GetScrollOffset() const noexcept;
    KRYS_NODISCARD ScrollOffset MinimumScrollOffset() const noexcept
    {
      return {};
    }
    KRYS_NODISCARD ScrollOffset MaximumScrollOffset() const noexcept;

    KRYS_NODISCARD ScrollPosition ScrollPositionFromOffset(ScrollOffset offset) const noexcept;
    KRYS_NODISCARD ScrollOffset ScrollOffsetFromPosition(ScrollPosition position) const noexcept;

    template <typename TPosition, typename TSize>
    KRYS_NODISCARD static TPosition ScrollPositionFromOffset(TPosition offset, TSize scrollOrigin) noexcept
    {
      return offset - scrollOrigin;
    }

    template <typename TPosition, typename TSize>
    KRYS_NODISCARD static TPosition ScrollOffsetFromPosition(TPosition position, TSize scrollOrigin) noexcept
    {
      return position + scrollOrigin;
    }

    KRYS_NODISCARD virtual bool ScrolledToTop() const noexcept;
    KRYS_NODISCARD virtual bool ScrolledToBottom() const noexcept;
    KRYS_NODISCARD virtual bool ScrolledToLeft() const noexcept;
    KRYS_NODISCARD virtual bool ScrolledToRight() const noexcept;

    KRYS_NODISCARD ScrollType CurrentScrollType() const noexcept
    {
      return _currentScrollType;
    }
    void SetCurrentScrollType(ScrollType scrollType) noexcept
    {
      _currentScrollType = scrollType;
    }

    // This reflects animated scrolls triggered by CSS OM View "smooth" scrolls.
    KRYS_NODISCARD ScrollAnimationStatus GetScrollAnimationStatus() const noexcept
    {
      return _scrollAnimationStatus;
    }
    void SetScrollAnimationStatus(ScrollAnimationStatus status) noexcept
    {
      _scrollAnimationStatus = status;
    }

    KRYS_NODISCARD IntRect VisibleContentRect() const noexcept;
    KRYS_NODISCARD IntRect VisibleContentRectIncludingScrollbars() const noexcept;
    KRYS_NODISCARD int32 VisibleWidth() const noexcept
    {
      return VisibleSize().Width();
    }
    KRYS_NODISCARD int32 VisibleHeight() const noexcept
    {
      return VisibleSize().Height();
    }
    KRYS_NODISCARD virtual IntSize VisibleSize() const noexcept = 0;

    KRYS_NODISCARD virtual IntSize ContentsSize() const noexcept = 0;
    KRYS_NODISCARD virtual int32 HeaderHeight() const noexcept
    {
      return 0;
    }
    KRYS_NODISCARD virtual int32 FooterHeight() const noexcept
    {
      return 0;
    }

    // The totalContentsSize() is equivalent to the contentsSize() plus the header and footer heights.
    KRYS_NODISCARD IntSize TotalContentsSize() const noexcept;
    KRYS_NODISCARD virtual IntSize ReachableTotalContentsSize() const noexcept;

    // Note that this only returns scrollable areas that can actually be scrolled.
    KRYS_NODISCARD virtual ScrollableArea *EnclosingScrollableArea() const noexcept = 0;

    KRYS_NODISCARD virtual bool ShouldPlaceVerticalScrollbarOnLeft() const noexcept = 0;

    KRYS_NODISCARD virtual bool UseDarkAppearanceForScrollbars() const noexcept
    {
      return false;
    }

    // The scrollbars tell the scrollable area what the mouse does to them.
    // FIXME: The scrollbars controller (overlay scrollbars, hover feedback), once there is one.
    virtual void MouseEnteredScrollbar(Scrollbar *) const noexcept
    {
    }
    virtual void MouseExitedScrollbar(Scrollbar *) const noexcept
    {
    }
    virtual void MouseIsDownInScrollbar(Scrollbar *, bool) const noexcept
    {
    }

    /// @brief Repaints @p rect of @p scrollbar.
    virtual void InvalidateScrollbar(Scrollbar &, const IntRect &) noexcept
    {
    }

    /// @brief Converts a point in the view that contains the scrollbars (for the frame view, the window) to
    /// @p scrollbar's coordinates.
    KRYS_NODISCARD virtual IntPoint
      ConvertFromContainingViewToScrollbar(const Scrollbar &scrollbar,
                                           const IntPoint &parentPoint) const noexcept;

    /// @brief The vertical step of a page scroll, less what fixed content covers.
    KRYS_NODISCARD virtual float AdjustVerticalPageScrollStepForFixedContent(float step) noexcept
    {
      return step;
    }

    /// @brief Whether the scroll position is at @p side's end, or the area can't scroll that way.
    KRYS_NODISCARD bool IsPinnedOnSide(BoxSide side) const noexcept;
    KRYS_NODISCARD static Maybe<BoxSide> TargetSideForScrollDelta(FloatSize delta,
                                                                  ScrollEventAxis axis) noexcept;

    KRYS_NODISCARD virtual OverscrollBehavior HorizontalOverscrollBehavior() const noexcept
    {
      return OverscrollBehavior::Auto;
    }
    KRYS_NODISCARD virtual OverscrollBehavior VerticalOverscrollBehavior() const noexcept
    {
      return OverscrollBehavior::Auto;
    }
    KRYS_NODISCARD bool HorizontalOverscrollBehaviorPreventsPropagation() const noexcept
    {
      return HorizontalOverscrollBehavior() != OverscrollBehavior::Auto;
    }
    KRYS_NODISCARD bool VerticalOverscrollBehaviorPreventsPropagation() const noexcept
    {
      return VerticalOverscrollBehavior() != OverscrollBehavior::Auto;
    }
    /// @brief @p biasedDelta, less the axes `overscroll-behavior` stops from scrolling past the area.
    KRYS_NODISCARD FloatSize DeltaForPropagation(const FloatSize &biasedDelta) const noexcept;
    KRYS_NODISCARD bool ShouldBlockScrollPropagation(const FloatSize &biasedDelta) const noexcept;

    KRYS_NODISCARD virtual bool IsHorizontalWritingMode() const noexcept
    {
      return false;
    }

    virtual void AnimatedScrollDidEnd() noexcept
    {
    }

    virtual void ScrollDidEnd() noexcept
    {
    }

  protected:
    ScrollableArea() noexcept = default;
    virtual ~ScrollableArea() noexcept = default;

    void SetScrollOrigin(const IntPoint &origin) noexcept;
    void ResetScrollOriginChanged() noexcept
    {
      _scrollOriginChanged = false;
    }

    KRYS_NODISCARD bool IsAwaitingScrollend() const noexcept
    {
      return _isAwaitingScrollend;
    }
    void SetIsAwaitingScrollend(bool isAwaitingScrollend) noexcept
    {
      _isAwaitingScrollend = isAwaitingScrollend;
    }

  private:
    KRYS_NODISCARD virtual IntRect
      VisibleContentRectInternal(VisibleContentRectIncludesScrollbars scrollbarInclusion) const noexcept;

    void ScrollPositionChanged(const ScrollPosition &position) noexcept;

    // This function should be overridden by subclasses to perform the actual
    // scroll of the content.
    virtual void SetScrollOffset(const ScrollOffset &offset) noexcept = 0;
  };
}
