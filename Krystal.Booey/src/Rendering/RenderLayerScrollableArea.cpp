#include "Krystal.Booey/Rendering/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Platform/ScrollAlignment.hpp"
#include "Krystal.Booey/Platform/ScrollbarTheme.hpp"
#include "Krystal.Booey/Rendering/LayoutScope.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Scrollbars/ScrollbarWidth.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include <algorithm>

namespace krys::boo
{
  RenderLayerScrollableArea::RenderLayerScrollableArea(RenderLayer &layer) noexcept : _layer(layer)
  {
    // FIXME: The scroll anchoring controller (cssScrollAnchoringEnabled), once there's scroll anchoring.
  }

  RenderLayerScrollableArea::~RenderLayerScrollableArea() noexcept = default;

  void RenderLayerScrollableArea::Clear() noexcept
  {
    // FIXME: Unregister from the frame view's scrollable areas and animated scrolls, once it keeps them.

    auto &renderer = _layer.Renderer();
    if (auto *element = renderer.GetElement())
    {
      element->SetSavedLayerScrollPosition(_scrollPosition);
    }

    DestroyScrollbar(ScrollbarOrientation::Horizontal);
    DestroyScrollbar(ScrollbarOrientation::Vertical);

    // FIXME: The custom scroll corner and resizer, once there are any.
  }

  void RenderLayerScrollableArea::RestoreScrollPosition() noexcept
  {
    auto *element = _layer.Renderer().GetElement();
    if (!element)
    {
      return;
    }

    if (_layer.GetRenderBox())
    {
      // We save and restore only the scrollOffset as the other scroll values are recalculated.
      _scrollPosition = element->SavedLayerScrollPosition();
      // FIXME: Tell the scroll animator the current position, once there is one.
    }

    element->SetSavedLayerScrollPosition({});
  }

  bool RenderLayerScrollableArea::ShouldPlaceVerticalScrollbarOnLeft() const noexcept
  {
    return _layer.Renderer().Style().ShouldPlaceVerticalScrollbarOnLeft();
  }

  void RenderLayerScrollableArea::SetPostLayoutScrollPosition(Maybe<ScrollPosition> position) noexcept
  {
    _postLayoutScrollPosition = position;
  }

  void RenderLayerScrollableArea::ApplyPostLayoutScrollPositionIfNeeded() noexcept
  {
    if (!_postLayoutScrollPosition)
    {
      return;
    }

    (void)ScrollToOffset(ScrollOffsetFromPosition(*_postLayoutScrollPosition));
    _postLayoutScrollPosition = null;
  }

  void RenderLayerScrollableArea::ScrollToXPosition(int32 x,
                                                    const ScrollPositionChangeOptions &options) noexcept
  {
    ScrollPosition position(x, _scrollPosition.Y());
    SetScrollPosition(position, options);
  }

  void RenderLayerScrollableArea::ScrollToYPosition(int32 y,
                                                    const ScrollPositionChangeOptions &options) noexcept
  {
    ScrollPosition position(_scrollPosition.X(), y);
    SetScrollPosition(position, options);
  }

  void RenderLayerScrollableArea::SetScrollPosition(const ScrollPosition &position,
                                                    const ScrollPositionChangeOptions &options) noexcept
  {
    (void)ScrollToOffset(ScrollOffsetFromPosition(position), options);
  }

  ScrollOffset RenderLayerScrollableArea::ClampScrollOffset(const ScrollOffset &scrollOffset) const noexcept
  {
    return scrollOffset.ConstrainedBetween(MinimumScrollOffset(), MaximumScrollOffset());
  }

  ScrollOffset RenderLayerScrollableArea::ScrollToOffset(const ScrollOffset &scrollOffset,
                                                         const ScrollPositionChangeOptions &options) noexcept
  {
    // FIXME: Cancel running scroll animations, once there are any.
    ScrollOffset clampedScrollOffset =
      options.Clamping == ScrollClamping::Clamped ? ClampScrollOffset(scrollOffset) : scrollOffset;
    if (clampedScrollOffset == this->GetScrollOffset())
    {
      return clampedScrollOffset;
    }

    auto previousScrollType = CurrentScrollType();
    SetCurrentScrollType(options.Type);

    // FIXME: Snap the offset (scrollOffsetAdjustedForSnapping) and scroll with an animation
    // (ScrollIsAnimated::Yes), once there are scroll snapping and animated scrolls.
    ScrollOffset snappedOffset = clampedScrollOffset;
    auto snappedPosition = ScrollPositionFromOffset(snappedOffset);
    if (!RequestScrollToPosition(snappedPosition, options))
    {
      (void)ScrollToPositionWithoutAnimation(snappedPosition, options.Clamping);
    }

    SetCurrentScrollType(previousScrollType);
    return snappedOffset;
  }

