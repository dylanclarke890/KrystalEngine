/*
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999 Lars Knoll <knoll@kde.org>
 *                     1999 Antti Koivisto <koivisto@kde.org>
 *                     2000 Dirk Mueller <mueller@kde.org>
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
 *           (C) 2006 Graham Dennis (graham.dennis@gmail.com)
 *           (C) 2006 Alexey Proskuryakov (ap@nypop.com)
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/DOM/Algorithms/FindRevealAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Highlights/HighlightRegistry.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Booey/Editing/Selection/TemporarySelectionChange.hpp"
#include "Krystal.Booey/Editing/Selection/VisiblePosition.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/Layers/GraphicsLayer.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLHtmlElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/HTML/Images/ImageDocument.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/ScrollBehavior.hpp"
#include "Krystal.Booey/Page/Scrolling/ScrollSnapOffsetsInfo.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Page/VisualViewport.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollAlignment.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollAnimator.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollbarTheme.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderListBox.hpp"
#include "Krystal.Booey/Rendering/Iterators/RenderDescendantIterator.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayerBacking.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayerCompositor.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/AnchorPositionEvaluator.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Booey/Style/Values/Scrollbars/ScrollbarWidth.hpp"
#include "Krystal.Booey/SVG/Elements/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/SVGDocument.hpp"
#include "Krystal.Booey/URL/URLParser.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Threading/NeverDestroyed.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  LocalFrameView::LocalFrameView(LocalFrame &frame) noexcept
      : _frame(CreateWeakPtr(&frame)), _mediaType(css::mq::ScreenAtom()), _layoutContext(*this),
        _delayedScrollToFocusedElementTimer(*this, &LocalFrameView::ScrollToFocusedElementTimerFired)
  {
  }

  LocalFrameView::~LocalFrameView() noexcept = default;

  namespace
  {
    /// @brief Whether a new view for the frame starts shown: when its page is visible, or always for a frame
    /// without a page, as nothing would show it.
    KRYS_NODISCARD bool NewViewStartsShown(const LocalFrame &frame) noexcept
    {
      auto *page = frame.GetPage();
      return !page || page->IsVisible();
    }
  }

  Ref<LocalFrameView> LocalFrameView::Create(LocalFrame &frame) noexcept
  {
    Ref<LocalFrameView> view = AdoptRef(*new LocalFrameView(frame));
    if (NewViewStartsShown(frame))
    {
      view->Show();
    }
    return view;
  }

  Ref<LocalFrameView> LocalFrameView::Create(LocalFrame &frame, const IntSize &initialSize) noexcept
  {
    Ref<LocalFrameView> view = AdoptRef(*new LocalFrameView(frame));
    view->_frameRect = IntRect(view->_frameRect.Location(), initialSize);
    if (NewViewStartsShown(frame))
    {
      view->Show();
    }
    return view;
  }

  void LocalFrameView::Show() noexcept
  {
    _selfVisible = true;
    // FIXME: Pausing speculative tiling for the main frame, once there's tiling.
  }

  void LocalFrameView::Hide() noexcept
  {
    _selfVisible = false;
  }

  LocalFrame &LocalFrameView::Frame() const noexcept
  {
    krys_assert_msg(_frame, "a view doesn't outlive its frame");
    return *_frame;
  }

  const RenderView *LocalFrameView::GetRenderView() const noexcept
  {
    auto *document = Frame().Document();
    return document ? document->GetRenderView() : nullptr;
  }

  void LocalFrameView::SetFrameRect(const IntRect &newRect) noexcept
  {
    Ref protectedThis = ShareRef(*this);
    IntRect oldRect = FrameRect();
    if (newRect == oldRect)
    {
      return;
    }

    _frameRect = newRect;
    RepaintEntireView();

    if (!_useFixedLayout && oldRect.Size() != newRect.Size())
    {
      AvailableContentSizeChanged(AvailableSizeChangeReason::AreaSizeChanged);
    }

    if (auto *renderView = GetRenderViewForUpdate())
    {
      if (renderView->UsesCompositing())
      {
        renderView->Compositor().FrameViewDidChangeSize();
      }
    }

    ViewportContentsChanged();

    if (auto *imageDocument = DynamicDowncast<html::ImageDocument>(Frame().Document()))
    {
      imageDocument->DidChangeViewSize();
    }
  }

  void LocalFrameView::Resize(const IntSize &size) noexcept
  {
    SetFrameRect(IntRect(_frameRect.Location(), size));
  }

  void LocalFrameView::RepaintContentRectangle(const IntRect &rect) noexcept
  {
    // Damage is kept while the view is off screen, so the host paints it once the view is shown.
    InvalidateRect(ContentsToView(rect));
  }

  void LocalFrameView::InvalidateRect(const IntRect &rect) noexcept
  {
    auto damage = Intersection(rect, IntRect(IntPoint(), _frameRect.Size()));
    if (damage.IsEmpty())
    {
      return;
    }
    _damage.Unite(Region(damage));
    // The host paints the damage in its next frame.
    if (auto *frame = _frame.get())
    {
      if (auto *page = frame->GetPage())
      {
        page->GetChromeClient().InvalidateContentsAndRootView(damage);
      }
      if (auto *document = frame->Document())
      {
        document->ScheduleRenderingUpdate({});
      }
    }
  }

  void LocalFrameView::RepaintEntireView() noexcept
  {
    InvalidateRect(IntRect(IntPoint(), _frameRect.Size()));
  }

  void LocalFrameView::InvalidateScrollbar(Scrollbar &scrollbar, const IntRect &rect) noexcept
  {
    if (&scrollbar == _horizontalScrollbar.get())
    {
      if (GraphicsLayer *graphicsLayer = LayerForHorizontalScrollbar())
      {
        graphicsLayer->SetNeedsDisplay();
        graphicsLayer->SetContentsNeedsDisplay();
        return;
      }
    }
    else if (&scrollbar == _verticalScrollbar.get())
    {
      if (GraphicsLayer *graphicsLayer = LayerForVerticalScrollbar())
      {
        graphicsLayer->SetNeedsDisplay();
        graphicsLayer->SetContentsNeedsDisplay();
        return;
      }
    }

    // The frame view's scrollbars are in its coordinates.
    auto damage = rect;
    damage.MoveBy(scrollbar.Location());
    InvalidateRect(damage);
  }

  Region LocalFrameView::TakeDamage() noexcept
  {
    return std::exchange(_damage, Region {});
  }

  Maybe<ScrolledContents> LocalFrameView::TakeScrolledContents() noexcept
  {
    return std::exchange(_scrolledContents, null);
  }

  void LocalFrameView::ScrollHostContents(const IntSize &scrollDelta, const IntRect &rectToScroll) noexcept
  {
    if (rectToScroll.IsEmpty())
    {
      return;
    }

    // The compositor moves a composited page's contents: the host has nothing to move.
    if (auto *renderView = GetRenderViewForUpdate(); renderView && renderView->UsesCompositing())
    {
      return;
    }

    // Moves that haven't been painted yet add up, while they move the same rect; otherwise all of it is
    // painted again.
    if (_scrolledContents && _scrolledContents->Rect != rectToScroll)
    {
      _scrolledContents = null;
      InvalidateRect(rectToScroll);
      return;
    }
    if (_scrolledContents)
    {
      _scrolledContents->Delta += scrollDelta;
    }
    else
    {
      _scrolledContents = ScrolledContents {.Delta = scrollDelta, .Rect = rectToScroll};
    }

    // The damage in the rect moves with the contents.
    Region moved = _damage;
    moved.Intersect(Region(rectToScroll));
    moved.Translate(scrollDelta);
    moved.Intersect(Region(rectToScroll));
    _damage.Subtract(Region(rectToScroll));
    _damage.Unite(moved);

    // What comes into view is painted.
    Region exposed(rectToScroll);
    exposed.Subtract(Region(IntRect(rectToScroll.Location() + scrollDelta, rectToScroll.Size())));
    for (const auto &rect : exposed.Rects())
    {
      InvalidateRect(rect);
    }
    if (exposed.IsEmpty())
    {
      // Nothing came into view, but the contents still moved: the host has a frame to paint.
      if (auto *frame = _frame.get())
      {
        if (auto *document = frame->Document())
        {
          document->ScheduleRenderingUpdate({});
        }
      }
    }
  }

  IntRect LocalFrameView::RectToCopyOnScroll() const noexcept
  {
    IntRect scrollViewRect(0, 0, VisibleWidth(), VisibleHeight());
    if (ShouldPlaceVerticalScrollbarOnLeft() && _verticalScrollbar)
    {
      scrollViewRect.Move(_verticalScrollbar->OccupiedWidth(), 0);
    }
    return scrollViewRect;
  }

  void LocalFrameView::ScrollContents(const IntSize &scrollDelta) noexcept
  {
    // FIXME: The pan scroll icon, once there's pan scrolling.
    IntRect clipRect(IntPoint(), _frameRect.Size());
    IntRect scrollViewRect = RectToCopyOnScroll();
    IntRect updateRect = clipRect;
    updateRect.Intersect(scrollViewRect);

    if (!UseSlowRepaints())
    {
      if (!ScrollContentsFastPath(-scrollDelta, scrollViewRect))
      {
        ScrollContentsSlowPath(updateRect);
      }
    }
    else
    {
      // We need to repaint the entire backing store.
      ScrollContentsSlowPath(updateRect);
    }
  }

  bool LocalFrameView::ScrollContentsFastPath(const IntSize &scrollDelta,
                                              const IntRect &rectToScroll) noexcept
  {
    if (_viewportConstrainedObjects.empty())
    {
      ScrollHostContents(scrollDelta, rectToScroll);
      return true;
    }

    bool isCompositedContentLayer = UsesCompositedScrolling();

    // Get the rects of the fixed objects visible in the rectToScroll
    Region regionToUpdate;
    for (const auto *renderer : _viewportConstrainedObjects)
    {
      if (!renderer->Style().HasViewportConstrainedPosition())
      {
        continue;
      }
      if (renderer->IsComposited())
      {
        continue;
      }

      // Fixed items should always have layers.
      krys_debug_assert(renderer->HasLayer());
      auto *layer = renderer->Layer();
      if (!layer)
      {
        continue;
      }

      if (layer->GetViewportConstrainedNotCompositedReason() == RenderLayer::NotCompositedForBoundsOutOfView
          || layer->GetViewportConstrainedNotCompositedReason()
               == RenderLayer::NotCompositedForNoVisibleContent)
      {
        // Don't invalidate for invisible fixed layers.
        continue;
      }

      if (layer->HasAncestorWithFilterOutsets())
      {
        // If the fixed layer has a blur/drop-shadow filter applied on at least one of its parents, we cannot
        // scroll using the fast path, otherwise the outsets of the filter will be moved around the page.
        return false;
      }

      // FIXME: use pixel snapping instead of enclosing when ScrollView has finished transitioning from
      // IntRect to Float/LayoutRect.
      IntRect updateRect = EnclosingIntRect(layer->RepaintRectIncludingNonCompositingDescendants());
      updateRect = ContentsToView(updateRect);
      if (!isCompositedContentLayer)
      {
        updateRect.Intersect(rectToScroll);
      }
      if (!updateRect.IsEmpty())
      {
        regionToUpdate.Unite(Region(updateRect));
      }
    }

    // 1) scroll
    ScrollHostContents(scrollDelta, rectToScroll);

    // 2) update the area of fixed objects that has been invalidated
    for (auto updateRect : regionToUpdate.Rects())
    {
      IntRect scrolledRect = updateRect;
      scrolledRect.Move(scrollDelta);
      updateRect.Unite(scrolledRect);
      if (isCompositedContentLayer)
      {
        updateRect = ViewToContents(updateRect);
        krys_debug_assert(GetRenderView());
        GetRenderViewForUpdate()->Layer()->SetBackingNeedsRepaintInRect(LayoutRect(updateRect));
        continue;
      }
      updateRect.Intersect(rectToScroll);
      InvalidateRect(updateRect);
    }

    return true;
  }

#pragma region Compositing

  void LocalFrameView::WillRecalcStyle() noexcept
  {
    auto *renderView = GetRenderViewForUpdate();
    if (!renderView)
    {
      return;
    }

    renderView->Compositor().WillRecalcStyle();
  }

  void LocalFrameView::SetIsInWindow(bool isInWindow) noexcept
  {
    // FIXME: Widgets (setParentVisible), once there are any.
    if (auto *renderView = GetRenderViewForUpdate())
    {
      renderView->SetIsInWindow(isInWindow);
    }
  }

  bool LocalFrameView::FlushCompositingState() noexcept
  {
    auto *renderView = GetRenderViewForUpdate();
    if (!renderView)
    {
      return true; // We don't want to keep trying to update layers if we have no renderer.
    }

    // If we sync compositing layers when a layout is pending, we may cause painting of compositing layer
    // content to occur before layout has happened, which will cause paintContents() to bail.
    if (_layoutContext.NeedsLayout())
    {
      return false;
    }

    renderView->Compositor().FlushPendingLayerChanges();

    return true;
  }

  void LocalFrameView::EnterCompositingMode() noexcept
  {
    if (auto *renderView = GetRenderViewForUpdate())
    {
      renderView->Compositor().EnableCompositingMode();
      if (!_layoutContext.NeedsLayout())
      {
        renderView->Compositor().ScheduleCompositingLayerUpdate();
      }
    }
  }

  bool LocalFrameView::UsesCompositedScrolling() const noexcept
  {
    auto *renderView = GetRenderViewForUpdate();
    if (renderView && renderView->IsComposited())
    {
      GraphicsLayer *layer = renderView->Layer()->Backing()->GetGraphicsLayer();
      if (layer && layer->DrawsContent())
      {
        return true;
      }
    }

    return false;
  }

  bool LocalFrameView::IsEnclosedInCompositingLayer() const noexcept
  {
    // FIXME: Frames inside frames (the owner renderer's repaint container), once there are any.
    return false;
  }

  void LocalFrameView::SetNeedsCompositingConfigurationUpdate() noexcept
  {
    auto *renderView = GetRenderViewForUpdate();
    if (renderView && renderView->UsesCompositing())
    {
      if (auto *rootLayer = renderView->Layer())
      {
        rootLayer->SetNeedsCompositingConfigurationUpdate();
      }
      renderView->Compositor().ScheduleCompositingLayerUpdate();
    }
  }

  void LocalFrameView::SetNeedsCompositingGeometryUpdate() noexcept
  {
    auto *renderView = GetRenderViewForUpdate();
    if (renderView && renderView->UsesCompositing())
    {
      if (auto *rootLayer = renderView->Layer())
      {
        rootLayer->SetNeedsCompositingGeometryUpdate();
      }
      renderView->Compositor().ScheduleCompositingLayerUpdate();
    }
  }

  void LocalFrameView::SetDescendantsNeedUpdateBackingAndHierarchyTraversal() noexcept
  {
    auto *renderView = GetRenderViewForUpdate();
    if (renderView && renderView->UsesCompositing())
    {
      if (auto *rootLayer = renderView->Layer())
      {
        rootLayer->SetDescendantsNeedUpdateBackingAndHierarchyTraversal();
      }
      renderView->Compositor().ScheduleCompositingLayerUpdate();
    }
  }

  GraphicsLayer *LocalFrameView::LayerForHorizontalScrollbar() const noexcept
  {
    auto *renderView = GetRenderViewForUpdate();
    if (!renderView || !renderView->UsesCompositing())
    {
      return nullptr;
    }
    return renderView->Compositor().LayerForHorizontalScrollbar();
  }

  GraphicsLayer *LocalFrameView::LayerForVerticalScrollbar() const noexcept
  {
    auto *renderView = GetRenderViewForUpdate();
    if (!renderView || !renderView->UsesCompositing())
    {
      return nullptr;
    }
    return renderView->Compositor().LayerForVerticalScrollbar();
  }

  GraphicsLayer *LocalFrameView::LayerForScrollCorner() const noexcept
  {
    auto *renderView = GetRenderViewForUpdate();
    if (!renderView || !renderView->UsesCompositing())
    {
      return nullptr;
    }
    return renderView->Compositor().LayerForScrollCorner();
  }

  namespace
  {
    void PositionScrollbarLayer(GraphicsLayer *graphicsLayer, Scrollbar *scrollbar) noexcept
    {
      if (!graphicsLayer || !scrollbar)
      {
        return;
      }

      IntRect scrollbarRect = scrollbar->FrameRect();
      graphicsLayer->SetPosition(FloatPoint(scrollbarRect.Location()));

      if (FloatSize(scrollbarRect.Size()) == graphicsLayer->Size())
      {
        return;
      }

      graphicsLayer->SetSize(FloatSize(scrollbarRect.Size()));

      if (graphicsLayer->UsesContentsLayer())
      {
        graphicsLayer->SetContentsRect(FloatRect(0, 0, static_cast<float>(scrollbarRect.Width()),
                                                 static_cast<float>(scrollbarRect.Height())));
        return;
      }

      graphicsLayer->SetDrawsContent(true);
      graphicsLayer->SetNeedsDisplay();
    }

    void PositionScrollCornerLayer(GraphicsLayer *graphicsLayer, const IntRect &cornerRect) noexcept
    {
      if (!graphicsLayer)
      {
        return;
      }
      graphicsLayer->SetDrawsContent(!cornerRect.IsEmpty());
      graphicsLayer->SetPosition(FloatPoint(cornerRect.Location()));
      if (FloatSize(cornerRect.Size()) != graphicsLayer->Size())
      {
        graphicsLayer->SetNeedsDisplay();
      }
      graphicsLayer->SetSize(FloatSize(cornerRect.Size()));
    }
  }

  void LocalFrameView::PositionScrollbarLayers() noexcept
  {
    PositionScrollbarLayer(LayerForHorizontalScrollbar(), _horizontalScrollbar.get());
    PositionScrollbarLayer(LayerForVerticalScrollbar(), _verticalScrollbar.get());
    PositionScrollCornerLayer(LayerForScrollCorner(), ScrollCornerRect());
  }

  void LocalFrameView::InvalidateScrollCorner(const IntRect &rect) noexcept
  {
    if (GraphicsLayer *graphicsLayer = LayerForScrollCorner())
    {
      graphicsLayer->SetNeedsDisplay();
      return;
    }

    InvalidateRect(rect);
  }

  boo::Color LocalFrameView::DocumentBackgroundColor() const noexcept
  {
    // <https://bugs.webkit.org/show_bug.cgi?id=59540> We blend the background color of the document and the
    // body against the base background color of the frame view. Background images are unfortunately
    // impractical to include.

    // Return invalid Color objects whenever there is insufficient information.
    // FIXME: The fullscreen document and element, once there's fullscreen.
    auto *backgroundDocument = Frame().Document();
    if (!backgroundDocument)
    {
      return boo::Color();
    }

    auto *htmlElement = backgroundDocument->DocumentElement().get();
    auto *bodyElement = backgroundDocument->BodyOrFrameset();

    // Start with invalid colors.
    boo::Color htmlBackgroundColor;
    boo::Color bodyBackgroundColor;
    if (htmlElement && htmlElement->Renderer())
    {
      htmlBackgroundColor =
        htmlElement->Renderer()->Style().ColorResolvingCurrentColor(css::PropertyId::BackgroundColor);
    }
    if (bodyElement && bodyElement->Renderer())
    {
      bodyBackgroundColor =
        bodyElement->Renderer()->Style().ColorResolvingCurrentColor(css::PropertyId::BackgroundColor);
    }

    if (!bodyBackgroundColor.IsValid())
    {
      if (!htmlBackgroundColor.IsValid())
      {
        return boo::Color();
      }
      return BlendSourceOver(BaseBackgroundColor(), htmlBackgroundColor);
    }

    if (!htmlBackgroundColor.IsValid())
    {
      return BlendSourceOver(BaseBackgroundColor(), bodyBackgroundColor);
    }

    // We take the aggregate of the base background color the <html> background color, and the <body>
    // background color to find the document color. The addition of the base background color is not
    // technically part of the document background, but it otherwise poses problems when the aggregate is not
    // a fully opaque color.
    return BlendSourceOver(BlendSourceOver(BaseBackgroundColor(), htmlBackgroundColor), bodyBackgroundColor);
  }

  style::ScrollbarGutter LocalFrameView::ScrollbarGutterStyle() const noexcept
  {
    auto *document = Frame().Document();
    auto *documentElement = document ? dom::TreeQueries::DocumentElement(*document) : nullptr;
    if (auto *scrollingObject = documentElement ? documentElement->Renderer() : nullptr)
    {
      return scrollingObject->Style().GetScrollbarGutter();
    }
    return css::keywords::Auto {};
  }

  int32 LocalFrameView::ScrollbarGutterWidth(bool isHorizontalWritingMode) const noexcept
  {
    if (_verticalScrollbar && _verticalScrollbar->IsOverlayScrollbar())
    {
      return 0;
    }

    // A stable gutter keeps the scrollbar's space without it.
    auto &theme = ScrollbarTheme::Theme();
    if (!_verticalScrollbar && !(ScrollbarGutterStyle().IsAuto() || theme.UsesOverlayScrollbars())
        && isHorizontalWritingMode)
    {
      return theme.ScrollbarThickness(ScrollbarWidthStyle());
    }

    if (!_verticalScrollbar)
    {
      return 0;
    }

    return _verticalScrollbar->Width();
  }

  IntSize LocalFrameView::TotalScrollbarSpace() const noexcept
  {
    IntSize scrollbarGutter(HorizontalScrollbarIntrusion(), VerticalScrollbarIntrusion());

    if (IsHorizontalWritingMode())
    {
      if (ScrollbarGutterStyle().IsStableBothEdges())
      {
        scrollbarGutter.SetWidth(ScrollbarGutterWidth() * 2);
      }
      else
      {
        scrollbarGutter.SetWidth(ScrollbarGutterWidth());
      }
    }
    return scrollbarGutter;
  }

  bool LocalFrameView::IsHorizontalWritingMode() const noexcept
  {
    auto *renderView = GetRenderView();
    return renderView && renderView->GetWritingMode().IsHorizontal();
  }

  int32 LocalFrameView::InsetForLeftScrollbarSpace() const noexcept
  {
    if (ScrollbarGutterStyle().IsStableBothEdges())
    {
      return ScrollbarGutterWidth();
    }
    if (ShouldPlaceVerticalScrollbarOnLeft())
    {
      return _verticalScrollbar ? _verticalScrollbar->OccupiedWidth() : 0;
    }
    return 0;
  }

  FloatPoint LocalFrameView::PositionForRootContentLayer() const noexcept
  {
    // FIXME: Obscured content insets and header layers, once there are any.
    return FloatPoint(static_cast<float>(-ScrollOrigin().X()), static_cast<float>(-ScrollOrigin().Y()));
  }

  LayoutRect LocalFrameView::FixedScrollableAreaBoundsInflatedForScrolling(
    const LayoutRect &uninflatedBounds) const noexcept
  {
    // FIXME: Visual viewports (layoutViewportRect, unscaledMinimumScrollPosition), once there are any.
    LayoutPoint scrollPosition = ScrollPositionRespectingCustomFixedPosition();
    LayoutSize topLeftExpansion = scrollPosition - LayoutPoint(MinimumScrollPosition());
    LayoutSize bottomRightExpansion = LayoutPoint(MaximumScrollPosition()) - scrollPosition;

    return LayoutRect(uninflatedBounds.Location() - topLeftExpansion,
                      uninflatedBounds.Size() + topLeftExpansion + bottomRightExpansion);
  }

  void LocalFrameView::UpdateCompositingLayersAfterScrolling() noexcept
  {
    if (!_layoutContext.IsLayoutNested() && !_viewportConstrainedObjects.empty())
    {
      if (auto *renderView = GetRenderViewForUpdate())
      {
        renderView->Compositor().UpdateCompositingLayers(CompositingUpdateType::OnScroll);
      }
    }
  }

  void LocalFrameView::AddedOrRemovedScrollbar() noexcept
  {
    if (auto *renderView = GetRenderViewForUpdate())
    {
      if (renderView->UsesCompositing())
      {
        renderView->Compositor().FrameViewDidAddOrRemoveScrollbars();
      }
    }

    // FIXME: Tiled backings' adaptive sizing and the inspector, once there are those.
  }

#pragma endregion

  void LocalFrameView::ScrollContentsSlowPath(const IntRect &updateRect) noexcept
  {
    RepaintSlowRepaintObjects();

    InvalidateRect(updateRect);
  }

  void LocalFrameView::RepaintSlowRepaintObjects() noexcept
  {
    // Renderers with fixed backgrounds may be in compositing layers, so we need to explicitly
    // repaint them after scrolling.
    for (const auto *renderer : _slowRepaintObjects)
    {
      renderer->RepaintSlowRepaintObject();
    }
  }

  bool LocalFrameView::UseSlowRepaints() const noexcept
  {
    // The host moves the page's own pixels, so the contents needn't be opaque, and there's no window to blit
    // into (m_contentIsOpaque, m_cannotBlitToWindow, overlapping).
    // FIXME: Frames in frames use their parent's, once there are any.
    return HasSlowRepaintObjects();
  }

  void LocalFrameView::AddSlowRepaintObject(const RenderElement &renderer) noexcept
  {
    // FIXME: The enclosing layer's scrolling tree update, once there's a scrolling tree.
    _slowRepaintObjects.insert(&renderer);
  }

  void LocalFrameView::RemoveSlowRepaintObject(const RenderElement &renderer) noexcept
  {
    _slowRepaintObjects.erase(&renderer);
  }

  bool LocalFrameView::HasSlowRepaintObject(const RenderElement &renderer) const noexcept
  {
    return _slowRepaintObjects.contains(&renderer);
  }

  void LocalFrameView::AddViewportConstrainedObject(const RenderLayerModelObject &object) noexcept
  {
    // FIXME: The scrolling coordinator (and the anchor-positioned viewport-constrained objects it can't
    // scroll), once there's one.
    _viewportConstrainedObjects.insert(&object);
  }

  void LocalFrameView::RemoveViewportConstrainedObject(const RenderLayerModelObject &object) noexcept
  {
    _viewportConstrainedObjects.erase(&object);
  }

  IntSize LocalFrameView::GetLayoutSize() const noexcept
  {
    return _fixedLayoutSize.IsEmpty() || !_useFixedLayout ? UnobscuredContentRect().Size() : _fixedLayoutSize;
  }

  void LocalFrameView::SetFixedLayoutSize(const IntSize &newSize) noexcept
  {
    if (FixedLayoutSize() == newSize)
    {
      return;
    }

    _fixedLayoutSize = newSize;
    if (_useFixedLayout)
    {
      AvailableContentSizeChanged(AvailableSizeChangeReason::AreaSizeChanged);
    }
  }

  void LocalFrameView::SetUseFixedLayout(bool enable) noexcept
  {
    if (UseFixedLayout() == enable)
    {
      return;
    }
    _useFixedLayout = enable;
    if (!_fixedLayoutSize.IsEmpty())
    {
      AvailableContentSizeChanged(AvailableSizeChangeReason::AreaSizeChanged);
    }
  }

  IntRect LocalFrameView::VisibleContentRectInternal(
    VisibleContentRectIncludesScrollbars scrollbarInclusion) const noexcept
  {
    return UnobscuredContentRect(scrollbarInclusion);
  }

  IntRect LocalFrameView::UnobscuredContentRect(
    VisibleContentRectIncludesScrollbars scrollbarInclusion) const noexcept
  {
    // FIXME: Less the obscured content insets and the visible content scale, once there are those.
    return IntRect(_scrollPosition, SizeForVisibleContent(scrollbarInclusion));
  }

  IntSize LocalFrameView::SizeForVisibleContent(
    VisibleContentRectIncludesScrollbars scrollbarInclusion) const noexcept
  {
    IntSize scrollbarSpace;
    if (!ScrollbarGutterStyle().IsAuto() && scrollbarInclusion == VisibleContentRectIncludesScrollbars::No)
    {
      scrollbarSpace = TotalScrollbarSpace();
    }
    else if (scrollbarInclusion == VisibleContentRectIncludesScrollbars::No)
    {
      scrollbarSpace = ScrollbarIntrusion();
    }

    return IntSize(Width() - scrollbarSpace.Width(), Height() - scrollbarSpace.Height())
      .ExpandedTo(IntSize());
  }

  void LocalFrameView::SetTransparent(bool isTransparent) noexcept
  {
    if (_isTransparent == isTransparent)
    {
      return;
    }

    _isTransparent = isTransparent;
    _layoutContext.SetNeedsLayoutAfterViewConfigurationChange();
    SetNeedsCompositingConfigurationUpdate();
  }

  void LocalFrameView::SetBaseBackgroundColor(const boo::Color &backgroundColor) noexcept
  {
    _baseBackgroundColor = backgroundColor.IsValid() ? backgroundColor : boo::Color(boo::Color::white);
    RecalculateScrollbarOverlayStyle();
  }

  void LocalFrameView::Paint(GraphicsContext &context, const IntRect &rect) noexcept
  {
    // FIXME: Painting disabled contexts and region contexts, once there are those.
    IntRect documentDirtyRect = rect;
    // FIXME: Painting the entire contents (paintsEntireContents), once something asks for it.
    IntRect visibleAreaWithoutScrollbars(FrameRect().Location(), VisibleContentRect().Size());
    documentDirtyRect.Intersect(visibleAreaWithoutScrollbars);

    if (!documentDirtyRect.IsEmpty())
    {
      GraphicsContextStateSaver stateSaver(context);

      IntPoint locationOfContents = FrameRect().Location();
      context.Translate(static_cast<float>(locationOfContents.X()),
                        static_cast<float>(locationOfContents.Y()));
      documentDirtyRect.MoveBy(-locationOfContents);

      context.Translate(static_cast<float>(-ScrollX()), static_cast<float>(-ScrollY()));
      documentDirtyRect.MoveBy(_scrollPosition);

      context.Clip(FloatRect(VisibleContentRect()));

      PaintContents(context, documentDirtyRect);
    }

    // Now paint the scrollbars.
    if (!_scrollbarsSuppressed && (_horizontalScrollbar || _verticalScrollbar))
    {
      GraphicsContextStateSaver stateSaver(context);
      IntRect scrollViewDirtyRect = rect;
      IntRect visibleAreaWithScrollbars(
        FrameRect().Location(), UnobscuredContentRect(VisibleContentRectIncludesScrollbars::Yes).Size());
      scrollViewDirtyRect.Intersect(visibleAreaWithScrollbars);
      context.Translate(static_cast<float>(FrameRect().X()), static_cast<float>(FrameRect().Y()));
      scrollViewDirtyRect.MoveBy(-FrameRect().Location());
      context.Clip(FloatRect(IntRect(IntPoint(), visibleAreaWithScrollbars.Size())));

      PaintScrollbarsBackground(context);
      PaintScrollbars(context, scrollViewDirtyRect);
    }

    if (_panScrollIconPosition)
    {
      GraphicsContextStateSaver stateSaver(context);
      // A light disc with a dot: the place the mouse pans the page from.
      auto disc = [&](float radius, const Color &color)
      {
        context.SetFillColor(color);
        context.FillEllipse(FloatRect(static_cast<float>(_panScrollIconPosition->X()) - radius,
                                      static_cast<float>(_panScrollIconPosition->Y()) - radius, radius * 2,
                                      radius * 2));
      };
      disc(PanScrollIconSize / 2.0f, Color::black);
      disc(PanScrollIconSize / 2.0f - 1, Color::white);
      disc(2, Color::black);
    }
  }

  void LocalFrameView::AddPanScrollIcon(const IntPoint &iconPosition) noexcept
  {
    _panScrollIconPosition = iconPosition;
    InvalidatePanScrollIcon(iconPosition);
  }

  void LocalFrameView::RemovePanScrollIcon() noexcept
  {
    if (auto position = std::exchange(_panScrollIconPosition, null))
    {
      InvalidatePanScrollIcon(*position);
    }
  }

  void LocalFrameView::InvalidatePanScrollIcon(const IntPoint &iconPosition) noexcept
  {
    auto *frame = _frame.get();
    auto *page = frame ? frame->GetPage() : nullptr;
    if (page)
    {
      page->GetChromeClient().InvalidateContentsAndRootView(IntRect(iconPosition.X() - PanScrollIconSize / 2,
                                                                    iconPosition.Y() - PanScrollIconSize / 2,
                                                                    PanScrollIconSize, PanScrollIconSize));
    }
    if (auto *document = frame ? frame->Document() : nullptr)
    {
      document->ScheduleRenderingUpdate({});
    }
  }

  void LocalFrameView::PaintScrollbars(GraphicsContext &context, const IntRect &rect) noexcept
  {
    // Flattened paints (snapshots) paint the scrollbars into the contents, though they have layers.
    const bool flattening = _paintBehavior.Contains(PaintBehavior::FlattenCompositingLayers);
    if (_horizontalScrollbar && (flattening || !LayerForHorizontalScrollbar()))
    {
      _horizontalScrollbar->Paint(context, rect);
    }
    if (_verticalScrollbar && (flattening || !LayerForVerticalScrollbar()))
    {
      _verticalScrollbar->Paint(context, rect);
    }

    if (flattening || !LayerForScrollCorner())
    {
      PaintScrollCorner(context, ScrollCornerRect());
    }
  }

  void LocalFrameView::PaintScrollbarsBackground(GraphicsContext &context) noexcept
  {
    // As the root background is painted: its colour over the base background colour, unless the view is
    // transparent.
    // FIXME: The root's background images, which stop at the scrollbars.
    Color color = IsTransparent() ? Color() : BaseBackgroundColor();
    if (auto *renderView = GetRenderView())
    {
      if (auto *rootBackground = renderView->RendererForRootBackground())
      {
        auto rootColor = rootBackground->Style().ColorResolvingCurrentColor(css::PropertyId::BackgroundColor);
        color = color.IsVisible() ? BlendSourceOver(color, rootColor) : rootColor;
      }
    }
    if (!color.IsVisible())
    {
      return;
    }

    if (_horizontalScrollbar)
    {
      context.FillRect(FloatRect(_horizontalScrollbar->FrameRect()), color);
    }
    if (_verticalScrollbar)
    {
      context.FillRect(FloatRect(_verticalScrollbar->FrameRect()), color);
    }
    if (auto cornerRect = ScrollCornerRect(); !cornerRect.IsEmpty())
    {
      context.FillRect(FloatRect(cornerRect), color);
    }
  }

  void LocalFrameView::PaintScrollCorner(GraphicsContext &context, const IntRect &cornerRect) noexcept
  {
    if (cornerRect.IsEmpty())
    {
      return;
    }
    ScrollbarTheme::Theme().PaintScrollCorner(*this, context, cornerRect);
  }

  IntRect LocalFrameView::ScrollCornerRect() const noexcept
  {
    IntRect cornerRect;

    if (HasOverlayScrollbars())
    {
      return cornerRect;
    }

    int32 widthTrackedByScrollbar = Width();
    int32 heightTrackedByScrollbar = Height();

    if (_horizontalScrollbar && widthTrackedByScrollbar > _horizontalScrollbar->Width())
    {
      cornerRect.Unite(IntRect(ShouldPlaceVerticalScrollbarOnLeft() ? 0 : _horizontalScrollbar->Width(),
                               Height() - _horizontalScrollbar->Height(),
                               Width() - _horizontalScrollbar->Width(), _horizontalScrollbar->Height()));
    }

    if (_verticalScrollbar && heightTrackedByScrollbar > _verticalScrollbar->Height())
    {
      cornerRect.Unite(
        IntRect(ShouldPlaceVerticalScrollbarOnLeft() ? 0 : Width() - _verticalScrollbar->Width(),
                _verticalScrollbar->Height(), _verticalScrollbar->Width(),
                heightTrackedByScrollbar - _verticalScrollbar->Height()));
    }

    return cornerRect;
  }

  bool LocalFrameView::IsPointInScrollbarCorner(const IntPoint &windowPoint) const noexcept
  {
    if (!ScrollbarCornerPresent())
    {
      return false;
    }

    // The view is the root of its (absent) widget tree, so the window's coordinates are its own.
    const auto &viewPoint = windowPoint;

    if (_horizontalScrollbar)
    {
      auto &frame = _horizontalScrollbar->FrameRect();
      return viewPoint.Y() > frame.Y() && viewPoint.Y() < frame.MaxY() && viewPoint.X() > frame.MaxX();
    }

    auto &frame = _verticalScrollbar->FrameRect();
    return viewPoint.X() > frame.X() && viewPoint.X() < frame.MaxX() && viewPoint.Y() > frame.MaxY();
  }

  bool LocalFrameView::ScrollbarCornerPresent() const noexcept
  {
    return (_horizontalScrollbar && Width() - _horizontalScrollbar->Width() > 0)
           || (_verticalScrollbar && Height() - _verticalScrollbar->Height() > 0);
  }

  Scrollbar *LocalFrameView::ScrollbarAtPoint(const IntPoint &windowPoint) const noexcept
  {
    // The view is the root of its (absent) widget tree, so the window's coordinates are its own.
    if (_horizontalScrollbar && _horizontalScrollbar->ShouldParticipateInHitTesting()
        && _horizontalScrollbar->FrameRect().Contains(windowPoint))
    {
      return _horizontalScrollbar.get();
    }
    if (_verticalScrollbar && _verticalScrollbar->ShouldParticipateInHitTesting()
        && _verticalScrollbar->FrameRect().Contains(windowPoint))
    {
      return _verticalScrollbar.get();
    }
    return nullptr;
  }

  void LocalFrameView::SetScrollbarModes(ScrollbarMode horizontalMode, ScrollbarMode verticalMode,
                                         bool horizontalLock, bool verticalLock) noexcept
  {
    bool needsUpdate = false;

    if (horizontalMode != HorizontalScrollbarMode() && !_horizontalScrollbarLock)
    {
      _horizontalScrollbarMode = horizontalMode;
      needsUpdate = true;
    }

    if (verticalMode != VerticalScrollbarMode() && !_verticalScrollbarLock)
    {
      _verticalScrollbarMode = verticalMode;
      needsUpdate = true;
    }

    if (horizontalLock)
    {
      _horizontalScrollbarLock = true;
    }

    if (verticalLock)
    {
      _verticalScrollbarLock = true;
    }

    if (!needsUpdate)
    {
      return;
    }

    UpdateScrollbars(GetScrollPosition());
  }

  bool LocalFrameView::SetHasHorizontalScrollbar(bool hasBar, bool *contentSizeAffected) noexcept
  {
    return SetHasScrollbarInternal(_horizontalScrollbar, ScrollbarOrientation::Horizontal, hasBar,
                                   contentSizeAffected);
  }

  bool LocalFrameView::SetHasVerticalScrollbar(bool hasBar, bool *contentSizeAffected) noexcept
  {
    return SetHasScrollbarInternal(_verticalScrollbar, ScrollbarOrientation::Vertical, hasBar,
                                   contentSizeAffected);
  }

  bool LocalFrameView::SetHasScrollbarInternal(RefPtr<Scrollbar> &scrollbar, ScrollbarOrientation orientation,
                                               bool hasBar, bool *contentSizeAffected) noexcept
  {
    if (hasBar && !scrollbar)
    {
      scrollbar = CreateScrollbar(orientation);
      DidAddScrollbar(scrollbar.get(), orientation);
      scrollbar->StyleChanged();
      if (contentSizeAffected)
      {
        *contentSizeAffected = !scrollbar->IsOverlayScrollbar();
      }
      return true;
    }

    if (!hasBar && scrollbar)
    {
      bool wasOverlayScrollbar = scrollbar->IsOverlayScrollbar();
      WillRemoveScrollbar(*scrollbar, orientation);
      scrollbar = nullptr;
      if (contentSizeAffected)
      {
        *contentSizeAffected = !wasOverlayScrollbar;
      }
      return true;
    }

    return false;
  }

  Ref<Scrollbar> LocalFrameView::CreateScrollbar(ScrollbarOrientation orientation) noexcept
  {
    return Scrollbar::CreateNativeScrollbar(*this, orientation, ScrollbarWidthStyle());
  }

  void LocalFrameView::UpdateScrollbarSteps() noexcept
  {
    // A page is what's in view less the root's scroll padding, which content scrolled to stays clear of.
    LayoutRect paddedViewRect(LayoutPoint(), LayoutSize(VisibleWidth(), VisibleHeight()));
    auto *document = Frame().Document();
    auto *documentElement = document ? dom::TreeQueries::DocumentElement(*document) : nullptr;
    if (auto *renderer = documentElement ? DynamicDowncast<RenderBox>(documentElement->Renderer()) : nullptr)
    {
      paddedViewRect.Contract(renderer->ScrollPaddingForViewportRect(paddedViewRect));
    }

    if (_horizontalScrollbar)
    {
      int32 width = paddedViewRect.Width().ToInt();
      _horizontalScrollbar->SetSteps(Scrollbar::PixelsPerLineStep(width), Scrollbar::PageStep(width));
    }
    if (_verticalScrollbar)
    {
      int32 height = paddedViewRect.Height().ToInt();
      _verticalScrollbar->SetSteps(Scrollbar::PixelsPerLineStep(height), Scrollbar::PageStep(height));
    }
  }

  void LocalFrameView::UpdateContentsSize() noexcept
  {
    // FIXME: Not with a custom fixed position layout rect (shouldLayoutAfterContentsResized), once there is
    // one.
    if (!UseFixedLayout() && _layoutContext.NeedsLayout())
    {
      _layoutContext.Layout();
    }
  }

  ScrollbarWidth LocalFrameView::ScrollbarWidthStyle() const noexcept
  {
    auto *document = Frame().Document();
    auto *documentElement = document ? dom::TreeQueries::DocumentElement(*document) : nullptr;
    auto *scrollingObject = documentElement ? documentElement->Renderer() : nullptr;
    if (scrollingObject && GetRenderView())
    {
      return style::ToPlatform<style::ScrollbarWidth> {}(scrollingObject->Style().GetScrollbarWidth());
    }
    return ScrollbarWidth::Auto;
  }

  Maybe<ScrollbarColor> LocalFrameView::ScrollbarColorStyle() const noexcept
  {
    // The root element's scrollbar-color is the viewport's.
    auto *document = Frame().Document();
    auto *documentElement = document ? dom::TreeQueries::DocumentElement(*document) : nullptr;
    auto *scrollingObject = documentElement ? documentElement->Renderer() : nullptr;
    if (scrollingObject && GetRenderView())
    {
      return scrollingObject->Style().UsedScrollbarColor();
    }
    return null;
  }

  void LocalFrameView::ScrollbarAppearanceChanged() noexcept
  {
    ScrollableArea::ScrollbarAppearanceChanged();
    InvalidateScrollCorner(ScrollCornerRect());
  }

  void LocalFrameView::ScrollbarStyleChanged() noexcept
  {
    ScrollableArea::ScrollbarStyleChanged();
    // Its scrollbars come and go with the space they take.
    UpdateScrollbars(GetScrollPosition());
  }

  bool LocalFrameView::UseDarkAppearanceForScrollbars() const noexcept
  {
    auto *document = Frame().Document();
    auto *documentElement = document ? dom::TreeQueries::DocumentElement(*document) : nullptr;
    auto *renderer = documentElement ? documentElement->Renderer() : nullptr;
    if (renderer && renderer->Style().HasExplicitlySetColorScheme())
    {
      return renderer->UseDarkAppearance();
    }
    if (document && document->UseDarkAppearance(nullptr))
    {
      return true;
    }
    // Overlay scrollbars over a dark page are light, as WebKit's; classic ones keep their appearance, as
    // Chrome's.
    return ScrollbarTheme::Theme().UsesOverlayScrollbars()
           && GetScrollbarOverlayStyle() == ScrollbarOverlayStyle::Light;
  }

  void LocalFrameView::RecalculateScrollbarOverlayStyle() noexcept
  {
    // FIXME: The client's preferred style (ChromeClient::preferredScrollbarOverlayStyle), once a host wants
    // one.
    auto style = [this]
    {
      auto background = DocumentBackgroundColor();
      if (background.IsVisible())
      {
        if (background.Lightness() <= .5)
        {
          return ScrollbarOverlayStyle::Light;
        }
      }
      else if (auto *document = Frame().Document(); document && document->UseDarkAppearance(nullptr))
      {
        return ScrollbarOverlayStyle::Light;
      }
      return ScrollbarOverlayStyle::Default;
    }();
    if (GetScrollbarOverlayStyle() != style)
    {
      // The scroll corner is repainted with the scrollbars, to match their new appearance.
      SetScrollbarOverlayStyle(style);
    }
  }

  bool LocalFrameView::AddScrollableArea(ScrollableArea &scrollableArea) noexcept
  {
    // FIXME: Telling the scrolling coordinator (scrollableAreaSetChanged), once there's one.
    return _scrollableAreas.insert(&scrollableArea).second;
  }

  bool LocalFrameView::RemoveScrollableArea(ScrollableArea &scrollableArea) noexcept
  {
    return _scrollableAreas.erase(&scrollableArea) != 0;
  }

  bool LocalFrameView::ContainsScrollableArea(const ScrollableArea &scrollableArea) const noexcept
  {
    return _scrollableAreas.contains(const_cast<ScrollableArea *>(&scrollableArea));
  }

  void LocalFrameView::ForEachScrollableArea(const Func<void(ScrollableArea &)> &function) noexcept
  {
    function(*this);
    // The function may lay out, which adds and removes areas, or destroy one, so it goes over a copy that
    // forgets the destroyed.
    List<WeakPtr<ScrollableArea>> areas;
    areas.reserve(_scrollableAreas.size());
    for (auto *area : _scrollableAreas)
    {
      areas.push_back(CreateWeakPtr(area));
    }
    for (auto &area : areas)
    {
      if (area)
      {
        function(*area);
      }
    }
  }

  void LocalFrameView::InvalidateScrollbarsForAllScrollableAreas() noexcept
  {
    ForEachScrollableArea([](ScrollableArea &area) { area.ScrollbarAppearanceChanged(); });
  }

  OverscrollBehavior LocalFrameView::HorizontalOverscrollBehavior() const noexcept
  {
    auto *document = Frame().Document();
    auto *documentElement = document ? dom::TreeQueries::DocumentElement(*document) : nullptr;
    auto *scrollingObject = documentElement ? documentElement->Renderer() : nullptr;
    if (scrollingObject && GetRenderView())
    {
      return scrollingObject->Style().OverscrollBehaviorX();
    }
    return OverscrollBehavior::Auto;
  }

  OverscrollBehavior LocalFrameView::VerticalOverscrollBehavior() const noexcept
  {
    auto *document = Frame().Document();
    auto *documentElement = document ? dom::TreeQueries::DocumentElement(*document) : nullptr;
    auto *scrollingObject = documentElement ? documentElement->Renderer() : nullptr;
    if (scrollingObject && GetRenderView())
    {
      return scrollingObject->Style().OverscrollBehaviorY();
    }
    return OverscrollBehavior::Auto;
  }

  void LocalFrameView::ApplyOverflowToViewport(const RenderElement &renderer, ScrollbarMode &hMode,
                                               ScrollbarMode &vMode) const noexcept
  {
    // Handle the overflow:hidden/scroll case for the body/html elements.  WinIE treats overflow:hidden and
    // overflow:scroll on <body> as applying to the document's scrollbars.  The CSS2.1 draft states that HTML
    // UAs should use the <html> or <body> element and XML/XHTML UAs should use the root element.

    // To combat the inability to scroll on a page with overflow:hidden on the root when scaled, disregard
    // hidden when there is a frameScaleFactor that is greater than one on the main frame.
    bool overrideHidden = Frame().FrameScaleFactor() > 1;

    Overflow overflowX = renderer.Style().OverflowX();
    Overflow overflowY = renderer.Style().OverflowY();

    switch (overflowX)
    {
      case Overflow::Hidden:
      case Overflow::Clip:   hMode = overrideHidden ? ScrollbarMode::Auto : ScrollbarMode::AlwaysOff; break;
      case Overflow::Scroll: hMode = ScrollbarMode::AlwaysOn; break;
      case Overflow::Auto:   hMode = ScrollbarMode::Auto; break;
      default:
        // Don't set it at all.
        break;
    }

    switch (overflowY)
    {
      case Overflow::Hidden:
      case Overflow::Clip:   vMode = overrideHidden ? ScrollbarMode::Auto : ScrollbarMode::AlwaysOff; break;
      case Overflow::Scroll: vMode = ScrollbarMode::AlwaysOn; break;
      case Overflow::Auto:   vMode = ScrollbarMode::Auto; break;
      default:               break;
    }
  }

  void LocalFrameView::CalculateScrollbarModesForLayout(
    ScrollbarMode &hMode, ScrollbarMode &vMode, ScrollbarModesCalculationStrategy strategy) const noexcept
  {
    // FIXME: The viewport renderer type.
    if (_canHaveScrollbars || strategy == ScrollbarModesCalculationStrategy::RulesFromWebContentOnly)
    {
      hMode = ScrollbarMode::Auto;
      vMode = ScrollbarMode::Auto;
    }
    else
    {
      hMode = ScrollbarMode::AlwaysOff;
      vMode = ScrollbarMode::AlwaysOff;
    }

    if (_layoutContext.SubtreeLayoutRoot())
    {
      return;
    }

    auto *document = Frame().Document();
    if (!document)
    {
      return;
    }

    auto *documentElement = dom::TreeQueries::DocumentElement(*document);
    if (!documentElement)
    {
      return;
    }

    auto *bodyOrFrameset = document->BodyOrFrameset();
    auto *rootRenderer = documentElement->Renderer();
    if (!bodyOrFrameset || !bodyOrFrameset->Renderer())
    {
      if (rootRenderer)
      {
        ApplyOverflowToViewport(*rootRenderer, hMode, vMode);
      }
      return;
    }

    if (Is<html::HTMLBodyElement>(*bodyOrFrameset) && rootRenderer)
    {
      // It's sufficient to just check the X overflow, since it's illegal to have visible in only one
      // direction.
      if (rootRenderer->EffectiveOverflowX() == Overflow::Visible
          && Is<html::HTMLHtmlElement>(*documentElement))
      {
        if (auto *bodyRenderer = bodyOrFrameset->Renderer())
        {
          ApplyOverflowToViewport(*bodyRenderer, hMode, vMode);
        }
      }
      else
      {
        ApplyOverflowToViewport(*rootRenderer, hMode, vMode);
      }
    }
  }

  void LocalFrameView::AdjustScrollbarsForLayout(bool isFirstLayout) noexcept
  {
    ScrollbarMode hMode;
    ScrollbarMode vMode;
    CalculateScrollbarModesForLayout(hMode, vMode);

    // The first layout starts with a vertical scrollbar and no horizontal one, as most pages are taller than
    // they are wide: it's made while the scrollbars are suppressed, then kept as the real modes are set.
    if (isFirstLayout && !_layoutContext.IsLayoutNested())
    {
      SetScrollbarsSuppressed(true);
      if (vMode == ScrollbarMode::Auto)
      {
        SetVerticalScrollbarMode(ScrollbarMode::AlwaysOn);
      }
      if (hMode == ScrollbarMode::Auto)
      {
        SetHorizontalScrollbarMode(ScrollbarMode::AlwaysOff);
      }
      SetScrollbarModes(hMode, vMode);
      SetScrollbarsSuppressed(false, true);
    }
    else if (hMode != HorizontalScrollbarMode() || vMode != VerticalScrollbarMode())
    {
      SetScrollbarModes(hMode, vMode);
    }
  }

  void LocalFrameView::SetScrollbarsSuppressed(bool suppressed, bool repaintOnUnsuppress) noexcept
  {
    if (suppressed == _scrollbarsSuppressed)
    {
      return;
    }

    _scrollbarsSuppressed = suppressed;

    if (repaintOnUnsuppress && !suppressed)
    {
      if (_horizontalScrollbar)
      {
        _horizontalScrollbar->Invalidate();
      }
      if (_verticalScrollbar)
      {
        _verticalScrollbar->Invalidate();
      }

      // Invalidate the scroll corner too on unsuppress.
      InvalidateRect(ScrollCornerRect());
    }
  }

  void LocalFrameView::RestoreScrollbar() noexcept
  {
    SetScrollbarsSuppressed(false);
  }

  void LocalFrameView::Clear() noexcept
  {
    // reset(), of what the view keeps.
    _lastViewportSize = IntSize();
    _lastUsedZoomFactor = 1;
    _layoutContext.ResetFirstLayoutFlag();
    _layoutContext.SetNeedsFullRepaint();

    SetScrollbarsSuppressed(true);
  }

  void LocalFrameView::WillDoLayout(const RenderElement &layoutRoot) noexcept
  {
    bool subtreeLayout = !Is<RenderView>(layoutRoot);
    if (subtreeLayout)
    {
      return;
    }

    auto firstLayout = !_layoutContext.DidFirstLayout();
    if (firstLayout)
    {
      _lastViewportSize = SizeForResizeEvent();
      _lastUsedZoomFactor = layoutRoot.Style().UsedZoom();
      _lastFrameScaleFactor = Frame().FrameScaleFactor();
    }
    AdjustScrollbarsForLayout(firstLayout);

    auto oldSize = _lastUsedSizeForLayout;
    auto newSize = GetLayoutSize();
    if (oldSize != newSize)
    {
      _lastUsedSizeForLayout = newSize;
      _layoutContext.SetNeedsFullRepaint();
      if (!firstLayout)
      {
        MarkRootOrBodyRendererDirty();
      }
    }
  }

  void LocalFrameView::MarkRootOrBodyRendererDirty() const noexcept
  {
    auto &document = *Frame().Document();
    auto *documentElement = dom::TreeQueries::DocumentElement(document);
    auto *rootRenderer = documentElement ? DynamicDowncast<RenderBox>(documentElement->Renderer()) : nullptr;
    auto *body = document.BodyOrFrameset();
    auto *bodyRenderer = rootRenderer && body ? DynamicDowncast<RenderBox>(body->Renderer()) : nullptr;
    if (bodyRenderer && bodyRenderer->StretchesToViewport())
    {
      bodyRenderer->SetChildNeedsLayout();
    }
    else if (rootRenderer && rootRenderer->StretchesToViewport())
    {
      rootRenderer->SetChildNeedsLayout();
    }
  }

  bool LocalFrameView::LogicalScroll(ScrollLogicalDirection direction, ScrollGranularity granularity,
                                     ScrollIsAnimated animated) noexcept
  {
    return Scroll(LogicalToPhysical(direction, IsVerticalDocument(), IsFlippedDocument()), granularity, 1,
                  animated);
  }

  bool LocalFrameView::IsVerticalDocument() const noexcept
  {
    const RenderView *renderView = GetRenderView();
    if (!renderView)
    {
      return true;
    }

    return renderView->GetWritingMode().IsHorizontal();
  }

  bool LocalFrameView::IsFlippedDocument() const noexcept
  {
    const RenderView *renderView = GetRenderView();
    if (!renderView)
    {
      return false;
    }

    return renderView->GetWritingMode().IsBlockFlipped();
  }

  bool LocalFrameView::IsScrollable() const noexcept
  {
    // Check for:
    // 1) If there an actual overflow.
    // 2) display:none or visibility:hidden set to self or inherited.
    // 3) overflow{-x,-y}: hidden;
    // 4) scrolling: no;
    if (!_layoutContext.DidFirstLayout())
    {
      return false;
    }

    // Covers #1
    IntSize totalContentsSize = TotalContentsSize();
    IntSize visibleContentSize = VisibleContentRect().Size();
    if (totalContentsSize.Height() <= visibleContentSize.Height()
        && totalContentsSize.Width() <= visibleContentSize.Width())
    {
      return false;
    }

    // Cover #3 and #4.
    ScrollbarMode horizontalMode;
    ScrollbarMode verticalMode;
    CalculateScrollbarModesForLayout(horizontalMode, verticalMode,
                                     ScrollbarModesCalculationStrategy::RulesFromWebContentOnly);
    return horizontalMode != ScrollbarMode::AlwaysOff || verticalMode != ScrollbarMode::AlwaysOff;
  }

  void LocalFrameView::PaintContents(GraphicsContext &context, const IntRect &dirtyRect) noexcept
  {
    auto *renderView = const_cast<RenderView *>(GetRenderView());
    if (!renderView)
    {
      return;
    }

    if (!LayoutContext().InPaintableState())
    {
      return;
    }

    krys_debug_assert(!LayoutContext().NeedsLayout());
    if (LayoutContext().NeedsLayout())
    {
      return;
    }

    krys_debug_assert(!_isPainting);
    _isPainting = true;
    auto paintBehavior = _paintBehavior;

    // _nodeToDraw is used to draw only one element (and its descendants)
    RenderObject *renderer = _nodeToDraw ? _nodeToDraw->Renderer() : nullptr;
    RenderLayer *rootLayer = renderView->Layer();
    rootLayer->Paint(context, LayoutRect(dirtyRect), LayoutSize(), _paintBehavior, renderer);
    if (auto *scrollableRootLayer = rootLayer->GetScrollableArea();
        scrollableRootLayer && scrollableRootLayer->ContainsDirtyOverlayScrollbars())
    {
      scrollableRootLayer->PaintOverlayScrollbars(context, LayoutRect(dirtyRect), _paintBehavior, renderer);
    }

    _paintBehavior = paintBehavior;
    _isPainting = false;
  }

  void LocalFrameView::SetNodeToDraw(dom::Node *node) noexcept
  {
    _nodeToDraw = ShareRefPtr(node);
  }

  // Booey has one frame: its selection is the only one to clear and restore.
  void LocalFrameView::PaintContentsForSnapshot(GraphicsContext &context, const IntRect &imageRect,
                                                SelectionInSnapshot shouldPaintSelection,
                                                CoordinateSpaceForSnapshot coordinateSpace) noexcept
  {
    auto *document = Frame().Document();
    if (!document)
    {
      return;
    }
    (void)document->UpdateLayout();

    // Cache paint behavior and set a new behavior appropriate for snapshots.
    auto oldBehavior = GetPaintBehavior();
    auto snapshotBehavior = oldBehavior;
    snapshotBehavior.Add({PaintBehavior::FlattenCompositingLayers, PaintBehavior::Snapshotting});
    SetPaintBehavior(snapshotBehavior);

    // If the snapshot should exclude selection, then we'll clear the current selection
    // in the render tree only. This will allow us to restore the selection from the DOM
    // after we paint the snapshot.
    if (shouldPaintSelection == ExcludeSelection)
    {
      if (auto *renderView = document->GetRenderView())
      {
        renderView->Selection().Clear();
      }
    }

    if (coordinateSpace == DocumentCoordinates)
    {
      PaintContents(context, imageRect);
    }
    else
    {
      // A snapshot in ViewCoordinates will include a scrollbar, and the snapshot will contain
      // whatever content the document is currently scrolled to.
      Paint(context, imageRect);
    }

    // Restore selection.
    if (shouldPaintSelection == ExcludeSelection)
    {
      document->Selection().UpdateAppearance();
    }

    // Restore cached paint behavior.
    SetPaintBehavior(oldBehavior);
  }

  void LocalFrameView::AvailableContentSizeChanged(AvailableSizeChangeReason reason) noexcept
  {
    if (auto *document = Frame().Document())
    {
      // FIXME: Find a more appropriate way of handling potential recursive layouts when the viewport is
      // resized to accommodate the content but the content always overflows the viewport. See
      // webkit.org/b/165781.
      if (_layoutContext.GetLayoutPhase() != LocalFrameViewLayoutContext::LayoutPhase::InViewSizeAdjust
          || !UseFixedLayout())
      {
        document->UpdateViewportUnitsOnResize();
      }
    }

    // FIXME: Update the layout viewport, once there's a visual viewport.
    _layoutContext.SetNeedsLayoutAfterViewConfigurationChange();

    // ScrollView::availableContentSizeChanged.
    ScrollableArea::AvailableContentSizeChanged(reason);
    if (reason != AvailableSizeChangeReason::ScrollbarsChanged)
    {
      UpdateScrollbars(GetScrollPosition());
    }
  }

  void LocalFrameView::SetMediaType(const css::CSSOMStringAtom &mediaType) noexcept
  {
    if (_mediaType == mediaType)
    {
      return;
    }

    _mediaType = mediaType;

    if (auto *document = Frame().Document())
    {
      document->StyleScope().DidChangeStyleSheetEnvironment();
    }
  }

  void LocalFrameView::ClearSizeOverrideForCSSDefaultViewportUnits() noexcept
  {
    if (!_defaultViewportSizeOverride)
    {
      return;
    }

    _defaultViewportSizeOverride = null;
    if (auto *document = Frame().Document())
    {
      document->StyleScope().DidChangeStyleSheetEnvironment();
    }
  }

  void LocalFrameView::SetSizeForCSSDefaultViewportUnits(FloatSize size) noexcept
  {
    SetOverrideSizeForCSSDefaultViewportUnits({size.Width(), size.Height()});
  }

  void LocalFrameView::SetOverrideSizeForCSSDefaultViewportUnits(OverrideViewportSize size) noexcept
  {
    if (_defaultViewportSizeOverride == size)
    {
      return;
    }

    _defaultViewportSizeOverride = size;

    if (auto *document = Frame().Document())
    {
      document->StyleScope().DidChangeStyleSheetEnvironment();
    }
  }

  FloatSize LocalFrameView::SizeForCSSDefaultViewportUnits() const noexcept
  {
    return CalculateSizeForCSSViewportUnitsOverride(_defaultViewportSizeOverride);
  }

  void LocalFrameView::ClearSizeOverrideForCSSSmallViewportUnits() noexcept
  {
    if (!_smallViewportSizeOverride)
    {
      return;
    }

    _smallViewportSizeOverride = null;
    if (auto *document = Frame().Document())
    {
      document->UpdateViewportUnitsOnResize();
    }
  }

  void LocalFrameView::SetSizeForCSSSmallViewportUnits(FloatSize size) noexcept
  {
    SetOverrideSizeForCSSSmallViewportUnits({size.Width(), size.Height()});
  }

  void LocalFrameView::SetOverrideSizeForCSSSmallViewportUnits(OverrideViewportSize size) noexcept
  {
    if (_smallViewportSizeOverride && *_smallViewportSizeOverride == size)
    {
      return;
    }

    _smallViewportSizeOverride = size;

    if (auto *document = Frame().Document())
    {
      document->UpdateViewportUnitsOnResize();
    }
  }

  FloatSize LocalFrameView::SizeForCSSSmallViewportUnits() const noexcept
  {
    return CalculateSizeForCSSViewportUnitsOverride(_smallViewportSizeOverride);
  }

  void LocalFrameView::ClearSizeOverrideForCSSLargeViewportUnits() noexcept
  {
    if (!_largeViewportSizeOverride)
    {
      return;
    }

    _largeViewportSizeOverride = null;
    if (auto *document = Frame().Document())
    {
      document->UpdateViewportUnitsOnResize();
    }
  }

  void LocalFrameView::SetSizeForCSSLargeViewportUnits(FloatSize size) noexcept
  {
    SetOverrideSizeForCSSLargeViewportUnits({size.Width(), size.Height()});
  }

  void LocalFrameView::SetOverrideSizeForCSSLargeViewportUnits(OverrideViewportSize size) noexcept
  {
    if (_largeViewportSizeOverride && *_largeViewportSizeOverride == size)
    {
      return;
    }

    _largeViewportSizeOverride = size;

    if (auto *document = Frame().Document())
    {
      document->UpdateViewportUnitsOnResize();
    }
  }

  FloatSize LocalFrameView::SizeForCSSLargeViewportUnits() const noexcept
  {
    return CalculateSizeForCSSViewportUnitsOverride(_largeViewportSizeOverride);
  }

  FloatSize LocalFrameView::CalculateSizeForCSSViewportUnitsOverride(
    Maybe<OverrideViewportSize> override) const noexcept
  {
    OverrideViewportSize viewportSize;

    if (override)
    {
      viewportSize = *override;

      // auto-size overrides the width only, so we can't always bail out early here.
      if (viewportSize.Width && viewportSize.Height)
      {
        return {*viewportSize.Width, *viewportSize.Height};
      }
    }

    if (UseFixedLayout())
    {
      auto fixedLayoutSize = FixedLayoutSize();
      viewportSize.Width = viewportSize.Width.value_or(static_cast<float>(fixedLayoutSize.Width()));
      viewportSize.Height = viewportSize.Height.value_or(static_cast<float>(fixedLayoutSize.Height()));
      return {*viewportSize.Width, *viewportSize.Height};
    }

    // FIXME: the value returned should take into account the value of the overflow
    // property on the root element.
    auto visibleContentSizeIncludingScrollbars = VisibleContentRect().Size();
    viewportSize.Width =
      viewportSize.Width.value_or(static_cast<float>(visibleContentSizeIncludingScrollbars.Width()));
    viewportSize.Height =
      viewportSize.Height.value_or(static_cast<float>(visibleContentSizeIncludingScrollbars.Height()));
    return {*viewportSize.Width, *viewportSize.Height};
  }

  FloatSize LocalFrameView::SizeForCSSDynamicViewportUnits() const noexcept
  {
    // FIXME: The layout viewport override rect comes first, once there's a layout viewport.

    if (UseFixedLayout())
    {
      return FloatSize(FixedLayoutSize());
    }

    return FloatSize(UnobscuredContentRect().Size());
  }

  namespace
  {
    void AdjustScrollRectToVisibleOptionsForHiddenOverflow(ScrollRectToVisibleOptions &options,
                                                           const RenderStyle &style) noexcept
    {
      if (options.AllowOverflowHidden == AllowScrollingOverflowHidden::Yes)
      {
        return;
      }
      if (style.OverflowX() == Overflow::Hidden)
      {
        options.AlignX = ScrollAlignment::NoScroll;
      }
      if (style.OverflowY() == Overflow::Hidden)
      {
        options.AlignY = ScrollAlignment::NoScroll;
      }
    }

    void AdjustForScrollByAnchor(const RenderLayerModelObject &renderer, EnumSet<BoxAxis> &isFixed) noexcept
    {
      if (!renderer.IsFixedPositioned())
      {
        return;
      }
      const auto *renderBox = DynamicDowncast<RenderBox>(&renderer);
      krys_debug_assert(renderBox);

      if (!renderBox || !style::AnchorPositionEvaluator::IsAnchorPositioned(renderBox->Style()))
      {
        return;
      }
      const auto *adjuster =
        renderBox->View().FrameView().LayoutContext().AnchorScrollAdjusterFor(*renderBox);
      if (!adjuster || !adjuster->HasViewportSnapshot())
      {
        return;
      }

      if (adjuster->MayNeedXAdjustment())
      {
        isFixed.Remove(BoxAxis::Horizontal);
      }
      if (adjuster->MayNeedYAdjustment())
      {
        isFixed.Remove(BoxAxis::Vertical);
      }
    }
  }

  bool LocalFrameView::ScrollRectToVisible(const LayoutRect &absoluteRect, const RenderObject &renderer,
                                           bool insideFixed,
                                           const ScrollRectToVisibleOptions &options) noexcept
  {
    if (options.RevealMode == SelectionRevealMode::DoNotReveal)
    {
      return false;
    }

    if (renderer.IsSkippedContent())
    {
      return false;
    }

    auto *layer = renderer.EnclosingLayer();
    if (!layer)
    {
      return false;
    }

    // FIXME: It would be nice to use RenderLayer::enclosingScrollableLayer here, but that seems to skip
    // overflow:hidden layers.
    auto adjustedRect = absoluteRect;
    auto adjustedOptions = options;
    EnumSet<BoxAxis> isFixed =
      insideFixed ? EnumSet<BoxAxis> {BoxAxis::Horizontal, BoxAxis::Vertical} : EnumSet<BoxAxis> {};

    for (; layer; layer = layer->EnclosingContainingBlockLayer(CrossFrameBoundaries::No))
    {
      if (layer->ShouldTryToScrollForScrollIntoView(adjustedOptions))
      {
        AdjustScrollRectToVisibleOptionsForHiddenOverflow(adjustedOptions, layer->Renderer().Style());
        adjustedRect = layer->EnsureLayerScrollableArea()->ScrollRectToVisible(adjustedRect, adjustedOptions);
        if (adjustedOptions.VisibilityCheckRect)
        {
          adjustedOptions.VisibilityCheckRect->SetLocation(adjustedRect.Location());
        }
      }
      // FIXME: Make scroll adjustments work for more than just fixedpos elements.
      if (insideFixed)
      {
        AdjustForScrollByAnchor(layer->Renderer(), isFixed);
      }
    }

    renderer.View().FrameView().ScrollRectToVisibleInTopLevelView(adjustedRect, isFixed, adjustedOptions);
    return true;
  }

  void LocalFrameView::ScrollRectToVisibleInTopLevelView(const LayoutRect &absoluteRect,
                                                         EnumSet<BoxAxis> isFixed,
                                                         const ScrollRectToVisibleOptions &options) noexcept
  {
    // FIXME: Delegating the scroll to the client (DelegateMainFrameScroll), once there's a client.
    if (options.RevealMode == SelectionRevealMode::RevealUpToMainFrame)
    {
      return;
    }

    auto minScrollPosition = MinimumScrollPosition();
    auto maxScrollPosition = MaximumScrollPosition();

    auto viewRect = LayoutRect(VisibleContentRect());

    // Move the target rect into "scrollView contents" coordinates.
    auto targetRect = absoluteRect;
    targetRect.Move(0, HeaderHeight());

    // scroll-padding applies to the scroll container, but expand the rectangle that we want to expose in
    // order simulate padding the scroll container. This rectangle is passed up the tree of scrolling elements
    // to ensure that the padding on this scroll container is maintained.
    auto *document = Frame().Document();
    auto *element = document ? dom::TreeQueries::DocumentElement(*document) : nullptr;
    if (auto *renderBox = element ? DynamicDowncast<RenderBox>(element->Renderer()) : nullptr)
    {
      targetRect.Expand(renderBox->ScrollPaddingForViewportRect(viewRect));
    }

    LayoutRect revealRect =
      GetPossiblyFixedRectToExpose(viewRect, targetRect, isFixed, options.AlignX, options.AlignY);

    // Avoid scrolling to the rounded value of revealRect.location() if we don't actually need to scroll
    // (Only scrolling forward, OnlyAllowForwardScrolling, is for find in page, which there isn't; see
    // DIFFERENCES.md.)
    if (revealRect != viewRect)
    {
      ScrollOffset clampedScrollPosition =
        RoundedIntPoint(revealRect.Location()).ConstrainedBetween(minScrollPosition, maxScrollPosition);
      auto scrollPositionOptions = ScrollPositionChangeOptions::CreateProgrammatic();
      if (Frame().GetEventHandler().IsAutoscrolling())
      {
        // An autoscroll moves bit by bit: it snaps when it ends.
        scrollPositionOptions.SnapPointSelectionMethod = ScrollSnapPointSelectionMethod::None;
      }
      else if (element && UseSmoothScrolling(options.Behavior, element))
      {
        scrollPositionOptions.Animated = ScrollIsAnimated::Yes;
      }
      SetScrollPosition(clampedScrollPosition, scrollPositionOptions);
    }

    // FIXME: Scrolling the client's containing scroll views (scrollContainingScrollViewsToRevealRect).
  }

  LayoutRect LocalFrameView::GetPossiblyFixedRectToExpose(const LayoutRect &visibleRect,
                                                          const LayoutRect &exposeRect,
                                                          EnumSet<BoxAxis> isFixed,
                                                          const ScrollAlignment &alignX,
                                                          const ScrollAlignment &alignY) const noexcept
  {
    if (!isFixed.ContainsAny({BoxAxis::Horizontal, BoxAxis::Vertical}))
    {
      return GetPossiblyFixedRectToExpose(visibleRect, exposeRect, false, alignX, alignY);
    }
    if (isFixed.ContainsAll({BoxAxis::Horizontal, BoxAxis::Vertical}))
    {
      return GetPossiblyFixedRectToExpose(visibleRect, exposeRect, true, alignX, alignY);
    }

    auto fixedRect = GetPossiblyFixedRectToExpose(visibleRect, exposeRect, true, alignX, alignY);
    auto scrollRect = GetPossiblyFixedRectToExpose(visibleRect, exposeRect, false, alignX, alignY);
    if (isFixed.Contains(BoxAxis::Horizontal))
    {
      return {fixedRect.X(), scrollRect.Y(), fixedRect.Width(), scrollRect.Height()};
    }
    return {scrollRect.X(), fixedRect.Y(), scrollRect.Width(), fixedRect.Height()};
  }

  LayoutRect LocalFrameView::GetPossiblyFixedRectToExpose(const LayoutRect &visibleRect,
                                                          const LayoutRect &exposeRect, bool isFixed,
                                                          const ScrollAlignment &alignX,
                                                          const ScrollAlignment &alignY) const noexcept
  {
    if (!isFixed)
    {
      return GetRectToExposeForScrollIntoView(visibleRect, exposeRect, alignX, alignY);
    }

    // If the element is inside position:fixed and we're not scaled, no amount of scrolling is going to move
    // things around.
    if (FrameScaleFactor() == 1)
    {
      return visibleRect;
    }

    // FIXME: Shouldn't this return visibleRect as well?
    // FIXME: The visual viewport's case (visualViewportEnabled), once there are visual viewports.
    return GetRectToExposeForScrollIntoView(visibleRect, exposeRect, alignX, alignY);
  }

  ScrollPosition LocalFrameView::MaximumScrollPosition() const noexcept
  {
    ScrollPosition maximumPosition = ScrollableArea::MaximumScrollPosition();
    // FIXME: can this be moved into the base class?
    maximumPosition.ClampNegativeToZero();
    return maximumPosition;
  }

  ScrollPosition
    LocalFrameView::AdjustScrollPositionWithinRange(const ScrollPosition &scrollPosition) const noexcept
  {
    if (GetScrollClamping() == ScrollClamping::Unclamped)
    {
      return scrollPosition;
    }

    return scrollPosition.ConstrainedBetween(MinimumScrollPosition(), MaximumScrollPosition());
  }

  void LocalFrameView::SetContentsSize(const IntSize &newSize) noexcept
  {
    if (ContentsSize() == newSize)
    {
      return;
    }
    _contentsSize = newSize;
    UpdateScrollbars(GetScrollPosition());
  }

  void LocalFrameView::AdjustViewSize() noexcept
  {
    auto *renderView = GetRenderView();
    if (!renderView)
    {
      return;
    }

    const IntRect rect = renderView->DocumentRect();
    const IntSize &size = rect.Size();

    // A new scroll origin moves the scroll position at once if the contents size isn't changing too.
    IntPoint scrollOrigin(-rect.X(), -rect.Y());
    if (scrollOrigin != ScrollOrigin())
    {
      SetScrollOrigin(scrollOrigin);
      if (size == ContentsSize())
      {
        UpdateScrollbars(GetScrollPosition());
      }
    }

    SetContentsSize(size);
  }

  void LocalFrameView::SetScrollPosition(const ScrollPosition &scrollPosition,
                                         const ScrollPositionChangeOptions &options) noexcept
  {
    SetScrollOffsetWithOptions(ScrollOffsetFromPosition(scrollPosition), options);
  }

  void LocalFrameView::SetScrollOffsetWithOptions(Maybe<int32> x, Maybe<int32> y,
                                                  const ScrollPositionChangeOptions &options) noexcept
  {
    if (x && y)
    {
      SetScrollOffsetWithOptions(ScrollOffset(*x, *y), options);
      return;
    }

    auto offset = ScrollOffsetFromPosition(GetScrollPosition());

    if (x)
    {
      offset.SetX(*x);
    }

    if (y)
    {
      offset.SetY(*y);
    }

    SetScrollOffsetWithOptions(offset, options);
  }

  void LocalFrameView::SetScrollOffsetWithOptions(const ScrollOffset &scrollOffset,
                                                  const ScrollPositionChangeOptions &options) noexcept
  {
    auto oldScrollType = CurrentScrollType();
    SetCurrentScrollType(options.Type);

    _maintainScrollPositionAnchor = nullptr;
    CancelScheduledScrolls();

    auto position = ScrollPositionFromOffset(
      ScrollOffsetAdjustedForSnapping(scrollOffset, options.SnapPointSelectionMethod));

    if (options.Animated == ScrollIsAnimated::Yes)
    {
      ScrollToPositionWithAnimation(position, options);
      SetCurrentScrollType(oldScrollType);
      return;
    }

    // ScrollView::setScrollPosition.
    // FIXME: Prohibiting scrolling, once something can.
    bool stoppedAnimation = StopAnimatedScroll();
    auto newScrollPosition =
      options.Clamping == ScrollClamping::Clamped ? AdjustScrollPositionWithinRange(position) : position;
    bool scrollPositionChanged = newScrollPosition != GetScrollPosition();

    if (CurrentScrollType() != ScrollType::User || scrollPositionChanged)
    {
      if (!RequestScrollToPosition(newScrollPosition, options))
      {
        UpdateScrollbars(newScrollPosition);
      }
    }
    if (stoppedAnimation && !scrollPositionChanged)
    {
      ScrollDidEnd();
    }

    SetCurrentScrollType(oldScrollType);
  }

  void LocalFrameView::UpdateScrollbars(const ScrollPosition &desiredPosition) noexcept
  {
    // FIXME: Prohibiting scrolling, once something can.
    if (_inUpdateScrollbars)
    {
      return;
    }

    auto scrollToPosition = [&](ScrollPosition position)
    {
      auto adjustedScrollPosition = AdjustScrollPositionWithinRange(position);
      if (adjustedScrollPosition != GetScrollPosition() || ScrollOriginChanged())
      {
        (void)ScrollToPositionWithoutAnimation(adjustedScrollPosition);
        ResetScrollOriginChanged();
      }
    };

    bool scrollbarCanTakeSpace = CanShowNonOverlayScrollbars();

    // If we came in here with the view already needing a layout then do that first. (This will be the common
    // case, e.g., when the page changes due to window resizing for example). This layout will not re-enter
    // updateScrollbars and does not count towards our max layout pass total.
    if (!_scrollbarsSuppressed && scrollbarCanTakeSpace)
    {
      SetForScope inUpdateScrollbarsScope(_inUpdateScrollbars, true);
      UpdateContentsSize();
    }

    IntRect oldScrollCornerRect = ScrollCornerRect();

    bool hasHorizontalScrollbar = static_cast<bool>(_horizontalScrollbar);
    bool hasVerticalScrollbar = static_cast<bool>(_verticalScrollbar);

    bool newHasHorizontalScrollbar = hasHorizontalScrollbar;
    bool newHasVerticalScrollbar = hasVerticalScrollbar;

    ScrollbarMode hScroll = _horizontalScrollbarMode;
    ScrollbarMode vScroll = _verticalScrollbarMode;

    if (hScroll != ScrollbarMode::Auto)
    {
      newHasHorizontalScrollbar = hScroll == ScrollbarMode::AlwaysOn;
    }
    if (vScroll != ScrollbarMode::Auto)
    {
      newHasVerticalScrollbar = vScroll == ScrollbarMode::AlwaysOn;
    }

    bool scrollbarAddedOrRemoved = false;

    if (_scrollbarsSuppressed || (hScroll != ScrollbarMode::Auto && vScroll != ScrollbarMode::Auto))
    {
      if (hasHorizontalScrollbar != newHasHorizontalScrollbar
          && SetHasHorizontalScrollbar(newHasHorizontalScrollbar))
      {
        scrollbarAddedOrRemoved = true;
      }

      if (hasVerticalScrollbar != newHasVerticalScrollbar && SetHasVerticalScrollbar(newHasVerticalScrollbar))
      {
        scrollbarAddedOrRemoved = true;
      }
    }
    else
    {
      bool sendContentResizedNotification = false;

      IntSize docSize = TotalContentsSize();
      IntSize fullVisibleSize = UnobscuredContentRect(VisibleContentRectIncludesScrollbars::Yes).Size();

      if (hScroll == ScrollbarMode::Auto)
      {
        newHasHorizontalScrollbar = docSize.Width() > VisibleWidth();
      }
      if (vScroll == ScrollbarMode::Auto)
      {
        newHasVerticalScrollbar = docSize.Height() > VisibleHeight();
      }

      bool needAnotherPass = false;
      if (scrollbarCanTakeSpace)
      {
        // If we ever turn one scrollbar off, do not turn the other one on. Never ever try to both gain/lose a
        // scrollbar in the same pass.
        if (!_updateScrollbarsPass && docSize.Width() <= fullVisibleSize.Width()
            && docSize.Height() <= fullVisibleSize.Height())
        {
          if (hScroll == ScrollbarMode::Auto)
          {
            newHasHorizontalScrollbar = false;
          }
          if (vScroll == ScrollbarMode::Auto)
          {
            newHasVerticalScrollbar = false;
          }
        }
        if (!newHasHorizontalScrollbar && hasHorizontalScrollbar && vScroll != ScrollbarMode::AlwaysOn
            && !hasVerticalScrollbar)
        {
          newHasVerticalScrollbar = false;
          needAnotherPass = true;
        }
        if (!newHasVerticalScrollbar && hasVerticalScrollbar && hScroll != ScrollbarMode::AlwaysOn
            && !hasHorizontalScrollbar)
        {
          newHasHorizontalScrollbar = false;
          needAnotherPass = true;
        }
      }

      if (hasHorizontalScrollbar != newHasHorizontalScrollbar)
      {
        if (ScrollOrigin().Y() && !newHasHorizontalScrollbar)
        {
          SetScrollOrigin(
            IntPoint(ScrollOrigin().X(), ScrollOrigin().Y() - _horizontalScrollbar->OccupiedHeight()));
        }
        if (_horizontalScrollbar)
        {
          _horizontalScrollbar->Invalidate();
        }

        bool changeAffectsContentSize = false;
        if (SetHasHorizontalScrollbar(newHasHorizontalScrollbar, &changeAffectsContentSize))
        {
          scrollbarAddedOrRemoved = true;
          sendContentResizedNotification |= changeAffectsContentSize;
        }
      }

      if (hasVerticalScrollbar != newHasVerticalScrollbar)
      {
        if (ScrollOrigin().X() && !newHasVerticalScrollbar)
        {
          SetScrollOrigin(
            IntPoint(ScrollOrigin().X() - _verticalScrollbar->OccupiedWidth(), ScrollOrigin().Y()));
        }
        if (_verticalScrollbar)
        {
          _verticalScrollbar->Invalidate();
        }

        bool changeAffectsContentSize = false;
        if (SetHasVerticalScrollbar(newHasVerticalScrollbar, &changeAffectsContentSize))
        {
          scrollbarAddedOrRemoved = true;
          sendContentResizedNotification |= changeAffectsContentSize;
        }
      }

      constexpr uint32 maxUpdateScrollbarsPass = 3;
      if ((sendContentResizedNotification || needAnotherPass)
          && _updateScrollbarsPass < maxUpdateScrollbarsPass)
      {
        ++_updateScrollbarsPass;
        AvailableContentSizeChanged(AvailableSizeChangeReason::ScrollbarsChanged);
        UpdateContentsSize();
        IntSize newDocSize = TotalContentsSize();
        if (newDocSize == docSize)
        {
          // The layout with the new scroll state had no impact on the document's overall size, so
          // updateScrollbars didn't get called. Recur manually.
          UpdateScrollbars(desiredPosition);
        }
        --_updateScrollbarsPass;
      }
    }

    if (scrollbarAddedOrRemoved)
    {
      AddedOrRemovedScrollbar();
    }

    // Set up the range (and page step/line step), but only do this if we're not in a nested call (to avoid
    // doing it multiple times).
    if (_updateScrollbarsPass)
    {
      return;
    }

    SetForScope inUpdateScrollbarsScope(_inUpdateScrollbars, true);

    if (_horizontalScrollbar)
    {
      int32 clientWidth = VisibleWidth();
      IntRect oldRect(_horizontalScrollbar->FrameRect());
      _horizontalScrollbar->SetFrameRect(IntRect(
        ShouldPlaceVerticalScrollbarOnLeft() && _verticalScrollbar ? _verticalScrollbar->OccupiedWidth() : 0,
        Height() - _horizontalScrollbar->Height(),
        Width() - (_verticalScrollbar ? _verticalScrollbar->OccupiedWidth() : 0),
        _horizontalScrollbar->Height()));
      if (!_scrollbarsSuppressed && oldRect != _horizontalScrollbar->FrameRect())
      {
        _horizontalScrollbar->Invalidate();
      }

      if (_scrollbarsSuppressed)
      {
        _horizontalScrollbar->SetSuppressInvalidation(true);
      }
      _horizontalScrollbar->SetEnabled(ContentsSize().Width() > clientWidth);
      _horizontalScrollbar->SetProportion(clientWidth, ContentsSize().Width());
      if (_scrollbarsSuppressed)
      {
        _horizontalScrollbar->SetSuppressInvalidation(false);
      }
    }

    if (_verticalScrollbar)
    {
      int32 clientHeight = VisibleHeight();
      IntRect oldRect(_verticalScrollbar->FrameRect());
      _verticalScrollbar->SetFrameRect(
        IntRect(ShouldPlaceVerticalScrollbarOnLeft() ? 0 : Width() - _verticalScrollbar->Width(), 0,
                _verticalScrollbar->Width(),
                Height() - (_horizontalScrollbar ? _horizontalScrollbar->OccupiedHeight() : 0)));
      if (!_scrollbarsSuppressed && oldRect != _verticalScrollbar->FrameRect())
      {
        _verticalScrollbar->Invalidate();
      }

      if (_scrollbarsSuppressed)
      {
        _verticalScrollbar->SetSuppressInvalidation(true);
      }
      _verticalScrollbar->SetEnabled(TotalContentsSize().Height() > clientHeight);
      _verticalScrollbar->SetProportion(clientHeight, TotalContentsSize().Height());
      if (_scrollbarsSuppressed)
      {
        _verticalScrollbar->SetSuppressInvalidation(false);
      }
    }

    UpdateScrollbarSteps();

    PositionScrollbarLayers();
    if (hasHorizontalScrollbar != newHasHorizontalScrollbar
        || hasVerticalScrollbar != newHasVerticalScrollbar)
    {
      // The corner the scrollbars left goes with them. (There are no custom scroll corners to update.)
      if (!_horizontalScrollbar && !_verticalScrollbar)
      {
        InvalidateScrollCorner(oldScrollCornerRect);
      }
    }

    scrollToPosition(desiredPosition);

    // Make sure the scrollbar offsets are up to date.
    if (_horizontalScrollbar)
    {
      _horizontalScrollbar->OffsetDidChange();
    }
    if (_verticalScrollbar)
    {
      _verticalScrollbar->OffsetDidChange();
    }
  }

  void LocalFrameView::SetScrollOffset(const ScrollOffset &offset) noexcept
  {
    auto constrainedOffset = offset;
    if (GetScrollClamping() == ScrollClamping::Clamped)
    {
      constrainedOffset = constrainedOffset.ConstrainedBetween(MinimumScrollOffset(), MaximumScrollOffset());
    }

    ScrollTo(ScrollPositionFromOffset(constrainedOffset));
  }

  void LocalFrameView::ScrollTo(const ScrollPosition &newPosition) noexcept
  {
    IntPoint oldPosition = GetScrollPosition();
    IntSize scrollDelta = newPosition - _scrollPosition;
    if (scrollDelta.IsZero())
    {
      return;
    }

    _scrollPosition = newPosition;

    // A view that's loading repaints all of itself; what's fixed moves with its next layout.
    // FIXME: Deferring the update while the contents size changes.
    if (_scrollbarsSuppressed)
    {
      if (oldPosition != GetScrollPosition())
      {
        ScrollPositionChanged(oldPosition, GetScrollPosition());
      }
      return;
    }

    // completeUpdatesAfterScrollTo: the fixed layers move with the scroll position, then the painted contents
    // move.
    if (!LayoutContext().IsInRenderTreeLayout())
    {
      if (auto *renderView = const_cast<RenderView *>(GetRenderView()); renderView && renderView->Layer())
      {
        renderView->Layer()->UpdateLayerPositionsAfterDocumentScroll();
      }
    }
    ScrollContents(scrollDelta);
    UpdateCompositingLayersAfterScrolling();

    if (oldPosition != GetScrollPosition())
    {
      ScrollPositionChanged(oldPosition, GetScrollPosition());
    }

    // FIXME: Telling the page overlays, the inspector and the client (didChangeScrollOffset), once there are
    // those.
  }

  void LocalFrameView::ScrollPositionChanged(const ScrollPosition &, const ScrollPosition &) noexcept
  {
    ScheduleScrollEvent();

    if (auto *renderView = GetRenderViewForUpdate())
    {
      if (renderView->UsesCompositing())
      {
        renderView->Compositor().FrameViewDidScroll();
      }
    }

    // FIXME: The layout viewport (updateLayoutViewport) and the editor, once there are those.
    LayoutOrVisualViewportChanged();
    ViewportContentsChanged();
  }

  void LocalFrameView::ViewportContentsChanged() noexcept
  {
    if (!Frame().View())
    {
      // The frame is being destroyed.
      return;
    }

    // When the viewport contents changes (scroll, resize, style recalc, layout, ...),
    // check if we should resume animated images.
    ResumeVisibleImageAnimations(VisibleContentRect());
  }

  void LocalFrameView::ResumeVisibleImageAnimations() noexcept
  {
    ResumeVisibleImageAnimations(VisibleContentRect());
  }

  void LocalFrameView::ResumeVisibleImageAnimations(const IntRect &visibleRect) noexcept
  {
    if (visibleRect.IsEmpty())
    {
      return;
    }

    auto *document = Frame().Document();
    if (auto *renderView = document ? document->GetRenderView() : nullptr)
    {
      renderView->ResumePausedImageAnimationsIfNeeded(visibleRect);
    }
  }

  void LocalFrameView::SetLastUserScrollType(Maybe<UserScrollType> userScrollType) noexcept
  {
    CancelScheduledScrolls();
    if (CurrentScrollType() == ScrollType::Programmatic)
    {
      return;
    }

    RefPtr<dom::Document> document = ShareRefPtr(Frame().Document());
    if (userScrollType && document)
    {
      document->SetGotoAnchorNeededAfterStylesheetsLoad(false);
    }

    _maintainScrollPositionAnchor = nullptr;
    if (_lastUserScrollType == userScrollType)
    {
      return;
    }
    _lastUserScrollType = userScrollType;

    if (userScrollType == UserScrollType::Explicit)
    {
      _wasEverScrolledExplicitlyByUser = true;
    }
  }

  void LocalFrameView::ScheduleScrollEvent() noexcept
  {
    SetIsAwaitingScrollend(true);
    Frame().GetEventHandler().ScheduleScrollEvent();
    Frame().GetEventHandler().DispatchFakeMouseMoveEventSoon();
  }

  void LocalFrameView::UpdateSnapOffsetsAndResnap() noexcept
  {
    auto *document = Frame().Document();
    auto *documentElement = document ? dom::TreeQueries::DocumentElement(*document) : nullptr;
    auto *rootRenderer = documentElement ? DynamicDowncast<RenderBox>(documentElement->Renderer()) : nullptr;
    if (!rootRenderer || rootRenderer->Style().GetScrollSnapType().IsNone())
    {
      ClearSnapOffsets();
      return;
    }

    // Snap positions are found relative to the scroll container's padding box, but the view's viewport
    // includes the root's margins.
    LayoutRect viewport(LayoutPoint(), BaseLayoutViewportSize());
    viewport.Move(-rootRenderer->MarginLeft(), -rootRenderer->MarginTop());
    UpdateSnapOffsetsForScrollableArea(*this, *rootRenderer, viewport);
    ResnapAfterLayout();
  }

  void LocalFrameView::RegisterForAnimatedScroll() noexcept
  {
    AddScrollableAreaForAnimatedScroll(*this);
  }

  void LocalFrameView::AddScrollableAreaForAnimatedScroll(ScrollableArea &area) noexcept
  {
    auto isArea = [&](const WeakPtr<ScrollableArea> &existing)
    {
      return existing.get() == &area;
    };
    if (std::none_of(_scrollableAreasForAnimatedScroll.begin(), _scrollableAreasForAnimatedScroll.end(),
                     isArea))
    {
      _scrollableAreasForAnimatedScroll.push_back(CreateWeakPtr<ScrollableArea>(&area));
    }
    if (auto *document = Frame().Document())
    {
      document->ScheduleRenderingUpdate(RenderingUpdateStep::Scroll);
    }
  }

  void LocalFrameView::ServiceScrollAnimations(MonotonicTime::duration now) noexcept
  {
    auto areas = krys::move(_scrollableAreasForAnimatedScroll);
    _scrollableAreasForAnimatedScroll.clear();
    bool inProgress = false;
    for (auto &weakArea : areas)
    {
      auto *area = weakArea.get();
      auto *animator = area ? area->ExistingScrollAnimator() : nullptr;
      if (animator && animator->ServiceScrollAnimation(now))
      {
        inProgress = true;
        _scrollableAreasForAnimatedScroll.push_back(krys::move(weakArea));
      }
    }
    if (inProgress)
    {
      if (auto *document = Frame().Document())
      {
        document->ScheduleRenderingUpdate(RenderingUpdateStep::Scroll);
      }
    }
  }

  void LocalFrameView::ScrollDidEnd() noexcept
  {
    if (!IsAwaitingScrollend())
    {
      return;
    }
    SetIsAwaitingScrollend(false);
    if (auto *document = Frame().Document())
    {
      document->AddPendingScrollEventTarget(*document, dom::ScrollEventType::Scrollend);
    }
  }

  void LocalFrameView::UpdateAnchorPositionedAfterScroll() noexcept
  {
    if (auto *view = GetRenderViewForUpdate())
    {
      style::AnchorPositionEvaluator::UpdateScrollAdjustments(*view);
    }
  }

  LayoutPoint LocalFrameView::ScrollPositionForFixedPosition() const noexcept
  {
    // FIXME: The layout viewport's origin (visualViewportEnabled), once there are visual viewports.
    LayoutPoint position(GetScrollPosition());
    LayoutRect visibleContentRect(VisibleContentRect());
    LayoutSize totalContentsSize(ContentsSize());
    float frameScaleFactor = FrameScaleFactor();

    LayoutSize maxSize = totalContentsSize - visibleContentRect.Size();

    float dragFactorX =
      !maxSize.Width()
        ? 1
        : (totalContentsSize.Width() - visibleContentRect.Width() * frameScaleFactor) / maxSize.Width();
    float dragFactorY =
      !maxSize.Height()
        ? 1
        : (totalContentsSize.Height() - visibleContentRect.Height() * frameScaleFactor) / maxSize.Height();

    return LayoutPoint(position.X() * dragFactorX / frameScaleFactor,
                       position.Y() * dragFactorY / frameScaleFactor);
  }

  LayoutRect LocalFrameView::RectForFixedPositionLayout() const noexcept
  {
    // FIXME: The layout viewport (visualViewportEnabled), once there are visual viewports.
    return LayoutRect(VisibleContentRect());
  }

  LayoutSize LocalFrameView::BaseLayoutViewportSize() const noexcept
  {
    return GetRenderView() ? GetRenderView()->Size() : LayoutSize(Size());
  }

  LayoutRect LocalFrameView::LayoutViewportRect() const noexcept
  {
    // FIXME: The layout viewport's own origin and its override (updateLayoutViewport), once there are visual
    // viewports; until then it's at the scroll position.
    // Size of initial containing block, anchored at scroll position, in document coordinates (unchanged by
    // scale factor).
    return LayoutRect(ScrollPositionForFixedPosition(), BaseLayoutViewportSize());
  }

  bool LocalFrameView::ShouldPlaceVerticalScrollbarOnLeft() const noexcept
  {
    auto *renderView = GetRenderView();
    return renderView && renderView->Style().ShouldPlaceVerticalScrollbarOnLeft();
  }

  IntPoint LocalFrameView::ViewToContents(const IntPoint &point) const noexcept
  {
    return point + ToIntSize(DocumentScrollPositionRelativeToViewOrigin());
  }

  IntPoint LocalFrameView::ContentsToView(const IntPoint &point) const noexcept
  {
    return point - ToIntSize(DocumentScrollPositionRelativeToViewOrigin());
  }

  FloatPoint LocalFrameView::ViewToContents(const FloatPoint &point) const noexcept
  {
    return point + FloatSize(ToIntSize(DocumentScrollPositionRelativeToViewOrigin()));
  }

  FloatPoint LocalFrameView::ContentsToView(const FloatPoint &point) const noexcept
  {
    return point - FloatSize(ToIntSize(DocumentScrollPositionRelativeToViewOrigin()));
  }

  IntRect LocalFrameView::ViewToContents(IntRect rect) const noexcept
  {
    rect.Move(ToIntSize(DocumentScrollPositionRelativeToViewOrigin()));
    return rect;
  }

  IntRect LocalFrameView::ContentsToView(IntRect rect) const noexcept
  {
    rect.Move(-ToIntSize(DocumentScrollPositionRelativeToViewOrigin()));
    return rect;
  }

  void LocalFrameView::ScheduleSelectionUpdate() noexcept
  {
    if (LayoutContext().NeedsLayout())
    {
      return;
    }
    // FIXME: We should not need to go through the layout process since selection update does not change
    // dimension/geometry. However we can't tell at this point if the tree is stable yet, so let's just
    // schedule a root only layout for now.
    LayoutContext().SetNeedsLayoutAfterViewConfigurationChange();
  }

  void LocalFrameView::ScheduleScrollToFocusedElement(SelectionRevealMode selectionRevealMode) noexcept
  {
    if (selectionRevealMode == SelectionRevealMode::DoNotReveal)
    {
      return;
    }

    _selectionRevealModeForFocusedElement = selectionRevealMode;
    if (_shouldScrollToFocusedElement)
    {
      return;
    }
    _shouldScrollToFocusedElement = true;
    _delayedScrollToFocusedElementTimer.StartOneShot({});
  }

  void LocalFrameView::CancelScheduledScrolls() noexcept
  {
    _shouldScrollToFocusedElement = false;
    _delayedScrollToFocusedElementTimer.Stop();
  }

  namespace
  {
    KRYS_NODISCARD RefPtr<svg::SVGSvgElement> SVGRootElement(const dom::Document &document) noexcept
    {
      auto documentElement = document.DocumentElement();
      auto *svgElement = documentElement ? DynamicDowncast<svg::SVGElement>(*documentElement) : nullptr;
      return svgElement
               ? ShareRefPtr(DynamicDowncast<svg::SVGSvgElement>(const_cast<svg::SVGElement &>(*svgElement)))
               : nullptr;
    }
  }

  bool LocalFrameView::ScrollToFragment(const url::URL &url) noexcept
  {
    krys_assert(Frame().Document());

    // https://html.spec.whatwg.org/#the-indicated-part-of-the-document
    // 1. If document's URL does not equal url with exclude fragments set to true, then return null.
    // (url is the document's.)
    // 2. Let fragment be url's fragment.
    auto fragmentIdentifier = url.FragmentIdentifier();
    // 3. If fragment is the empty string, then return the special value top of the document.
    // 4. Let potentialIndicatedElement be the result of finding a potential indicated element given document
    //    and fragment.
    // 5. If potentialIndicatedElement is not null, then return potentialIndicatedElement.
    // (ScrollToAnchorFragment also takes a fragment that's "top" to be the top of the document, as step 10
    // would.)
    if (ScrollToAnchorFragment(fragmentIdentifier))
    {
      return true;
    }

    if (fragmentIdentifier.data())
    {
      // 6. Let fragmentBytes be the result of percent-decoding fragment.
      // 7. Let decodedFragment be the result of running UTF-8 decode without BOM on fragmentBytes.
      auto decoded = url::URLParser::PercentDecode(fragmentIdentifier);
      // 8. Set potentialIndicatedElement to the result of finding a potential indicated element given
      // document
      //    and decodedFragment.
      // 9. If potentialIndicatedElement is not null, then return potentialIndicatedElement.
      // 10. If decodedFragment is an ASCII case-insensitive match for the string top, then return the top of
      //     the document.
      if (decoded != fragmentIdentifier && ScrollToAnchorFragment(decoded))
      {
        return true;
      }
    }

    // 11. Return null.
    ResetScrollAnchor();
    return false;
  }

  bool LocalFrameView::ScrollToAnchorFragment(dom::DOMStringView fragmentIdentifier) noexcept
  {
    // If our URL has no ref, then we have no place we need to jump to.
    if (!fragmentIdentifier.data())
    {
      return false;
    }

    auto &document = *Frame().Document();
    krys_debug_assert(document.HaveStylesheetsLoaded());

    RefPtr<dom::Element> anchorElement = ShareRefPtr(document.FindAnchor(fragmentIdentifier));

    // https://html.spec.whatwg.org/#scroll-to-the-fragment-identifier
    // 1. If document's indicated part is null, then set document's target element to null.
    // 2a. Set document's target element to null.
    // 3a. Assert: document's indicated part is an element.
    // 3b. Let target be document's indicated part.
    // 3c. Set document's target element to target.
    document.SetCSSTarget(anchorElement.get());

    if (Is<svg::SVGDocument>(document))
    {
      if (fragmentIdentifier.empty())
      {
        return false;
      }
      if (auto rootElement = SVGRootElement(document))
      {
        if (rootElement->ScrollToFragment(fragmentIdentifier))
        {
          return true;
        }
        // If SVG failed to scrollToAnchor() and anchorElement is null, no other scrolling will be possible.
        if (!anchorElement)
        {
          return false;
        }
      }
    }
    else if (!anchorElement
             && !(fragmentIdentifier.empty() || text::IsASCIICaselessEqual(fragmentIdentifier, u8"top")))
    {
      // Implement the rule that "" and "top" both mean top of page as in other browsers.
      return false;
    }

    // 3d. Run the ancestor revealing algorithm on target.
    if (anchorElement)
    {
      (void)dom::RevealClosedDetailsAndHiddenUntilFoundAncestors(*anchorElement);
    }

    // 2. Otherwise, if document's indicated part is top of the document:
    // 2b. Scroll to the beginning of the document for document.
    // 3e. Scroll target into view, with behavior set to "auto", block set to "start", and inline set to
    //     "nearest".
    // (The scroll position is kept at the anchor while layout changes, until the user scrolls.)
    dom::ContainerNode *scrollPositionAnchor = anchorElement.get();
    if (!scrollPositionAnchor)
    {
      scrollPositionAnchor = &document;
    }
    MaintainScrollPositionAtAnchor(scrollPositionAnchor);

    if (anchorElement)
    {
      // 3f. Run the focusing steps for target, with the Document's viewport as the fallback target.
      // 3g. Move the sequential focus navigation starting point to target.
      dom::FocusOptions options;
      options.FocusVisible = true;
      html::FocusAlgorithms::FocusingSteps(*anchorElement, &document, options);
      document.SetSequentialFocusNavigationStartingPoint(anchorElement.get());
    }

    return true;
  }

  void LocalFrameView::MaintainScrollPositionAtAnchor(dom::ContainerNode *anchorNode) noexcept
  {
    _maintainScrollPositionAnchor = CreateWeakPtr(anchorNode);
    if (!anchorNode)
    {
      return;
    }

    CancelScheduledScrolls();

    if (auto *element = DynamicDowncast<dom::Element>(anchorNode))
    {
      Frame().Document()->UpdateContentRelevancyForScrollIfNeeded(*element);
    }

    // We need to update the layout before scrolling, otherwise we could really mess things up if an anchor
    // scroll comes at a bad moment.
    auto &document = *Frame().Document();
    document.UpdateStyleIfNeeded();
    // Only do a layout if changes have occurred that make it necessary.
    auto *renderView = GetRenderView();
    if (renderView && renderView->NeedsLayout())
    {
      LayoutContext().Layout();
    }
    else
    {
      ScheduleScrollToAnchor();
    }

    ScrollToAnchorNowIfNeeded();
  }

  int32 LocalFrameView::MapFromLayoutToCSSUnits(LayoutUnit value) const noexcept
  {
    return static_cast<int32>(value / (Frame().PageZoomFactor() * Frame().FrameScaleFactor()));
  }

  LayoutUnit LocalFrameView::MapFromCSSToLayoutUnits(int32 value) const noexcept
  {
    return LayoutUnit(value * Frame().PageZoomFactor() * Frame().FrameScaleFactor());
  }

  IntSize LocalFrameView::SizeForResizeEvent() const noexcept
  {
    // FIXME: The fixed layout size when scrolling is delegated, once it can be.
    return VisibleContentRectIncludingScrollbars().Size();
  }

  void LocalFrameView::ScheduleResizeEventIfNeeded() noexcept
  {
    if (_layoutContext.IsInRenderTreeLayout() || _layoutContext.NeedsLayout())
    {
      return;
    }

    auto *renderView = GetRenderView();
    if (!renderView)
    {
      return;
    }

    auto *page = Frame().GetPage();
    if (page && page->GetChromeClient().IsSVGImageChromeClient())
    {
      return;
    }

    auto currentSize = SizeForResizeEvent();
    float currentZoomFactor = renderView->Style().UsedZoom();
    float currentFrameScaleFactor = Frame().FrameScaleFactor();

    if (currentSize == _lastViewportSize && currentZoomFactor == _lastUsedZoomFactor
        && currentFrameScaleFactor == _lastFrameScaleFactor)
    {
      return;
    }

    _lastViewportSize = currentSize;
    _lastUsedZoomFactor = currentZoomFactor;
    _lastFrameScaleFactor = currentFrameScaleFactor;

    if (!_layoutContext.DidFirstLayout())
    {
      return;
    }

    Frame().Document()->SetNeedsDOMWindowResizeEvent();
  }

  void LocalFrameView::LayoutOrVisualViewportChanged() noexcept
  {
    if (auto *window = Frame().Window())
    {
      if (auto *visualViewport = window->OptionalVisualViewport())
      {
        visualViewport->Update();
      }
    }
  }

  void LocalFrameView::ScheduleScrollToAnchor() noexcept
  {
    auto *anchorNode = _maintainScrollPositionAnchor.get();
    if (!anchorNode)
    {
      return;
    }

    _scheduledMaintainScrollPositionAnchor = CreateWeakPtr(anchorNode);

    if (_scheduledToScrollToAnchor)
    {
      return;
    }

    auto *document = Frame().Document();
    if (!document)
    {
      return;
    }

    _scheduledToScrollToAnchor = true;
    html::EventLoop::Current().QueueTask(html::TaskSource::DOMManipulation, *document,
                                         [protectedThis = ShareRef(*this)]
                                         {
                                           if (!protectedThis->_maintainScrollPositionAnchor)
                                           {
                                             SetForScope scrollPositionAnchor(
                                               protectedThis->_maintainScrollPositionAnchor,
                                               protectedThis->_scheduledMaintainScrollPositionAnchor);
                                             protectedThis->ScrollToAnchorNowIfNeeded();
                                           }
                                           else
                                           {
                                             protectedThis->ScrollToAnchorNowIfNeeded();
                                           }
                                           protectedThis->_scheduledMaintainScrollPositionAnchor = nullptr;
                                         });
  }

  void LocalFrameView::ScrollToAnchorNowIfNeeded() noexcept
  {
    if (!_scheduledToScrollToAnchor)
    {
      return;
    }

    _scheduledToScrollToAnchor = false;
    ScrollToAnchor();
  }

  void LocalFrameView::ScrollToAnchor() noexcept
  {
    RefPtr<dom::ContainerNode> anchorNode = ShareRefPtr(_maintainScrollPositionAnchor.get());
    if (!anchorNode)
    {
      return;
    }

    if (!anchorNode->Renderer())
    {
      return;
    }

    CancelScheduledScrolls();

    LayoutRect rect;
    bool insideFixed = false;
    if (anchorNode != Frame().Document())
    {
      if (auto *renderElement = DynamicDowncast<RenderElement>(anchorNode->Renderer()))
      {
        rect = renderElement->AbsoluteAnchorRectWithScrollMargin(&insideFixed).first;
      }
    }

    // Scroll nested layers and frames to reveal the anchor.
    // Align to the top and to the closest side (this matches other browsers).
    auto &renderer = *anchorNode->Renderer();
    ScrollRectToVisibleOptions options;
    auto writingMode = renderer.GetWritingMode();
    if (writingMode.IsHorizontal())
    {
      options.AlignX = ScrollAlignment::AlignToEdgeIfNeeded;
      options.AlignY = ScrollAlignment::AlignTopAlways;
    }
    else if (writingMode.BlockDirection() == FlowDirection::RightToLeft)
    {
      options.AlignX = ScrollAlignment::AlignRightAlways;
      options.AlignY = ScrollAlignment::AlignToEdgeIfNeeded;
    }
    else
    {
      options.AlignX = ScrollAlignment::AlignLeftAlways;
      options.AlignY = ScrollAlignment::AlignToEdgeIfNeeded;
    }
    ScrollRectToVisible(rect, renderer, insideFixed, options);

    if (auto *cache = Frame().Document() ? Frame().Document()->ExistingAXObjectCache() : nullptr)
    {
      cache->HandleScrolledToAnchor(*anchorNode);
    }

    // scrollRectToVisible can call into setScrollPosition(), which resets _maintainScrollPositionAnchor.
    _maintainScrollPositionAnchor = CreateWeakPtr(anchorNode.get());
    CancelScheduledScrolls();
  }

  void LocalFrameView::ScrollToFocusedElementTimerFired() noexcept
  {
    ScrollToFocusedElementInternal();
  }

  void LocalFrameView::ScrollToFocusedElementInternal() noexcept
  {
    krys_assert(_shouldScrollToFocusedElement);
    auto *document = Frame().Document();
    if (!document)
    {
      return;
    }

    document->UpdateLayoutIgnorePendingStylesheets();
    if (!_shouldScrollToFocusedElement || _delayedScrollToFocusedElementTimer.IsActive())
    {
      return; // Updating the layout may have ran scripts.
    }

    _shouldScrollToFocusedElement = false;

    auto *focusedElement = document->FocusedElement();
    if (!focusedElement)
    {
      return;
    }

    auto updateTarget = focusedElement->FocusAppearanceUpdateTarget();
    if (!updateTarget)
    {
      return;
    }

    // Get the scroll-margin of the shadow host when we're inside a user agent shadow root.
    if (updateTarget->IsInUserAgentShadowTree())
    {
      updateTarget = ShareRefPtr(dom::TreeQueries::ShadowHost(*updateTarget));
    }

    auto *renderer = updateTarget->Renderer();
    if (!renderer)
    {
      return;
    }

    bool insideFixed = false;
    auto [anchorRectWithScrollMargin, anchorRect] =
      renderer->AbsoluteAnchorRectWithScrollMargin(&insideFixed);
    ScrollRectToVisibleOptions options;
    options.RevealMode = _selectionRevealModeForFocusedElement;
    options.VisibilityCheckRect = anchorRect;
    ScrollRectToVisible(anchorRectWithScrollMargin, *renderer, insideFixed, options);
  }

  float LocalFrameView::FrameScaleFactor() const noexcept
  {
    return Frame().FrameScaleFactor();
  }

  float LocalFrameView::DocumentToAbsoluteScaleFactor(Maybe<float> usedZoom) const noexcept
  {
    // If usedZoom is passed, it already factors in pageZoomFactor().
    return (usedZoom ? *usedZoom : Frame().PageZoomFactor()) * Frame().FrameScaleFactor();
  }

  float LocalFrameView::AbsoluteToDocumentScaleFactor(Maybe<float> usedZoom) const noexcept
  {
    // If usedZoom is passed, it already factors in pageZoomFactor().
    return 1 / DocumentToAbsoluteScaleFactor(usedZoom);
  }

  FloatRect LocalFrameView::AbsoluteToDocumentRect(FloatRect rect, Maybe<float> usedZoom) const noexcept
  {
    rect.Scale(AbsoluteToDocumentScaleFactor(usedZoom));
    return rect;
  }

  FloatSize LocalFrameView::DocumentToClientOffset() const noexcept
  {
    auto clientOrigin =
      -FloatSize(static_cast<float>(VisibleContentRect().X()), static_cast<float>(VisibleContentRect().Y()));

    // Layout and visual viewports are affected by page zoom, so we need to factor that out.
    return clientOrigin.Scaled(1 / (Frame().PageZoomFactor() * Frame().FrameScaleFactor()));
  }

  FloatRect LocalFrameView::DocumentToClientRect(FloatRect rect) const noexcept
  {
    rect.Move(DocumentToClientOffset());
    return rect;
  }

  FloatRect LocalFrameView::AbsoluteToClientRect(FloatRect rect, Maybe<float> usedZoom) const noexcept
  {
    return DocumentToClientRect(AbsoluteToDocumentRect(rect, usedZoom));
  }

  FloatRect LocalFrameView::AbsoluteToLayoutViewportRect(FloatRect rect) const noexcept
  {
    rect.Scale(1 / Frame().FrameScaleFactor());
    rect.MoveBy(-FloatPoint(LayoutViewportRect().Location()));
    return rect;
  }

  DoublePoint LocalFrameView::AbsoluteToDocumentPoint(DoublePoint point, Maybe<float> usedZoom) const noexcept
  {
    return point.Scaled(AbsoluteToDocumentScaleFactor(usedZoom));
  }

  DoublePoint LocalFrameView::DocumentToClientPoint(DoublePoint point) const noexcept
  {
    return point + DoubleSize(DocumentToClientOffset());
  }

  // The window is the view's own coordinates: it's the root of its (absent) widget tree, at the origin.
  // FIXME: The conversions through the root view (rootViewToContents), once views are widgets in a tree.

  DoublePoint LocalFrameView::WindowToContents(const DoublePoint &point) const noexcept
  {
    return point + DoubleSize(ToIntSize(DocumentScrollPositionRelativeToViewOrigin()));
  }

  IntPoint LocalFrameView::WindowToContents(const IntPoint &point) const noexcept
  {
    return ViewToContents(point);
  }

  IntPoint LocalFrameView::ContentsToWindow(const IntPoint &point) const noexcept
  {
    return ContentsToView(point);
  }

  IntRect LocalFrameView::ContentsToWindow(const IntRect &rect) const noexcept
  {
    return ContentsToView(rect);
  }

  IntRect LocalFrameView::ContentsToScreen(const IntRect &rect) const noexcept
  {
    auto *frame = _frame.get();
    auto *page = frame ? frame->GetPage() : nullptr;
    if (!page)
    {
      return {};
    }
    return page->GetChromeClient().RootViewToScreen(ContentsToWindow(rect));
  }

  IntPoint LocalFrameView::ScreenToContents(const IntPoint &point) const noexcept
  {
    auto *frame = _frame.get();
    auto *page = frame ? frame->GetPage() : nullptr;
    if (!page)
    {
      return {};
    }
    return WindowToContents(page->GetChromeClient().ScreenToRootView(point));
  }

  void LocalFrameView::ResetScrollAnchor() noexcept
  {
    krys_assert(Frame().Document());
    auto &document = *Frame().Document();

    // If CSS target was set previously, we want to set it to 0, recalc
    // and possibly repaint because :target pseudo class may have been
    // set (see bug 11321).
    document.SetCSSTarget(nullptr);

    if (Is<svg::SVGDocument>(document))
    {
      if (auto rootElement = SVGRootElement(document))
      {
        // We need to update the layout before resetScrollAnchor(), otherwise we
        // could really mess things up if resetting the anchor comes at a bad moment.
        (void)document.UpdateStyleIfNeeded();
        rootElement->ResetScrollAnchor();
      }
    }
  }
}
