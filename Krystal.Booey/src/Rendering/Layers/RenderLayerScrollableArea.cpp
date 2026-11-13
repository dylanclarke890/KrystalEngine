/*
 * Copyright (C) 2006-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2019 Adobe. All rights reserved.
 * Copyright (C) 2014 Google. All rights reserved.
 * Copyright (C) 2020 Igalia S.L.
 *
 * Portions are Copyright (C) 1998 Netscape Communications Corporation.
 *
 * Other contributors:
 *   Robert O'Callahan <roc+@cs.cmu.edu>
 *   David Baron <dbaron@fas.harvard.edu>
 *   Christian Biesinger <cbiesinger@web.de>
 *   Randall Jesup <rjesup@wgate.com>
 *   Roland Mainz <roland.mainz@informatik.med.uni-giessen.de>
 *   Josh Soref <timeless@mac.com>
 *   Boris Zbarsky <bzbarsky@mit.edu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Alternatively, the contents of this file may be used under the terms
 * of either the Mozilla Public License Version 1.1, found at
 * http://www.mozilla.org/MPL/ (the "MPL") or the GNU General Public
 * License Version 2.0, found at http://www.fsf.org/copyleft/gpl.html
 * (the "GPL"), in which case the provisions of the MPL or the GPL are
 * applicable instead of those above.  If you wish to allow use of your
 * version of this file only under the terms of one of those two
 * licenses (the MPL or the GPL) and not to allow others to use your
 * version of this file under the LGPL, indicate your decision by
 * deletingthe provisions above and replace them with the notice and
 * other provisions required by the MPL or the GPL, as the case may be.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under any of the LGPL, the MPL or the GPL.
 */