  void RenderLayerScrollableArea::ScrollTo(const ScrollPosition &position) noexcept
  {
    RenderBox *box = _layer.GetRenderBox();
    if (!box)
    {
      return;
    }

    ScrollPosition newPosition = position;
    if (!box->IsHTMLMarquee())
    {
      // Ensure that the dimensions will be computed if they need to be (for overflow:hidden blocks).
      if (_scrollDimensionsDirty)
      {
        ComputeScrollDimensions();
      }
    }

    if (_scrollPosition == newPosition && GetScrollAnimationStatus() == ScrollAnimationStatus::NotAnimating)
    {
      // FIXME: The composited scroll position reconciliation, once there's compositing.
      return;
    }

    _scrollPosition = newPosition;
    _layer.SetSelfAndDescendantsNeedPositionUpdate();

    auto &renderer = _layer.Renderer();
    if (auto *element = renderer.GetElement())
    {
      element->SetSavedLayerScrollPosition(_scrollPosition);
    }

    RenderView &view = renderer.View();

    // Update the positions of our child layers (if needed as only fixed layers should be impacted by a
    // scroll). We don't update compositing layers, because we need to do a deep update from the compositing
    // ancestor.
    if (!view.FrameView().LayoutContext().IsInRenderTreeLayout())
    {
      // If we're in the middle of layout, we'll just update layers once layout has finished.
      _layer.UpdateLayerPositionsAfterOverflowScroll();
      // FIXME: The compositing layers, rendering-dependent regions and debug overlays, once there are any.
    }

    // FIXME: The caret rect, the fake mouse move, repaint and the frame view's scroll offset and viewport
    // notifications, once there are selection, fake mouse moves and repaint tracking.

    // Schedule the scroll and scroll-related DOM events.
    if (auto *element = renderer.GetElement())
    {
      SetIsAwaitingScrollend(true);
      element->NodeDocument().AddPendingScrollEventTarget(*element, dom::ScrollEventType::Scroll);
    }
  }

  void RenderLayerScrollableArea::ScrollDidEnd() noexcept
  {
    if (!IsAwaitingScrollend())
    {
      return;
    }
    SetIsAwaitingScrollend(false);
    if (auto *element = _layer.Renderer().GetElement())
    {
      element->NodeDocument().AddPendingScrollEventTarget(*element, dom::ScrollEventType::Scrollend);
    }
  }

  int32 RenderLayerScrollableArea::ScrollWidth() const noexcept
  {
    krys_debug_assert(_layer.GetRenderBox());
    if (_scrollDimensionsDirty)
    {
      const_cast<RenderLayerScrollableArea *>(this)->ComputeScrollDimensions();
    }
    // FIXME: This should use snappedIntSize() instead with absolute coordinates.
    return _scrollWidth;
  }

  int32 RenderLayerScrollableArea::ScrollHeight() const noexcept
  {
    krys_debug_assert(_layer.GetRenderBox());
    if (_scrollDimensionsDirty)
    {
      const_cast<RenderLayerScrollableArea *>(this)->ComputeScrollDimensions();
    }
    // FIXME: This should use snappedIntSize() instead with absolute coordinates.
    return _scrollHeight;
  }

  bool RenderLayerScrollableArea::ScrollsOverflow() const noexcept
  {
    auto *renderer = DynamicDowncast<RenderBox>(_layer.Renderer());
    return renderer && renderer->ScrollsOverflow();
  }

  bool RenderLayerScrollableArea::CanUseCompositedScrolling() const noexcept
  {
    // FIXME: Asynchronous overflow scrolling (asyncOverflowScrollingEnabled), once there's compositing.
    return false;
  }

  void RenderLayerScrollableArea::SetScrollOffset(const ScrollOffset &offset) noexcept
  {
    ScrollTo(ScrollPositionFromOffset(offset));
  }

  IntRect RenderLayerScrollableArea::ScrollCornerRect() const noexcept
  {
    return OverflowControlsRects().ScrollCorner;
  }

  bool RenderLayerScrollableArea::IsScrollCornerVisible() const noexcept
  {
    krys_debug_assert(_layer.Renderer().IsRenderBox());
    return !ScrollCornerRect().IsEmpty();
  }

  IntSize RenderLayerScrollableArea::VisibleSize() const noexcept
  {
    return _layer.VisibleSize();
  }

  IntSize RenderLayerScrollableArea::ContentsSize() const noexcept
  {
    return IntSize(ScrollWidth(), ScrollHeight());
  }

  IntSize RenderLayerScrollableArea::ReachableTotalContentsSize() const noexcept
  {
    IntSize contentsSize = this->ContentsSize();

    if (!HasScrollableHorizontalOverflow())
    {
      contentsSize.SetWidth(std::min(contentsSize.Width(), VisibleSize().Width()));
    }

    if (!HasScrollableVerticalOverflow())
    {
      contentsSize.SetHeight(std::min(contentsSize.Height(), VisibleSize().Height()));
    }

    return contentsSize;
  }

  void RenderLayerScrollableArea::AvailableContentSizeChanged(AvailableSizeChangeReason reason) noexcept
  {
    ScrollableArea::AvailableContentSizeChanged(reason);

    auto &renderer = _layer.Renderer();
    if (reason == AvailableSizeChangeReason::ScrollbarsChanged)
    {
      if (auto *renderBlock = DynamicDowncast<RenderBlock>(renderer))
      {
        renderBlock->SetShouldForceRelayoutChildren(true);
      }
      renderer.SetNeedsLayout();
    }
  }

  RenderLayer::OverflowControlRects RenderLayerScrollableArea::OverflowControlsRects() const noexcept
  {
    const auto &renderBox = Downcast<RenderBox>(_layer.Renderer());
    // Scrollbars sit inside the border box.
    auto overflowControlsPositioningRect = SnappedIntRect(renderBox.PaddingBoxRectIncludingScrollbar());

    auto horizontalScrollbarHeight = _hBar ? _hBar->Height() : 0;
    auto verticalScrollbarWidth = _vBar ? _vBar->Width() : 0;

    auto isNonOverlayScrollbar = [](const Scrollbar *scrollbar)
    {
      return scrollbar && !scrollbar->IsOverlayScrollbar();
    };

    bool haveNonOverlayHorizontalScrollbar = isNonOverlayScrollbar(_hBar.get());
    bool haveNonOverlayVerticalScrollbar = isNonOverlayScrollbar(_vBar.get());
    bool placeVerticalScrollbarOnTheLeft = ShouldPlaceVerticalScrollbarOnLeft();
    bool haveResizer =
      renderBox.Style().GetResize() != style::Resize::None && !renderBox.Style().GetPseudoElementType();
    bool scrollbarsAvoidCorner =
      ((haveNonOverlayHorizontalScrollbar && haveNonOverlayVerticalScrollbar)
       || (haveResizer && (haveNonOverlayHorizontalScrollbar || haveNonOverlayVerticalScrollbar)))
      && renderBox.Style().GetScrollbarWidth() != style::ScrollbarWidth::None;

    IntSize cornerSize;
    if (scrollbarsAvoidCorner)
    {
      // If only one scrollbar is present, the corner is square.
      cornerSize = IntSize {verticalScrollbarWidth ? verticalScrollbarWidth : horizontalScrollbarHeight,
                            horizontalScrollbarHeight ? horizontalScrollbarHeight : verticalScrollbarWidth};
    }

    RenderLayer::OverflowControlRects result;

    if (_hBar)
    {
      auto barRect = overflowControlsPositioningRect;
      barRect.ShiftYEdgeTo(barRect.MaxY() - horizontalScrollbarHeight);
      if (scrollbarsAvoidCorner)
      {
        if (placeVerticalScrollbarOnTheLeft)
        {
          barRect.ShiftXEdgeTo(barRect.X() + cornerSize.Width());
        }
        else
        {
          barRect.Contract(cornerSize.Width(), 0);
        }
      }

      result.HorizontalScrollbar = barRect;
    }

    if (_vBar)
    {
      auto barRect = overflowControlsPositioningRect;
      if (placeVerticalScrollbarOnTheLeft)
      {
        barRect.SetWidth(verticalScrollbarWidth);
      }
      else
      {
        barRect.ShiftXEdgeTo(barRect.MaxX() - verticalScrollbarWidth);
      }

      if (scrollbarsAvoidCorner)
      {
        barRect.Contract(0, cornerSize.Height());
      }

      result.VerticalScrollbar = barRect;
    }

    auto cornerRect = [&](IntSize cornerSize)
    {
      if (placeVerticalScrollbarOnTheLeft)
      {
        auto bottomLeftCorner = overflowControlsPositioningRect.MinXMaxYCorner();
        return IntRect {{bottomLeftCorner.X(), bottomLeftCorner.Y() - cornerSize.Height()}, cornerSize};
      }
      return IntRect {overflowControlsPositioningRect.MaxXMaxYCorner() - cornerSize, cornerSize};
    };

    if (scrollbarsAvoidCorner)
    {
      result.ScrollCorner = cornerRect(cornerSize);
    }

    if (haveResizer)
    {
      if (scrollbarsAvoidCorner)
      {
        result.Resizer = result.ScrollCorner;
      }
      else
      {
        auto scrollbarThickness = ScrollbarTheme::Theme().ScrollbarThickness();
        result.Resizer = cornerRect({scrollbarThickness, scrollbarThickness});
      }
    }

    return result;
  }

  IntSize RenderLayerScrollableArea::ScrollbarOffset(const Scrollbar &scrollbar) const noexcept
  {
    auto rects = OverflowControlsRects();

    if (&scrollbar == _vBar.get())
    {
      return ToIntSize(rects.VerticalScrollbar.Location());
    }

    if (&scrollbar == _hBar.get())
    {
      return ToIntSize(rects.HorizontalScrollbar.Location());
    }

    krys_debug_assert(false);
    return {};
  }