#include "Krystal.Booey/Rendering/Layers/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatQuad.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Page/ScrollBehavior.hpp"
#include "Krystal.Booey/Page/Scrolling/ScrollSnapOffsetsInfo.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollAlignment.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollAnimator.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollbarTheme.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayerBacking.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayerCompositor.hpp"
#include "Krystal.Booey/Rendering/LayoutScope.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/AnchorPositionEvaluator.hpp"
#include "Krystal.Booey/Style/Values/Scrollbars/ScrollbarWidth.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  RenderLayerScrollableArea::RenderLayerScrollableArea(RenderLayer &layer) noexcept : _layer(layer)
  {
    // FIXME: The scroll anchoring controller (cssScrollAnchoringEnabled), once there's scroll anchoring.
  }

  RenderLayerScrollableArea::~RenderLayerScrollableArea() noexcept = default;

  void RenderLayerScrollableArea::Clear() noexcept
  {
    if (_registeredScrollableArea)
    {
      (void)_layer.Renderer().View().FrameView().RemoveScrollableArea(*this);
      _registeredScrollableArea = false;
    }
    (void)StopAnimatedScroll();

    auto &renderer = _layer.Renderer();
    if (auto *element = renderer.GetElement())
    {
      element->SetSavedLayerScrollPosition(_scrollPosition);
    }

    DestroyScrollbar(ScrollbarOrientation::Horizontal);
    DestroyScrollbar(ScrollbarOrientation::Vertical);
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

  void RenderLayerScrollableArea::PanScrollFromPoint(const IntPoint &sourcePoint,
                                                     const IntPoint &mousePosition) noexcept
  {
    // The speed grows faster than the distance, as in WebKit.
    auto adjustedDelta = [](int32 delta)
    {
      if (std::abs(delta) <= LocalFrameView::NoPanScrollRadius)
      {
        return 0;
      }
      constexpr int32 speedReducer = 12;
      int32 adjusted = delta / speedReducer;
      return std::abs(adjusted) > 1
               ? static_cast<int32>(adjusted * std::sqrt(static_cast<double>(std::abs(adjusted))))
               : adjusted;
    };

    ScrollByRecursively(IntSize(adjustedDelta(mousePosition.X() - sourcePoint.X()),
                                adjustedDelta(mousePosition.Y() - sourcePoint.Y())));
  }

  void RenderLayerScrollableArea::ScrollByRecursively(const IntSize &delta,
                                                      ScrollableArea **scrolledArea) noexcept
  {
    if (delta.IsZero())
    {
      return;
    }

    auto &renderer = _layer.Renderer();
    if (renderer.HasNonVisibleOverflow())
    {
      ScrollOffset newScrollOffset = GetScrollOffset() + delta;
      auto options = ScrollPositionChangeOptions::CreateProgrammatic();
      options.SnapPointSelectionMethod = ScrollSnapPointSelectionMethod::None;
      (void)ScrollToOffset(newScrollOffset, options);
      if (scrolledArea)
      {
        *scrolledArea = this;
      }

      // If this layer can't do the scroll we ask the next layer up that can scroll to try
      IntSize remainingScrollOffset = newScrollOffset - GetScrollOffset();
      if (!remainingScrollOffset.IsZero() && renderer.Parent())
      {
        // FIXME: This skips scrollable frames.
        if (auto *enclosingScrollableLayer =
              _layer.EnclosingScrollableLayer(IncludeSelfOrNot::ExcludeSelf, CrossFrameBoundaries::Yes))
        {
          if (auto *scrollableArea = enclosingScrollableLayer->GetScrollableArea())
          {
            scrollableArea->ScrollByRecursively(remainingScrollOffset, scrolledArea);
          }
        }

        renderer.View().FrameView().Frame().GetEventHandler().UpdateAutoscrollRenderer();
      }
    }
    else
    {
      // If we are here, we were called on a renderer that can be programmatically scrolled, but doesn't
      // have an overflow clip. Which means that it is a document node that can be scrolled.
      // Bit by bit, it doesn't snap as it goes (it would never leave a snap position): it snaps when it ends.
      auto &frameView = renderer.View().FrameView();
      auto options = ScrollPositionChangeOptions::CreateProgrammatic();
      options.SnapPointSelectionMethod = ScrollSnapPointSelectionMethod::None;
      frameView.SetScrollPosition(frameView.GetScrollPosition() + delta, options);
      if (scrolledArea)
      {
        *scrolledArea = &renderer.View().FrameView();
      }

      // FIXME: If we didn't scroll the whole way, do we want to try looking at the frames ownerElement?
      // https://bugs.webkit.org/show_bug.cgi?id=28237
    }
  }

  ScrollOffset RenderLayerScrollableArea::ScrollToOffset(const ScrollOffset &scrollOffset,
                                                         const ScrollPositionChangeOptions &options) noexcept
  {
    // A smooth scroll is retargeted by another, and stopped by any other.
    bool animated = options.Animated == ScrollIsAnimated::Yes;
    bool wasAnimating = GetScrollAnimationStatus() == ScrollAnimationStatus::Animating;
    bool stoppedAnimation = !animated && StopAnimatedScroll();
    ScrollOffset clampedScrollOffset =
      options.Clamping == ScrollClamping::Clamped ? ClampScrollOffset(scrollOffset) : scrollOffset;
    if (clampedScrollOffset == this->GetScrollOffset() && !(animated && wasAnimating))
    {
      if (stoppedAnimation)
      {
        ScrollDidEnd();
      }
      return clampedScrollOffset;
    }

    auto previousScrollType = CurrentScrollType();
    SetCurrentScrollType(options.Type);

    ScrollOffset snappedOffset =
      ScrollOffsetAdjustedForSnapping(clampedScrollOffset, options.SnapPointSelectionMethod);
    auto snappedPosition = ScrollPositionFromOffset(snappedOffset);
    if (animated)
    {
      ScrollToPositionWithAnimation(snappedPosition, options);
    }
    else if (!RequestScrollToPosition(snappedPosition, options))
    {
      (void)ScrollToPositionWithoutAnimation(snappedPosition, options.Clamping);
    }

    SetCurrentScrollType(previousScrollType);
    return snappedOffset;
  }

  void RenderLayerScrollableArea::RegisterForAnimatedScroll() noexcept
  {
    _layer.Renderer().View().FrameView().AddScrollableAreaForAnimatedScroll(*this);
  }

  void RenderLayerScrollableArea::UpdateSnapOffsets() noexcept
  {
    auto *box = _layer.GetRenderBox();
    if (!box || !box->GetElement())
    {
      return;
    }
    UpdateSnapOffsetsForScrollableArea(*this, *box, box->PaddingBoxRect());
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

      // FIXME: Not while a marquee updates its position (m_updatingMarqueePosition), once there are marquees.
      if (UsesCompositedScrolling())
      {
        _layer.SetNeedsCompositingGeometryUpdate();

        // Scroll position can affect the location of a composited descendant (which may be a sibling in
        // z-order), so trigger a descendant walk from the stacking context.
        if (auto *paintParent = _layer.StackingContext())
        {
          paintParent->SetDescendantsNeedUpdateBackingAndHierarchyTraversal();
        }
      }

      UpdateCompositingLayersAfterScroll();

      // FIXME: Rendering-dependent regions and debug overlays, once there are any.
    }

    const auto *repaintContainer = renderer.ContainerForRepaint().Renderer;
    // The caret rect needs to be invalidated after scrolling.
    renderer.GetDocument().Selection().SetCaretRectNeedsUpdate();

    auto cachedRect = _layer.CachedClippedOverflowRect();
    auto rectForRepaint = cachedRect ? *cachedRect : renderer.ClippedOverflowRectForRepaint(repaintContainer);

    FloatQuad quadForFakeMouseMoveEvent = FloatQuad(FloatRect(rectForRepaint));
    if (repaintContainer)
    {
      quadForFakeMouseMoveEvent = repaintContainer->LocalToAbsoluteQuad(quadForFakeMouseMoveEvent);
    }
    view.FrameView().Frame().GetEventHandler().DispatchFakeMouseMoveEventSoonInQuad(
      quadForFakeMouseMoveEvent);

    bool requiresRepaint = true;
    if (UsesCompositedScrolling())
    {
      _layer.SetNeedsCompositingGeometryUpdate();
      _layer.SetDescendantsNeedUpdateBackingAndHierarchyTraversal();
      requiresRepaint = _layer.Backing()->NeedsRepaintOnCompositedScroll();
    }

    // Just schedule a full repaint of our object.
    if (requiresRepaint)
    {
      renderer.RepaintUsingContainer(repaintContainer, rectForRepaint);

      auto isScrolledBy = [](const RenderElement &renderer, const RenderLayer &scrollableLayer)
      {
        auto *layer = renderer.EnclosingLayer();
        return layer && layer->AncestorLayerIsInContainingBlockChain(scrollableLayer);
      };

      // We also have to repaint any descendant composited layers that have fixed backgrounds.
      for (const auto *slowRepaintObject : view.FrameView().SlowRepaintObjects())
      {
        if (isScrolledBy(*slowRepaintObject, _layer))
        {
          slowRepaintObject->Repaint();
        }
      }
    }

    // Schedule the scroll and scroll-related DOM events.
    if (auto *element = renderer.GetElement())
    {
      SetIsAwaitingScrollend(true);
      element->NodeDocument().AddPendingScrollEventTarget(*element, dom::ScrollEventType::Scroll);
    }

    view.FrameView().ViewportContentsChanged();
  }

  GraphicsLayer *RenderLayerScrollableArea::LayerForHorizontalScrollbar() const noexcept
  {
    return _layer.Backing() ? _layer.Backing()->LayerForHorizontalScrollbar() : nullptr;
  }

  GraphicsLayer *RenderLayerScrollableArea::LayerForVerticalScrollbar() const noexcept
  {
    return _layer.Backing() ? _layer.Backing()->LayerForVerticalScrollbar() : nullptr;
  }

  GraphicsLayer *RenderLayerScrollableArea::LayerForScrollCorner() const noexcept
  {
    return _layer.Backing() ? _layer.Backing()->LayerForScrollCorner() : nullptr;
  }

  void RenderLayerScrollableArea::UpdateCompositingLayersAfterScroll() noexcept
  {
    if (_layer.Compositor().HasContentCompositingLayers())
    {
      // Our stacking container is guaranteed to contain all of our descendants that may need repositioning,
      // so update compositing layers from there.
      if (RenderLayer *compositingAncestor = _layer.StackingContext()->EnclosingCompositingLayer())
      {
        if (UsesCompositedScrolling())
        {
          _layer.Compositor().UpdateCompositingLayers(CompositingUpdateType::OnCompositedScroll,
                                                      compositingAncestor);
        }
        else
        {
          // FIXME: would be nice to only dirty layers whose positions were affected by scrolling.
          compositingAncestor->SetDescendantsNeedUpdateBackingAndHierarchyTraversal();
          _layer.Compositor().UpdateCompositingLayers(CompositingUpdateType::OnScroll, compositingAncestor);
        }
      }
    }
  }

  void RenderLayerScrollableArea::InvalidateScrollbar(Scrollbar &scrollbar, const IntRect &rect) noexcept
  {
    if (!ShowsOverflowControls())
    {
      return;
    }

    if (&scrollbar == _vBar.get())
    {
      if (GraphicsLayer *layer = LayerForVerticalScrollbar())
      {
        layer->SetNeedsDisplayInRect(FloatRect(rect));
        return;
      }
    }
    else
    {
      if (GraphicsLayer *layer = LayerForHorizontalScrollbar())
      {
        layer->SetNeedsDisplayInRect(FloatRect(rect));
        return;
      }
    }

    auto scrollRect = rect;
    RenderBox *box = _layer.GetRenderBox();
    krys_debug_assert(box);
    // If we are not yet inserted into the tree, there is no need to repaint.
    if (!box->Parent())
    {
      return;
    }

    auto rects = OverflowControlsRects();

    if (&scrollbar == _vBar.get())
    {
      scrollRect.MoveBy(rects.VerticalScrollbar.Location());
    }
    else
    {
      scrollRect.MoveBy(rects.HorizontalScrollbar.Location());
    }

    LayoutRect repaintRect = LayoutRect(scrollRect);
    box->FlipForWritingMode(repaintRect);
    box->RepaintRectangle(repaintRect);
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

  void RenderLayerScrollableArea::UpdateAnchorPositionedAfterScroll() noexcept
  {
    style::AnchorPositionEvaluator::UpdateScrollAdjustments(_layer.Renderer().View());
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

  bool RenderLayerScrollableArea::UsesCompositedScrolling() const noexcept
  {
    return HasCompositedScrollableOverflow() && _layer.IsComposited();
  }

  bool RenderLayerScrollableArea::CanUseCompositedScrolling() const noexcept
  {
    // Overflow scrolls on layers whenever the page composites, as asyncOverflowScrollingEnabled has it for
    // the compositing drawing areas.
    auto &renderer = _layer.Renderer();
    bool isVisible = renderer.Style().UsedVisibility() == Visibility::Visible;
    if (_layer.Compositor().HasAcceleratedCompositing())
    {
      return isVisible && ScrollsOverflow();
    }

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
    // FIXME: The scrollbars of user agent shadow trees' hosts (rendererForScrollbar), once there are any.
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

    WillRemoveScrollbar(*scrollbar, orientation);

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

  bool RenderLayerScrollableArea::UseDarkAppearanceForScrollbars() const noexcept
  {
    return _layer.Renderer().UseDarkAppearance();
  }

  Maybe<ScrollbarColor> RenderLayerScrollableArea::ScrollbarColorStyle() const noexcept
  {
    if (_layer.GetRenderBox())
    {
      return _layer.Renderer().Style().UsedScrollbarColor();
    }
    return null;
  }

  bool RenderLayerScrollableArea::HasOverflowControls() const noexcept
  {
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

    // Whether this layer does composited scrolling affects the configuration of descendant sticky layers. We
    // have to dirty from the enclosing stacking context because overflow scroll doesn't create stacking
    // context so those containing block descendants may not be paint-order descendants, and the compositing
    // dirty bits on RenderLayer act in paint order.
    if (auto *paintParent = _layer.StackingContext())
    {
      paintParent->SetDescendantsNeedUpdateBackingAndHierarchyTraversal();
    }

    _hasCompositedScrollableOverflow = hasCompositedScrollableOverflow;
    if (_hasCompositedScrollableOverflow)
    {
      _layer.Compositor().LayerGainedCompositedScrollableOverflow(_layer);
    }
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

    // List box parts handle the scrollbars by themselves so we have nothing to do.
    if (box->Style().UsedAppearance() == StyleAppearance::Listbox)
    {
      return;
    }

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

    // Before the box lays out, the overflow is known after it.
    if (!_scrollDimensionsDirty)
    {
      UpdateScrollableAreaSet(HasScrollableHorizontalOverflow() || HasScrollableVerticalOverflow());
    }
  }

  void RenderLayerScrollableArea::UpdateScrollbarsAfterLayout() noexcept
  {
    RenderBox *box = _layer.GetRenderBox();
    krys_debug_assert(box);

    // List box parts handle the scrollbars by themselves so we have nothing to do.
    if (box->Style().UsedAppearance() == StyleAppearance::Listbox)
    {
      return;
    }

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

      auto &renderer = _layer.Renderer();
      renderer.Repaint();

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

    UpdateScrollableAreaSet(HasScrollableHorizontalOverflow() || HasScrollableVerticalOverflow());
  }

  void RenderLayerScrollableArea::UpdateScrollableAreaSet(bool hasOverflow) noexcept
  {
    // WebKit keeps only those that can scroll and are visible to hit testing; scrollbars that can't scroll,
    // and hidden ones, are kept too here, so the theme's changes reach them.
    // FIXME: Touch event listeners for unaccelerated overflow scrolling
    // (registerAsTouchEventListenerForScrolling).
    auto &frameView = _layer.Renderer().View().FrameView();
    bool needsToBeRegistered =
      hasOverflow || _hBar || _vBar || GetScrollAnimationStatus() == ScrollAnimationStatus::Animating;
    if (needsToBeRegistered)
    {
      if (!_registeredScrollableArea)
      {
        (void)frameView.AddScrollableArea(*this);
        _registeredScrollableArea = true;
      }
    }
    else if (_registeredScrollableArea)
    {
      (void)frameView.RemoveScrollableArea(*this);
      _registeredScrollableArea = false;
    }
  }

  void RenderLayerScrollableArea::UpdateScrollbarSteps() noexcept
  {
    if (!_hBar && !_vBar)
    {
      return;
    }

    auto *box = _layer.GetRenderBox();
    krys_debug_assert(box);

    LayoutRect paddedLayerBounds(0_lu, 0_lu, box->ClientWidth(), box->ClientHeight());
    paddedLayerBounds.Contract(box->ScrollPaddingForViewportRect(paddedLayerBounds));

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

    UpdateSnapOffsets();

    // FIXME: Don't clamp during user scrolls, once there are any.
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

    ResnapAfterLayout();

    // FIXME: The compositing updates and the inspector, once there are any.
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
    auto scrollPositionOptions = ScrollPositionChangeOptions::CreateProgrammatic();
    auto *frame = box->GetDocument().Frame();
    if (frame && frame->GetEventHandler().IsAutoscrolling())
    {
      // An autoscroll moves bit by bit: it snaps when it ends.
      scrollPositionOptions.SnapPointSelectionMethod = ScrollSnapPointSelectionMethod::None;
    }
    else if (frame && box->GetElement() && UseSmoothScrolling(options.Behavior, box->GetElement()))
    {
      scrollPositionOptions.Animated = ScrollIsAnimated::Yes;
    }
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

  void RenderLayerScrollableArea::PaintOverflowControls(GraphicsContext &context,
                                                        EnumSet<PaintBehavior> paintBehavior,
                                                        const IntPoint &paintOffset,
                                                        const IntRect &damageRect,
                                                        bool paintingOverlayControls) noexcept
  {
    // Don't do anything if we have no overflow.
    auto &renderer = _layer.Renderer();
    if (!renderer.HasNonVisibleOverflow())
    {
      return;
    }

    if (!ShowsOverflowControls())
    {
      return;
    }

    // Overlay scrollbars paint in a second pass through the layer tree so that they will paint on top of
    // everything else. If this is the normal painting pass, paintingOverlayControls will be false, and we
    // should just tell the root layer that there are overlay scrollbars that need to be painted. That will
    // cause the second pass through the layer tree to run, and we'll paint the scrollbars then. In the
    // meantime, cache the paint offset so that the second pass doesn't need to re-enter the render tree to
    // get it right.
    if (HasOverlayScrollbars() && !paintingOverlayControls)
    {
      _cachedOverlayScrollbarOffset = paintOffset;

      // It's not necessary to do the second pass if the scrollbars paint into layers.
      if ((_hBar && LayerForHorizontalScrollbar()) || (_vBar && LayerForVerticalScrollbar()))
      {
        return;
      }
      IntRect localDamageRect = damageRect;
      localDamageRect.MoveBy(-paintOffset);
      if (!OverflowControlsIntersectRect(localDamageRect))
      {
        return;
      }

      auto *paintingRoot = _layer.EnclosingCompositingLayer();
      if (!paintingRoot)
      {
        paintingRoot = renderer.View().Layer();
      }

      if (auto *scrollableArea = paintingRoot->GetScrollableArea())
      {
        scrollableArea->SetContainsDirtyOverlayScrollbars(true);
      }
      return;
    }

    // Classic scrollbars painted in the first pass.
    if (paintingOverlayControls && !HasOverlayScrollbars())
    {
      return;
    }

    IntPoint adjustedPaintOffset = paintingOverlayControls ? _cachedOverlayScrollbarOffset : paintOffset;

    // Move the scrollbar widgets if necessary. We normally move and resize widgets during layout, but
    // sometimes widgets can move without layout occurring (most notably when you scroll a document that
    // contains fixed positioned elements).
    (void)PositionOverflowControls(ToIntSize(adjustedPaintOffset));

    // Now that we're sure the scrollbars are in the right place, paint them.
    if (RefPtr<Scrollbar> hBar = _hBar;
        hBar
        && (!LayerForHorizontalScrollbar()
            || paintBehavior.Contains(PaintBehavior::FlattenCompositingLayers)))
    {
      hBar->Paint(context, damageRect);
    }
    if (RefPtr<Scrollbar> vBar = _vBar;
        vBar
        && (!LayerForVerticalScrollbar() || paintBehavior.Contains(PaintBehavior::FlattenCompositingLayers)))
    {
      vBar->Paint(context, damageRect);
    }

    if (LayerForScrollCorner() && !paintBehavior.Contains(PaintBehavior::FlattenCompositingLayers))
    {
      return;
    }

    // We fill our scroll corner with white if we have a scrollbar that doesn't run all the way up to the edge
    // of the box.
    PaintScrollCorner(context, adjustedPaintOffset, damageRect);

    // Paint our resizer last, since it sits on top of the scroll corner.
    PaintResizer(context, LayoutPoint(adjustedPaintOffset), LayoutRect(damageRect));
  }

  void RenderLayerScrollableArea::PaintResizer(GraphicsContext &context, const LayoutPoint &paintOffset,
                                               const LayoutRect &damageRect) noexcept
  {
    auto &renderer = _layer.Renderer();
    if (renderer.Style().GetResize() == style::Resize::None)
    {
      return;
    }

    auto rects = OverflowControlsRects();

    LayoutRect resizerAbsRect = LayoutRect(rects.Resizer);
    resizerAbsRect.MoveBy(paintOffset);
    if (!resizerAbsRect.Intersects(damageRect))
    {
      return;
    }

    // FIXME: Invalidating control tints.

    RenderTheme::Singleton().PaintPlatformResizer(renderer, context, resizerAbsRect);

    // Draw a frame around the resizer if there are any scrollbars present.
    if (!HasOverlayScrollbars() && (_vBar || _hBar)
        && renderer.Style().GetScrollbarWidth() != style::ScrollbarWidth::None)
    {
      RenderTheme::Singleton().PaintPlatformResizerFrame(renderer, context, resizerAbsRect);
    }
  }

  bool RenderLayerScrollableArea::HitTestResizerInFragments(const LayerFragments &layerFragments,
                                                            const HitTestLocation &hitTestLocation,
                                                            LayoutPoint &pointInFragment) const noexcept
  {
    if (layerFragments.empty())
    {
      return false;
    }

    auto &renderer = _layer.Renderer();
    if (!renderer.VisibleToHitTesting())
    {
      return false;
    }

    auto borderBoxRect = SnappedIntRect(Downcast<RenderBox>(renderer).BorderBoxRect());
    auto rects = OverflowControlsRects();

    auto cornerRectInFragment = [&](const IntRect &fragmentBounds, const IntRect &resizerRect)
    {
      if (ShouldPlaceVerticalScrollbarOnLeft())
      {
        IntSize offsetFromBottomLeft = borderBoxRect.MinXMaxYCorner() - resizerRect.MinXMaxYCorner();
        return IntRect {fragmentBounds.MinXMaxYCorner() - offsetFromBottomLeft
                          - IntSize {0, resizerRect.Height()},
                        resizerRect.Size()};
      }
      IntSize offsetFromBottomRight = borderBoxRect.MaxXMaxYCorner() - resizerRect.MaxXMaxYCorner();
      return IntRect {fragmentBounds.MaxXMaxYCorner() - offsetFromBottomRight - resizerRect.Size(),
                      resizerRect.Size()};
    };

    for (auto i = static_cast<int64>(layerFragments.size()) - 1; i >= 0; --i)
    {
      const LayerFragment &fragment = layerFragments[static_cast<size_t>(i)];
      auto resizerRectInFragment =
        cornerRectInFragment(SnappedIntRect(fragment.LayerBounds()), rects.Resizer);
      if (fragment.DirtyBackgroundRect().Intersects(hitTestLocation)
          && resizerRectInFragment.Contains(hitTestLocation.RoundedPoint()))
      {
        pointInFragment = ToLayoutPoint(hitTestLocation.Point() - fragment.LayerBounds().Location());
        return true;
      }
    }

    return false;
  }

  void RenderLayerScrollableArea::PaintScrollCorner(GraphicsContext &context, const IntPoint &paintOffset,
                                                    const IntRect &damageRect) noexcept
  {
    IntRect absRect = ScrollCornerRect();
    absRect.MoveBy(paintOffset);
    if (!absRect.Intersects(damageRect))
    {
      return;
    }

    // We don't want to paint a corner if we have overlay scrollbars, since we need to see what is behind it.
    if (!HasOverlayScrollbars())
    {
      ScrollbarTheme::Theme().PaintScrollCorner(*this, context, absRect);
    }
  }

  void RenderLayerScrollableArea::PaintOverlayScrollbars(GraphicsContext &context,
                                                         const LayoutRect &damageRect,
                                                         EnumSet<PaintBehavior> paintBehavior,
                                                         RenderObject *subtreePaintRoot) noexcept
  {
    if (!_containsDirtyOverlayScrollbars)
    {
      return;
    }

    RenderLayer::LayerPaintingInfo paintingInfo(&_layer, LayoutRect(EnclosingIntRect(damageRect)),
                                                paintBehavior, LayoutSize(), subtreePaintRoot);
    _layer.PaintLayer(context, paintingInfo, RenderLayer::PaintLayerFlag::PaintingOverlayScrollbars);

    _containsDirtyOverlayScrollbars = false;
  }

  bool RenderLayerScrollableArea::HitTestOverflowControls(HitTestResult &result,
                                                          const IntPoint &localPoint) noexcept
  {
    if (!_hBar && !_vBar && !_layer.CanResize())
    {
      return false;
    }

    auto rects = OverflowControlsRects();

    auto &renderer = _layer.Renderer();
    if (renderer.Style().GetResize() != style::Resize::None)
    {
      if (rects.Resizer.Contains(localPoint))
      {
        return true;
      }
    }

    // FIXME: We should hit test the scroll corner and pass it back through the result.
    if (RefPtr<Scrollbar> vBar = _vBar; vBar && vBar->ShouldParticipateInHitTesting())
    {
      if (rects.VerticalScrollbar.Contains(localPoint))
      {
        result.SetScrollbar(krys::move(vBar));
        return true;
      }
    }

    if (RefPtr<Scrollbar> hBar = _hBar; hBar && hBar->ShouldParticipateInHitTesting())
    {
      if (rects.HorizontalScrollbar.Contains(localPoint))
      {
        result.SetScrollbar(krys::move(hBar));
        return true;
      }
    }

    return false;
  }

  IntPoint RenderLayerScrollableArea::ConvertFromContainingViewToScrollbar(
    const Scrollbar &scrollbar, const IntPoint &parentPoint) const noexcept
  {
    auto &renderer = _layer.Renderer();
    // LocalFrameView::convertFromContainingViewToRenderer: from the view's coordinates into page ("absolute")
    // coordinates, then the renderer's.
    auto &view = renderer.View().FrameView();
    IntPoint absolutePoint = parentPoint + IntSize(view.ScrollX(), view.ScrollY());
    IntPoint point = RoundedIntPoint(
      renderer.AbsoluteToLocal(FloatPoint(absolutePoint), {MapCoordinatesMode::UseTransforms}));
    point.Move(-ScrollbarOffset(scrollbar));
    return point;
  }

  OverscrollBehavior RenderLayerScrollableArea::HorizontalOverscrollBehavior() const noexcept
  {
    if (_layer.GetRenderBox())
    {
      return _layer.Renderer().Style().OverscrollBehaviorX();
    }
    return OverscrollBehavior::Auto;
  }

  OverscrollBehavior RenderLayerScrollableArea::VerticalOverscrollBehavior() const noexcept
  {
    if (_layer.GetRenderBox())
    {
      return _layer.Renderer().Style().OverscrollBehaviorY();
    }
    return OverscrollBehavior::Auto;
  }

  bool RenderLayerScrollableArea::IsScrollableOrRubberbandable() const noexcept
  {
    auto *box = _layer.GetRenderBox();
    return box && box->CanBeScrolledAndHasScrollableArea();
  }

  bool RenderLayerScrollableArea::ScrollAnimatorEnabled() const noexcept
  {
    return _layer.Renderer().GetDocument().GetSettings().ScrollAnimatorEnabled;
  }
}