  bool RenderLayerScrollableArea::CanShowNonOverlayScrollbars() const noexcept
  {
    return CanHaveScrollbars()
           && !(_layer.GetRenderBox() && _layer.GetRenderBox()->CanUseOverlayScrollbars());
  }

  Ref<Scrollbar> RenderLayerScrollableArea::CreateScrollbar(ScrollbarOrientation orientation) noexcept
  {
    // FIXME: Custom scrollbars for the legacy scrollbar style, and the scrollbars of user agent shadow trees'
    // hosts (rendererForScrollbar), once there are any. Add the scrollbar to the frame view's widgets, once
    // it has some.
    auto widget = Scrollbar::CreateNativeScrollbar(*this, orientation, ScrollbarWidthStyle());
    DidAddScrollbar(widget.get(), orientation);
    return widget;
  }

  void RenderLayerScrollableArea::DestroyScrollbar(ScrollbarOrientation orientation) noexcept
  {
    RefPtr<Scrollbar> &scrollbar = orientation == ScrollbarOrientation::Horizontal ? _hBar : _vBar;
    if (!scrollbar)
    {
      return;
    }

    if (!scrollbar->IsCustomScrollbar())
    {
      WillRemoveScrollbar(*scrollbar, orientation);
    }

    scrollbar = nullptr;
  }

  void RenderLayerScrollableArea::SetHasHorizontalScrollbar(bool hasScrollbar) noexcept
  {
    if (hasScrollbar == HasHorizontalScrollbar())
    {
      return;
    }

    if (hasScrollbar)
    {
      _hBar = CreateScrollbar(ScrollbarOrientation::Horizontal);
      // FIXME: Rubber-banding elasticity, once there's rubber-banding.
    }
    else
    {
      DestroyScrollbar(ScrollbarOrientation::Horizontal);
    }

    // Destroying or creating one bar can cause our scrollbar corner to come and go. We need to update the
    // opposite scrollbar's style.
    if (_hBar)
    {
      _hBar->StyleChanged();
    }
    if (_vBar)
    {
      _vBar->StyleChanged();
    }
  }

  void RenderLayerScrollableArea::SetHasVerticalScrollbar(bool hasScrollbar) noexcept
  {
    if (hasScrollbar == HasVerticalScrollbar())
    {
      return;
    }

    if (hasScrollbar)
    {
      _vBar = CreateScrollbar(ScrollbarOrientation::Vertical);
      // FIXME: Rubber-banding elasticity, once there's rubber-banding.
    }
    else
    {
      DestroyScrollbar(ScrollbarOrientation::Vertical);
    }

    // Destroying or creating one bar can cause our scrollbar corner to come and go. We need to update the
    // opposite scrollbar's style.
    if (_hBar)
    {
      _hBar->StyleChanged();
    }
    if (_vBar)
    {
      _vBar->StyleChanged();
    }
  }

  ScrollableArea *RenderLayerScrollableArea::EnclosingScrollableArea() const noexcept
  {
    if (auto *scrollableLayer = _layer.EnclosingScrollableLayer(ExcludeSelf, CrossFrameBoundaries::No))
    {
      return scrollableLayer->GetScrollableArea();
    }

    // FIXME: The frame view, once it's a scrollable area.
    return nullptr;
  }

  int32 RenderLayerScrollableArea::VerticalScrollbarWidth(OverlayScrollbarSizeRelevancy relevancy,
                                                          bool isHorizontalWritingMode) const noexcept
  {
    if (_vBar && _vBar->IsOverlayScrollbar()
        && (relevancy == OverlayScrollbarSizeRelevancy::IgnoreOverlayScrollbarSize
            || !_vBar->ShouldParticipateInHitTesting()))
    {
      return 0;
    }

    if (!_vBar && isHorizontalWritingMode
        && !(_layer.Renderer().Style().GetScrollbarGutter().IsAuto()
             || ScrollbarTheme::Theme().UsesOverlayScrollbars()))
    {
      return ScrollbarTheme::Theme().ScrollbarThickness(ScrollbarWidthStyle());
    }

    if (!_vBar || !ShowsOverflowControls())
    {
      return 0;
    }

    return _vBar->Width();
  }

  int32 RenderLayerScrollableArea::HorizontalScrollbarHeight(OverlayScrollbarSizeRelevancy relevancy,
                                                             bool isHorizontalWritingMode) const noexcept
  {
    if (_hBar && _hBar->IsOverlayScrollbar()
        && (relevancy == OverlayScrollbarSizeRelevancy::IgnoreOverlayScrollbarSize
            || !_hBar->ShouldParticipateInHitTesting()))
    {
      return 0;
    }

    if (!_hBar && !isHorizontalWritingMode
        && !(_layer.Renderer().Style().GetScrollbarGutter().IsAuto()
             || ScrollbarTheme::Theme().UsesOverlayScrollbars()))
    {
      return ScrollbarTheme::Theme().ScrollbarThickness(ScrollbarWidthStyle());
    }

    if (!_hBar || !ShowsOverflowControls())
    {
      return 0;
    }

    return _hBar->Height();
  }

  ScrollbarWidth RenderLayerScrollableArea::ScrollbarWidthStyle() const noexcept
  {
    if (_layer.GetRenderBox())
    {
      return style::ToPlatform<style::ScrollbarWidth> {}(_layer.Renderer().Style().GetScrollbarWidth());
    }
    return ScrollbarWidth::Auto;
  }

  bool RenderLayerScrollableArea::HasOverflowControls() const noexcept
  {
    // FIXME: The custom scroll corner too, once there are any.
    return _hBar || _vBar || _layer.Renderer().Style().GetResize() != style::Resize::None;
  }

  bool RenderLayerScrollableArea::PositionOverflowControls(const IntSize &offsetFromRoot) noexcept
  {
    if (!_hBar && !_vBar && !_layer.CanResize())
    {
      return false;
    }

    if (!_layer.GetRenderBox())
    {
      return false;
    }

    auto rects = OverflowControlsRects();
    bool changed = false;

    if (_vBar)
    {
      rects.VerticalScrollbar.Move(offsetFromRoot);
      if (_vBar->FrameRect() != rects.VerticalScrollbar)
      {
        _vBar->SetFrameRect(rects.VerticalScrollbar);
        changed = true;
      }
    }

    if (_hBar)
    {
      rects.HorizontalScrollbar.Move(offsetFromRoot);
      if (_hBar->FrameRect() != rects.HorizontalScrollbar)
      {
        _hBar->SetFrameRect(rects.HorizontalScrollbar);
        changed = true;
      }
    }

    // FIXME: The custom scroll corner and resizer, once there are any.
    return changed;
  }

  void RenderLayerScrollableArea::ComputeScrollDimensions() noexcept
  {
    _scrollDimensionsDirty = false;

    auto *box = _layer.GetRenderBox();

    auto overflowRect = box->LayoutOverflowRect();

    _scrollWidth = RoundToInt(overflowRect.Width());
    _scrollHeight = RoundToInt(overflowRect.Height());

    ComputeScrollOrigin();
    ComputeHasCompositedScrollableOverflow(LayoutUpToDate::Yes);
  }

  void RenderLayerScrollableArea::ComputeScrollOrigin() noexcept
  {
    auto *box = _layer.GetRenderBox();

    auto overflowRect = box->LayoutOverflowRect();
    box->FlipForWritingMode(overflowRect);

    int32 scrollableLeftOverflow = RoundToInt(overflowRect.X() - box->BorderLeft());
    if (ShouldPlaceVerticalScrollbarOnLeft())
    {
      scrollableLeftOverflow -=
        VerticalScrollbarWidth(OverlayScrollbarSizeRelevancy::IgnoreOverlayScrollbarSize,
                               box->Style().GetWritingMode().IsHorizontal());
    }

    int32 scrollableTopOverflow = RoundToInt(overflowRect.Y() - box->BorderTop());
    SetScrollOrigin(IntPoint(-scrollableLeftOverflow, -scrollableTopOverflow));

    // Horizontal scrollbar offsets depend on the scroll origin when vertical
    // scrollbars are on the left.
    if (_hBar)
    {
      _hBar->OffsetDidChange();
    }
  }

  void
    RenderLayerScrollableArea::ComputeHasCompositedScrollableOverflow(LayoutUpToDate layoutUpToDate) noexcept
  {
    bool hasCompositedScrollableOverflow = _hasCompositedScrollableOverflow;

    switch (layoutUpToDate)
    {
      case LayoutUpToDate::No:
        // If layout is not up to date, the only thing we can reliably know is that style prevents overflow
        // scrolling.
        if (!CanUseCompositedScrolling())
        {
          hasCompositedScrollableOverflow = false;
        }
        break;
      case LayoutUpToDate::Yes:
        hasCompositedScrollableOverflow =
          CanUseCompositedScrolling()
          && (HasScrollableHorizontalOverflow() || HasScrollableVerticalOverflow());
        break;
    }

    if (hasCompositedScrollableOverflow == _hasCompositedScrollableOverflow)
    {
      return;
    }

    _layer.SetSelfAndDescendantsNeedPositionUpdate();

    // FIXME: Dirty the compositing of the enclosing stacking context's descendants and tell the compositor,
    // once there's compositing.

    _hasCompositedScrollableOverflow = hasCompositedScrollableOverflow;
  }

  bool RenderLayerScrollableArea::HasScrollableHorizontalOverflow() const noexcept
  {
    return HasHorizontalOverflow() && _layer.GetRenderBox()->ScrollsOverflowX();
  }

  bool RenderLayerScrollableArea::HasScrollableVerticalOverflow() const noexcept
  {
    return HasVerticalOverflow() && _layer.GetRenderBox()->ScrollsOverflowY();
  }

  bool RenderLayerScrollableArea::HasHorizontalOverflow() const noexcept
  {
    krys_debug_assert(!_scrollDimensionsDirty);

    return ScrollWidth() > RoundToInt(_layer.GetRenderBox()->ClientWidth());
  }

  bool RenderLayerScrollableArea::HasVerticalOverflow() const noexcept
  {
    krys_debug_assert(!_scrollDimensionsDirty);

    return ScrollHeight() > RoundToInt(_layer.GetRenderBox()->ClientHeight());
  }

  void RenderLayerScrollableArea::UpdateScrollbarPresenceAndState(Maybe<bool> hasHorizontalOverflow,
                                                                  Maybe<bool> hasVerticalOverflow) noexcept
  {
    auto *box = _layer.GetRenderBox();
    krys_debug_assert(box);

    enum class ScrollbarState : uint8
    {
      NoScrollbar,
      Enabled,
      Disabled
    };

    auto scrollbarForAxis = [&](ScrollbarOrientation orientation) -> RefPtr<Scrollbar> &
    {
      return orientation == ScrollbarOrientation::Horizontal ? _hBar : _vBar;
    };

    auto stateForScrollbar =
      [&](ScrollbarOrientation orientation, Maybe<bool> hasOverflow, ScrollbarState nonScrollableState)
    {
      if (hasOverflow)
      {
        return *hasOverflow ? ScrollbarState::Enabled : nonScrollableState;
      }

      // If we don't have information about overflow (because we haven't done layout yet), just return the
      // current state of the scrollbar.
      auto &existingScrollbar = scrollbarForAxis(orientation);
      return (existingScrollbar && existingScrollbar->Enabled()) ? ScrollbarState::Enabled
                                                                 : nonScrollableState;
    };

    auto stateForScrollbarOnAxis = [&](ScrollbarOrientation orientation, Maybe<bool> hasOverflow)
    {
      if (box->HasAlwaysPresentScrollbar(orientation))
      {
        return stateForScrollbar(orientation, hasOverflow, ScrollbarState::Disabled);
      }

      if (box->HasAutoScrollbar(orientation))
      {
        return stateForScrollbar(orientation, hasOverflow, ScrollbarState::NoScrollbar);
      }

      return ScrollbarState::NoScrollbar;
    };

    auto horizontalBarState =
      stateForScrollbarOnAxis(ScrollbarOrientation::Horizontal, hasHorizontalOverflow);
    SetHasHorizontalScrollbar(horizontalBarState != ScrollbarState::NoScrollbar);
    if (horizontalBarState != ScrollbarState::NoScrollbar)
    {
      _hBar->SetEnabled(horizontalBarState == ScrollbarState::Enabled);
    }

    auto verticalBarState = stateForScrollbarOnAxis(ScrollbarOrientation::Vertical, hasVerticalOverflow);
    SetHasVerticalScrollbar(verticalBarState != ScrollbarState::NoScrollbar);
    if (verticalBarState != ScrollbarState::NoScrollbar)
    {
      _vBar->SetEnabled(verticalBarState == ScrollbarState::Enabled);
    }
  }

  void RenderLayerScrollableArea::UpdateScrollbarsAfterStyleChange(const RenderStyle *oldStyle) noexcept
  {
    // Overflow is a box concept.
    RenderBox *box = _layer.GetRenderBox();
    if (!box)
    {
      return;
    }

    // FIXME: List boxes handle their scrollbars by themselves, once there are list boxes.

    bool hadVerticalScrollbar = HasVerticalScrollbar();
    UpdateScrollbarPresenceAndState();
    bool hasVerticalScrollbar = this->HasVerticalScrollbar();

    if (hadVerticalScrollbar != hasVerticalScrollbar
        || (hasVerticalScrollbar && oldStyle
            && oldStyle->ShouldPlaceVerticalScrollbarOnLeft()
                 != box->Style().ShouldPlaceVerticalScrollbarOnLeft()))
    {
      ComputeScrollOrigin();
    }

    // FIXME: Register with the frame view's scrollable areas (updateScrollableAreaSet), and the scrollbars'
    // dark appearance, once the frame view keeps them and there's painting.
  }

  void RenderLayerScrollableArea::UpdateScrollbarsAfterLayout() noexcept
  {
    RenderBox *box = _layer.GetRenderBox();
    krys_debug_assert(box);

    // FIXME: List boxes handle their scrollbars by themselves, once there are list boxes.

    bool hadHorizontalScrollbar = HasHorizontalScrollbar();
    bool hadVerticalScrollbar = HasVerticalScrollbar();

    bool hasHorizontalOverflow = this->HasHorizontalOverflow();
    bool hasVerticalOverflow = this->HasVerticalOverflow();

    UpdateScrollbarPresenceAndState(hasHorizontalOverflow, hasVerticalOverflow);

    // Scrollbars with auto behavior may need to lay out again if scrollbars got added or removed.
    bool autoHorizontalScrollBarChanged = box->HasAutoScrollbar(ScrollbarOrientation::Horizontal)
                                          && (hadHorizontalScrollbar != HasHorizontalScrollbar());
    bool autoVerticalScrollBarChanged = box->HasAutoScrollbar(ScrollbarOrientation::Vertical)
                                        && (hadVerticalScrollbar != HasVerticalScrollbar());

    if (autoHorizontalScrollBarChanged || autoVerticalScrollBarChanged)
    {
      if (autoVerticalScrollBarChanged && ShouldPlaceVerticalScrollbarOnLeft())
      {
        ComputeScrollOrigin();
      }

      _layer.UpdateSelfPaintingLayer();

      // FIXME: Repaint, once there's painting.
      auto &renderer = _layer.Renderer();

      if (renderer.Style().OverflowX() == Overflow::Auto || renderer.Style().OverflowY() == Overflow::Auto)
      {
        if (!_inOverflowRelayout)
        {
          SetForScope inOverflowRelayoutScope(_inOverflowRelayout, true);
          renderer.SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
          if (auto *block = DynamicDowncast<RenderBlock>(renderer))
          {
            // FIXME: Calling layoutBlock here is a bit of a layering violation.
            auto scope = LayoutScope {*block};
            block->ScrollbarsChanged(autoHorizontalScrollBarChanged, autoVerticalScrollBarChanged);
            block->LayoutBlock(RelayoutChildren::Yes);
          }
          else
          {
            renderer.Layout();
          }
        }
      }

      // FIXME: This does not belong here.
      auto *parent = renderer.Parent();
      if (auto *parentFlexibleBox = DynamicDowncast<RenderFlexibleBox>(parent);
          parentFlexibleBox && renderer.IsRenderBox())
      {
        parentFlexibleBox->ClearCachedMainSizeForFlexItem(*_layer.GetRenderBox());
      }
    }

    // Set up the range.
    if (_hBar)
    {
      _hBar->SetProportion(RoundToInt(box->ClientWidth()), _scrollWidth);
    }
    if (_vBar)
    {
      _vBar->SetProportion(RoundToInt(box->ClientHeight()), _scrollHeight);
    }

    UpdateScrollbarSteps();

    // FIXME: Register with the frame view's scrollable areas (updateScrollableAreaSet), once it keeps them.
  }

  void RenderLayerScrollableArea::UpdateScrollbarSteps() noexcept
  {
    if (!_hBar && !_vBar)
    {
      return;
    }

    auto *box = _layer.GetRenderBox();
    krys_debug_assert(box);

    // FIXME: Contract the bounds by the scroll padding (scrollPaddingForViewportRect), once there's scroll
    // snapping.
    LayoutRect paddedLayerBounds(0_lu, 0_lu, box->ClientWidth(), box->ClientHeight());

    // Set up the  page step/line step.
    if (_hBar)
    {
      int32 width = RoundToInt(paddedLayerBounds.Width());
      _hBar->SetSteps(Scrollbar::PixelsPerLineStep(width), Scrollbar::PageStep(width));
    }
    if (_vBar)
    {
      int32 height = RoundToInt(paddedLayerBounds.Height());
      _vBar->SetSteps(Scrollbar::PixelsPerLineStep(height), Scrollbar::PageStep(height));
    }
  }

  // This is called from layout code (before updateLayerPositions).
  void RenderLayerScrollableArea::UpdateScrollInfoAfterLayout() noexcept
  {
    RenderBox *box = _layer.GetRenderBox();
    if (!box)
    {
      return;
    }

    _scrollDimensionsDirty = true;
    auto originalScrollPosition = GetScrollPosition();

    ComputeScrollDimensions();
    _layer.UpdateSelfPaintingLayer();

    // FIXME: Update the snap offsets, once there's scroll snapping.

    // FIXME: Don't clamp during rubber-banding or user scrolls, once there are any.
    if (!box->IsHTMLMarquee())
    {
      // Layout may cause us to be at an invalid scroll position. In this case we need
      // to pull our scroll offsets back to the max (or push them up to the min).
      auto scrollOffset = this->GetScrollOffset();
      if (scrollOffset != MinimumScrollOffset())
      {
        auto clampedScrollOffset = ClampScrollOffset(scrollOffset);
        if (clampedScrollOffset != scrollOffset)
        {
          (void)ScrollToOffset(clampedScrollOffset);
        }
      }
    }

    UpdateScrollbarsAfterLayout();

    if (originalScrollPosition != GetScrollPosition())
    {
      (void)ScrollToPositionWithoutAnimation(IntPoint(GetScrollPosition()));
    }

    // FIXME: The compositing updates, resnapping after layout and the inspector, once there are any.
  }

  bool RenderLayerScrollableArea::OverflowControlsIntersectRect(const IntRect &localRect) const noexcept
  {
    auto rects = OverflowControlsRects();

    if (rects.HorizontalScrollbar.Intersects(localRect))
    {
      return true;
    }

    if (rects.VerticalScrollbar.Intersects(localRect))
    {
      return true;
    }

    if (rects.ScrollCorner.Intersects(localRect))
    {
      return true;
    }

    if (rects.Resizer.Intersects(localRect))
    {
      return true;
    }

    return false;
  }

  bool RenderLayerScrollableArea::ShowsOverflowControls() const noexcept
  {
    return true;
  }

  void RenderLayerScrollableArea::UpdateAllScrollbarRelatedStyle() noexcept
  {
    if (_hBar)
    {
      _hBar->StyleChanged();
    }
    if (_vBar)
    {
      _vBar->StyleChanged();
    }
    // FIXME: The custom scroll corner and resizer styles, once there are any.
  }

  namespace
  {
    KRYS_NODISCARD LayoutRect GetLocalExposeRect(const LayoutRect &absoluteRect, RenderBox *box,
                                                 int32 verticalScrollbarWidth,
                                                 const LayoutRect &layerBounds) noexcept
    {
      auto localExposeRect =
        LayoutRect(box->AbsoluteToLocalQuad(FloatQuad(FloatRect(absoluteRect))).BoundingBox());

      // localExposedRect is now the absolute rect in local coordinates, but relative to the
      // border edge. Make the rectangle relative to the scrollable area.
      localExposeRect.MoveBy(-LayoutPoint(box->BorderLeft(), box->BorderTop()));

      if (box->Style().ShouldPlaceVerticalScrollbarOnLeft())
      {
        // For `direction: rtl; writing-mode: horizontal-{tb,bt}` and `writing-mode: vertical-rl`
        // boxes, the scroll bar is on the left side. The visible rect starts from the right side
        // of the scroll bar. So the x of localExposeRect should start from the same position too.
        localExposeRect.MoveBy(LayoutPoint(LayoutUnit(-verticalScrollbarWidth), 0_lu));
      }

      // scroll-padding applies to the scroll container, but expand the rectangle that we want to expose in
      // order simulate padding the scroll container. This rectangle is passed up the tree of scrolling
      // elements to ensure that the padding on this scroll container is maintained.
      localExposeRect.Expand(box->ScrollPaddingForViewportRect(layerBounds));
      return localExposeRect;
    }
  }

  LayoutRect
    RenderLayerScrollableArea::ScrollRectToVisible(const LayoutRect &absoluteRect,
                                                   const ScrollRectToVisibleOptions &options) noexcept
  {
    auto *box = _layer.GetRenderBox();
    krys_debug_assert(box);

    auto layerBounds = LayoutRect(0_lu, 0_lu, box->ClientWidth(), box->ClientHeight());

    auto localExposeRect = GetLocalExposeRect(absoluteRect, box, VerticalScrollbarWidth(), layerBounds);
    Maybe<LayoutRect> localVisiblityRect;
    if (options.VisibilityCheckRect)
    {
      localVisiblityRect =
        GetLocalExposeRect(*options.VisibilityCheckRect, box, VerticalScrollbarWidth(), layerBounds);
    }

    auto revealRect = GetRectToExposeForScrollIntoView(layerBounds, localExposeRect, options.AlignX,
                                                       options.AlignY, localVisiblityRect);
    // FIXME: Smooth scrolling (useSmoothScrolling), once there are scroll animations.
    auto scrollPositionOptions = ScrollPositionChangeOptions::CreateProgrammatic();
    if (auto result =
          UpdateScrollPositionForScrollIntoView(scrollPositionOptions, revealRect, localExposeRect))
    {
      return *result;
    }
    return absoluteRect;
  }

  Maybe<LayoutRect> RenderLayerScrollableArea::UpdateScrollPositionForScrollIntoView(
    const ScrollPositionChangeOptions &options, const LayoutRect &revealRect,
    const LayoutRect &localExposeRect) noexcept
  {
    auto *box = _layer.GetRenderBox();
    krys_debug_assert(box);

    auto clampedScrollOffset =
      ClampScrollOffset(GetScrollOffset() + ToIntSize(RoundedIntRect(revealRect).Location()));
    if (clampedScrollOffset == GetScrollOffset()
        && GetScrollAnimationStatus() == ScrollAnimationStatus::NotAnimating)
    {
      return null;
    }

    auto oldScrollOffset = GetScrollOffset();
    auto realScrollOffset = ScrollToOffset(clampedScrollOffset, options);

    auto scrollOffsetDifference = realScrollOffset - oldScrollOffset;
    auto localExposeRectScrolled = localExposeRect;
    localExposeRectScrolled.Move(-LayoutSize(scrollOffsetDifference));
    return LayoutRect(box
                        ->LocalToAbsoluteQuad(FloatQuad(FloatRect(localExposeRectScrolled)),
                                              MapCoordinatesMode::UseTransforms)
                        .BoundingBox());
  }
}
