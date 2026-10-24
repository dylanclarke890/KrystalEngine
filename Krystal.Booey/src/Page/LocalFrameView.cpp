#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/FragmentDirectiveParser.hpp"
#include "Krystal.Booey/DOM/FragmentDirectiveRangeFinder.hpp"
#include "Krystal.Booey/DOM/HighlightRegistry.hpp"
#include "Krystal.Booey/Editing/TemporarySelectionChange.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/Editing/VisiblePosition.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayer.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLHtmlElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Page/VisualViewport.hpp"
#include "Krystal.Booey/Platform/ScrollAlignment.hpp"
#include "Krystal.Booey/Platform/ScrollbarTheme.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerBacking.hpp"
#include "Krystal.Booey/Rendering/RenderLayerCompositor.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Values/Scrollbars/ScrollbarWidth.hpp"
#include "Krystal.Booey/URL/URLParser.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  KRYS_NODISCARD static const css::CSSOMStringAtom &ScreenAtom() noexcept
  {
    static MainThreadNeverDestroyed<css::CSSOMStringAtom> screen {u8"screen"};
    return screen;
  }

  LocalFrameView::LocalFrameView(LocalFrame &frame) noexcept
      : _frame(WeakPtr<LocalFrame>::WithRef(frame)), _mediaType(ScreenAtom()), _layoutContext(*this),
        _delayedScrollToFocusedElementTimer(*this, &LocalFrameView::ScrollToFocusedElementTimerFired),
        _scrollToTextFragmentRetryTimer(*this, &LocalFrameView::ScrollToTextFragmentRetryTimerFired)
  {
  }

  LocalFrameView::~LocalFrameView() noexcept = default;

  Ref<LocalFrameView> LocalFrameView::Create(LocalFrame &frame) noexcept
  {
    return AdoptRef(*new LocalFrameView(frame));
  }

  Ref<LocalFrameView> LocalFrameView::Create(LocalFrame &frame, const IntSize &initialSize) noexcept
  {
    Ref<LocalFrameView> view = AdoptRef(*new LocalFrameView(frame));
    view->_frameRect = IntRect(view->_frameRect.Location(), initialSize);
    return view;
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

    // FIXME: Tell the document its view changed size (resize events) and update the viewport-constrained
    // objects and intersection observers, once there's a window and layout.
  }

  void LocalFrameView::Resize(const IntSize &size) noexcept
  {
    SetFrameRect(IntRect(_frameRect.Location(), size));
  }

  void LocalFrameView::RepaintContentRectangle(const IntRect &rect) noexcept
  {
    // FIXME: Not while updates are suppressed (shouldUpdate), once there's printing.
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
    // FIXME: Overlay scrollbars, once there are any.
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

    // FIXME: The overhang areas, once there's rubber-banding.
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

  bool LocalFrameView::FlushCompositingStateForThisFrame(const LocalFrame &rootFrameForFlush) noexcept
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

    renderView->Compositor().FlushPendingLayerChanges(&rootFrameForFlush == &Frame());

    return true;
  }

  bool LocalFrameView::FlushCompositingStateIncludingSubframes() noexcept
  {
    // FIXME: Subframes, once there are any.
    return FlushCompositingStateForThisFrame(Frame());
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
      htmlBackgroundColor = htmlElement->Renderer()->Style().VisitedDependentColorWithColorFilter(
        css::PropertyId::BackgroundColor);
    }
    if (bodyElement && bodyElement->Renderer())
    {
      bodyBackgroundColor = bodyElement->Renderer()->Style().VisitedDependentColorWithColorFilter(
        css::PropertyId::BackgroundColor);
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

  int32 LocalFrameView::InsetForLeftScrollbarSpace() const noexcept
  {
    // FIXME: Stable scrollbar gutters on both edges (scrollbarGutterWidth), once there are scrollbar gutters.
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
    // FIXME: The scrolling thread updating the fixed elements (shouldUpdateCompositingLayersAfterScrolling),
    // once there's async scrolling.
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

    // FIXME: A frame inside a compositing layer repaints its owner's box, once there are frames in frames.

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
    // FIXME: The scrolling coordinator and anchor-positioned viewport-constrained objects, once there are
    // any.
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
    // FIXME: The space a stable `scrollbar-gutter` keeps for the view's scrollbars (totalScrollbarSpace),
    // once the view reads it.
    IntSize scrollbarSpace;
    if (scrollbarInclusion == VisibleContentRectIncludesScrollbars::No)
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

    // FIXME: The overhang areas, once there's rubber-banding.

    // Now paint the scrollbars.
    if (_horizontalScrollbar || _verticalScrollbar)
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

    // FIXME: The pan scroll icon, once there's pan scrolling.
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
        auto rootColor =
          rootBackground->Style().VisitedDependentColorWithColorFilter(css::PropertyId::BackgroundColor);
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
    // FIXME: Custom scroll corners (::-webkit-scrollbar-corner), dropped as non-standard.
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
    // FIXME: Custom scrollbars (::-webkit-scrollbar, dropped as non-standard) and a frame owner's scrollbar
    // style, once there are frames.
    return Scrollbar::CreateNativeScrollbar(*this, orientation, ScrollbarWidthStyle());
  }

  void LocalFrameView::UpdateScrollbarSteps() noexcept
  {
    if (_horizontalScrollbar)
    {
      _horizontalScrollbar->SetSteps(Scrollbar::PixelsPerLineStep(VisibleWidth()),
                                     Scrollbar::PageStep(VisibleWidth()));
    }
    if (_verticalScrollbar)
    {
      _verticalScrollbar->SetSteps(Scrollbar::PixelsPerLineStep(VisibleHeight()),
                                   Scrollbar::PageStep(VisibleHeight()));
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

    // FIXME: Disregarding hidden when the main frame is scaled or has a header or footer (overrideHidden),
    // once frames scale and have those; embedded SVG documents, once there's SVG rendering.
    bool overrideHidden = false;

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
      default:
        // Don't set it at all. Values of Overflow::PagedX and Overflow::PagedY are handled by
        // applyPaginationToViewPort().
        break;
    }
  }

  void LocalFrameView::CalculateScrollbarModesForLayout(
    ScrollbarMode &hMode, ScrollbarMode &vMode, ScrollbarModesCalculationStrategy strategy) const noexcept
  {
    // FIXME: The frame's scrolling mode (scrollingMode), once there are frames; the viewport renderer type.
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

    // FIXME: Framesets turn the scrollbars off, once there are framesets.

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

  void LocalFrameView::AdjustScrollbarsForLayout(bool) noexcept
  {
    ScrollbarMode hMode;
    ScrollbarMode vMode;
    CalculateScrollbarModesForLayout(hMode, vMode);

    // FIXME: Starting the first layout with a vertical scrollbar (suppressed, with the vertical mode always
    // on and the horizontal always off), once scrollbars can be suppressed.
    if (hMode != HorizontalScrollbarMode() || vMode != VerticalScrollbarMode())
    {
      SetScrollbarModes(hMode, vMode);
    }
  }

  void LocalFrameView::WillDoLayout(const RenderElement &layoutRoot) noexcept
  {
    bool subtreeLayout = !Is<RenderView>(layoutRoot);
    if (subtreeLayout)
    {
      return;
    }

    // FIXME: A frameset body lays its children out, once there are framesets.
    auto firstLayout = !_layoutContext.DidFirstLayout();
    if (firstLayout)
    {
      _firstLayoutCallbackPending = true;
      _lastViewportSize = SizeForResizeEvent();
      _lastUsedZoomFactor = layoutRoot.Style().UsedZoom();
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

  void LocalFrameView::FireLayoutRelatedMilestonesIfNeeded() noexcept
  {
    // FIXME: The page's other layout milestones (visually non-empty), once a client asks for them.
    if (_firstLayoutCallbackPending)
    {
      _firstLayoutCallbackPending = false;
      Frame().Loader().DidFirstLayout();
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

  bool LocalFrameView::LogicalScroll(ScrollLogicalDirection direction, ScrollGranularity granularity) noexcept
  {
    return Scroll(LogicalToPhysical(direction, IsVerticalDocument(), IsFlippedDocument()), granularity);
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

    // FIXME: #2, a frame owner that isn't visible to hit testing, once there are frames.

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

    RenderLayer *rootLayer = renderView->Layer();
    rootLayer->Paint(context, LayoutRect(dirtyRect), LayoutSize(), _paintBehavior, nullptr);

    _paintBehavior = paintBehavior;
    _isPainting = false;
  }

  void LocalFrameView::AvailableContentSizeChanged(AvailableSizeChangeReason reason) noexcept
  {
    if (auto *document = Frame().Document())
    {
      // FIXME: Merge this logic with m_setNeedsLayoutWasDeferred and find a more appropriate
      // way of handling potential recursive layouts when the viewport is resized to accomodate
      // the content but the content always overflows the viewport. See webkit.org/b/165781.
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

  void LocalFrameView::SetPagination(const boo::Pagination &pagination) noexcept
  {
    if (_pagination == pagination)
    {
      return;
    }

    _pagination = pagination;

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
      // FIXME: Adjust the fixed axes for anchor-positioned fixed boxes (adjustForScrollByAnchor), once
      // there's anchor positioning.
    }

    // FIXME: Scroll a child frame's view and then its owner (scrollRectToVisibleInChildView), once there are
    // frames.
    renderer.View().FrameView().ScrollRectToVisibleInTopLevelView(adjustedRect, isFixed, adjustedOptions);
    return true;
  }

  void LocalFrameView::ScrollRectToVisibleInTopLevelView(const LayoutRect &absoluteRect,
                                                         EnumSet<BoxAxis> isFixed,
                                                         const ScrollRectToVisibleOptions &options) noexcept
  {
    // FIXME: Revealing up to the main frame only and delegating the main frame's scroll to the client
    // (RevealUpToMainFrame, DelegateMainFrameScroll), once there are frames and a client; the view is the
    // main frame's.
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
    // FIXME: Only scrolling forward (OnlyAllowForwardScrolling), once there's find-in-page; smooth scrolling
    // (useSmoothScrolling), once there are scroll animations.
    if (revealRect != viewRect)
    {
      ScrollOffset clampedScrollPosition =
        RoundedIntPoint(revealRect.Location()).ConstrainedBetween(minScrollPosition, maxScrollPosition);
      SetScrollPosition(clampedScrollPosition, ScrollPositionChangeOptions::CreateProgrammatic());
    }

    // FIXME: Scrolling the client's containing scroll views (scrollContainingScrollViewsToRevealRect), once
    // there's a client.
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
    // FIXME: The scaled cases, once frames can be scaled.
    return visibleRect;
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
    // FIXME: The overhang areas, once there's rubber-banding.
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

    // FIXME: Snapping, once there's scroll snapping; animated scrolls (scrollToPositionWithAnimation), once
    // there are scroll animations.
    auto position = ScrollPositionFromOffset(scrollOffset);

    // ScrollView::setScrollPosition.
    // FIXME: Prohibiting scrolling and stopping scroll animations, once there are those.
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
      // FIXME: Not while rubber-banding, once there's rubber-banding.
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
    // FIXME: Not while the scrollbars are suppressed, once they can be.
    if (scrollbarCanTakeSpace)
    {
      SetForScope inUpdateScrollbarsScope(_inUpdateScrollbars, true);
      UpdateContentsSize();
    }

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

    if (hScroll != ScrollbarMode::Auto && vScroll != ScrollbarMode::Auto)
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
      if (oldRect != _horizontalScrollbar->FrameRect())
      {
        _horizontalScrollbar->Invalidate();
      }

      _horizontalScrollbar->SetEnabled(ContentsSize().Width() > clientWidth);
      _horizontalScrollbar->SetProportion(clientWidth, ContentsSize().Width());
    }

    if (_verticalScrollbar)
    {
      int32 clientHeight = VisibleHeight();
      IntRect oldRect(_verticalScrollbar->FrameRect());
      _verticalScrollbar->SetFrameRect(
        IntRect(ShouldPlaceVerticalScrollbarOnLeft() ? 0 : Width() - _verticalScrollbar->Width(), 0,
                _verticalScrollbar->Width(),
                Height() - (_horizontalScrollbar ? _horizontalScrollbar->OccupiedHeight() : 0)));
      if (oldRect != _verticalScrollbar->FrameRect())
      {
        _verticalScrollbar->Invalidate();
      }

      _verticalScrollbar->SetEnabled(TotalContentsSize().Height() > clientHeight);
      _verticalScrollbar->SetProportion(clientHeight, TotalContentsSize().Height());
    }

    UpdateScrollbarSteps();

    PositionScrollbarLayers();
    // FIXME: Repainting the scroll corner when it changes (updateScrollCorner), once there are custom scroll
    // corners.

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

    // FIXME: Suppressed scrollbars and deferring the update while the contents size changes, once there are
    // scrollbars.

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
    // FIXME: Throttling the scroll events (eventThrottlingDelay), once there's a client.
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
    // FIXME: Subframes, once there are any.
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

  LayoutPoint LocalFrameView::ScrollPositionForFixedPosition() const noexcept
  {
    // FIXME: The layout viewport's origin (visualViewportEnabled), the scroll position constrained to the
    // document for rubber-banding, the header and footer and the frame scale factor (the drag factors), once
    // there are visual viewports, rubber-banding, headers and page scaling.
    return LayoutPoint(GetScrollPosition());
  }

  LayoutRect LocalFrameView::RectForFixedPositionLayout() const noexcept
  {
    // FIXME: The layout viewport (visualViewportEnabled), once there are visual viewports.
    return LayoutRect(VisibleContentRect());
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
    CancelScheduledTextFragmentIndicatorTimer();
  }

  void LocalFrameView::CancelScheduledTextFragmentIndicatorTimer() noexcept
  {
    if (_skipScrollResetOfScrollToTextFragmentRange)
    {
      return;
    }
    _pendingTextFragmentIndicatorRange = std::nullopt;
    _pendingTextFragmentIndicatorText.clear();
    _lastRevealedTextFragmentRect = std::nullopt;
    // FIXME: The text indicator (its delayed timer and whether it was created), once there's one.
  }

  bool LocalFrameView::ScrollToTextFragment(bool isRetry) noexcept
  {
    static constexpr MonotonicTime::duration scrollToTextFragmentRetryInterval =
      std::chrono::milliseconds(500);
    static constexpr MonotonicTime::duration scrollToTextFragmentRetryTimeout = std::chrono::seconds(3);

    krys_assert(Frame().Document());
    Ref<dom::Document> document = ShareRef(*Frame().Document());

    if (!document->GetSettings().ScrollToTextFragmentEnabled)
    {
      return false;
    }

    // FIXME: Only the main frame, once there are other frames.

    document->FragmentHighlightRegistry().Clear();

    auto &fragmentDirective = document->FragmentDirective();
    if (fragmentDirective.empty())
    {
      return false;
    }

    dom::FragmentDirectiveParser fragmentDirectiveParser(fragmentDirective);
    if (!fragmentDirectiveParser.IsValid())
    {
      return false;
    }

    auto &parsedTextDirectives = fragmentDirectiveParser.ParsedTextDirectives();
    auto highlightRanges =
      dom::FragmentDirectiveRangeFinder::FindRangesFromTextDirectives(parsedTextDirectives, *document);
    if (document->GetSettings().ScrollToTextFragmentMarkingEnabled)
    {
      for (auto &range : highlightRanges)
      {
        if (auto init = dom::StaticRangeInit::Create(range.Start, range.End); !init.HasException())
        {
          document->FragmentHighlightRegistry().AddAnnotationHighlightWithRange(
            CreateRef<dom::StaticRange>(init.ReleaseValue()));
        }
      }
    }

    // If we didn't find the expected range, retry every 500ms, until the first of:
    //   - we successfully find the range
    //   - the user scrolls the view
    //   - the 3s timeout expires
    if (highlightRanges.empty())
    {
      auto now = html::EventLoop::Current().Now();
      if (!isRetry)
      {
        _scrollToTextFragmentRetryTimer.StartRepeating(scrollToTextFragmentRetryInterval);
        _scrollToTextFragmentInitialAttemptTime = now;
      }
      else if (!_scrollToTextFragmentInitialAttemptTime
               || now - *_scrollToTextFragmentInitialAttemptTime >= scrollToTextFragmentRetryTimeout
               || _wasEverScrolledExplicitlyByUser)
      {
        _scrollToTextFragmentRetryTimer.Stop();
      }

      return false;
    }

    _scrollToTextFragmentRetryTimer.Stop();

    auto &range = highlightRanges.front();
    RefPtr<dom::Node> commonAncestor =
      ShareRefPtr(dom::CommonInclusiveAncestor<dom::TreeType::ComposedTree>(range));
    if (commonAncestor && !Is<dom::Element>(*commonAncestor))
    {
      commonAncestor = ShareRefPtr<dom::Node>(commonAncestor->ParentElement());
    }
    if (commonAncestor)
    {
      document->SetCSSTarget(&Downcast<dom::Element>(*commonAncestor));
      // FIXME: Revealing the closed details and hidden=until-found ancestors, with <details> (B4).
    }

    {
      TemporarySelectionChange selectionChange(
        *document, VisibleSelection(range),
        {TemporarySelectionOption::RevealSelection, TemporarySelectionOption::RevealSelectionBounds,
         TemporarySelectionOption::UserTriggered, TemporarySelectionOption::ForceCenterScroll});
    }
    // FIXME: The text indicator, once there's one (scrollToTextFragmentIndicatorEnabled).

    MaintainScrollPositionAtScrollToTextFragmentRange(range);

    return true;
  }

  void LocalFrameView::ScrollToTextFragmentRetryTimerFired() noexcept
  {
    (void)ScrollToTextFragment(true);
  }

  void
    LocalFrameView::MaintainScrollPositionAtScrollToTextFragmentRange(const dom::SimpleRange &range) noexcept
  {
    _pendingTextFragmentIndicatorRange = range;
    _pendingTextFragmentIndicatorText = PlainText(range);
    _lastRevealedTextFragmentRect = std::nullopt;
    if (!_pendingTextFragmentIndicatorRange)
    {
      return;
    }

    ScrollToPendingTextFragmentRange();
  }

  void LocalFrameView::ScrollToPendingTextFragmentRange() noexcept
  {
    if (_wasEverScrolledExplicitlyByUser)
    {
      return;
    }

    if (!_pendingTextFragmentIndicatorRange)
    {
      return;
    }

    if (LayoutContext().NeedsLayout())
    {
      return;
    }

    auto range = *_pendingTextFragmentIndicatorRange;
    if (_pendingTextFragmentIndicatorText != PlainText(range))
    {
      return;
    }

    if (!range.StartContainer().Renderer() || !range.EndContainer().Renderer())
    {
      return;
    }

    // Revealing the range changes the selection, which lays out again (Booey brings the render tree's
    // selection up to date with a layout), so the range is only revealed again once layout has moved it.
    auto rangeRect =
      UnionRect(VisiblePosition(dom::MakeContainerOffsetPosition(range.Start)).AbsoluteCaretBounds(),
                VisiblePosition(dom::MakeContainerOffsetPosition(range.End)).AbsoluteCaretBounds());
    if (_lastRevealedTextFragmentRect == rangeRect)
    {
      return;
    }
    _lastRevealedTextFragmentRect = rangeRect;

    auto &document = *Frame().Document();

    SetForScope skipScrollResetOfScrollToTextFragmentRange(_skipScrollResetOfScrollToTextFragmentRange, true);

    TemporarySelectionChange selectionChange(
      document, VisibleSelection(range),
      {TemporarySelectionOption::RevealSelection, TemporarySelectionOption::RevealSelectionBounds,
       TemporarySelectionOption::UserTriggered, TemporarySelectionOption::ForceCenterScroll,
       TemporarySelectionOption::OnlyAllowForwardScrolling});

    // FIXME: The text indicator, once there's one.
  }

  bool LocalFrameView::ScrollToFragment(const url::URL &url) noexcept
  {
    krys_assert(Frame().Document());

    if (ScrollToTextFragment())
    {
      return true;
    }

    auto fragmentIdentifier = url.FragmentIdentifier();
    if (ScrollToAnchorFragment(fragmentIdentifier))
    {
      return true;
    }

    if (fragmentIdentifier.data())
    {
      // https://html.spec.whatwg.org/#find-a-potential-indicated-element, with the fragment percent-decoded.
      auto decoded = url::URLParser::PercentDecode(fragmentIdentifier);
      if (decoded != fragmentIdentifier && ScrollToAnchorFragment(decoded))
      {
        return true;
      }
    }

    // FIXME: Resetting the scroll anchor (resetScrollAnchor), once there's scroll anchoring.
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

    // Setting to null will clear the current target.
    document.SetCSSTarget(anchorElement.get());

    // FIXME: SVG documents' own fragments (SVGSVGElement::scrollToFragment), with SVG.
    // Implement the rule that "" and "top" both mean top of page as in other browsers.
    if (!anchorElement
        && !(fragmentIdentifier.empty()
             || text::IsASCIICaselessEqual(fragmentIdentifier, utf8_stringview(u8"top"))))
    {
      return false;
    }

    // FIXME: Revealing the target's closed details and hidden=until-found ancestors, with <details> (B4).

    dom::ContainerNode *scrollPositionAnchor = anchorElement.get();
    if (!scrollPositionAnchor)
    {
      scrollPositionAnchor = &document;
    }
    MaintainScrollPositionAtAnchor(scrollPositionAnchor);

    if (anchorElement)
    {
      // Run the focusing steps for target, with the Document's viewport as the fallback target, and move the
      // sequential focus navigation starting point to target.
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

    // FIXME: Making a content-visibility: auto ancestor relevant (updateContentRelevancyForScrollIfNeeded),
    // once content-visibility is.

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
      ScheduleScrollToAnchorAndTextFragment();
    }

    ScrollToAnchorAndTextFragmentNowIfNeeded();
  }

  int32 LocalFrameView::MapFromLayoutToCSSUnits(LayoutUnit value) const noexcept
  {
    // FIXME: The frame scale factor, once frames can be scaled.
    return static_cast<int32>(value / Frame().PageZoomFactor());
  }

  LayoutUnit LocalFrameView::MapFromCSSToLayoutUnits(int32 value) const noexcept
  {
    return LayoutUnit(value * Frame().PageZoomFactor());
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

    auto currentSize = SizeForResizeEvent();
    float currentZoomFactor = renderView->Style().UsedZoom();

    if (currentSize == _lastViewportSize && currentZoomFactor == _lastUsedZoomFactor)
    {
      return;
    }

    _lastViewportSize = currentSize;
    _lastUsedZoomFactor = currentZoomFactor;

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

  void LocalFrameView::ScheduleScrollToAnchorAndTextFragment() noexcept
  {
    auto *anchorNode = _maintainScrollPositionAnchor.get();
    if (!anchorNode && !_pendingTextFragmentIndicatorRange)
    {
      return;
    }

    _scheduledMaintainScrollPositionAnchor = CreateWeakPtr(anchorNode);

    if (_scheduledToScrollToAnchor)
    {
      return;
    }

    _scheduledToScrollToAnchor = true;
    html::EventLoop::Current().QueueTask(html::TaskSource::DOMManipulation,
                                         [protectedThis = ShareRef(*this)]
                                         {
                                           if (!protectedThis->_maintainScrollPositionAnchor)
                                           {
                                             SetForScope scrollPositionAnchor(
                                               protectedThis->_maintainScrollPositionAnchor,
                                               protectedThis->_scheduledMaintainScrollPositionAnchor);
                                             protectedThis->ScrollToAnchorAndTextFragmentNowIfNeeded();
                                           }
                                           else
                                           {
                                             protectedThis->ScrollToAnchorAndTextFragmentNowIfNeeded();
                                           }
                                           protectedThis->_scheduledMaintainScrollPositionAnchor = nullptr;
                                         });
  }

  void LocalFrameView::ScrollToAnchorAndTextFragmentNowIfNeeded() noexcept
  {
    if (!_scheduledToScrollToAnchor)
    {
      return;
    }

    _scheduledToScrollToAnchor = false;
    ScrollToAnchor();
    ScrollToPendingTextFragmentRange();
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

    // FIXME: Telling accessibility (handleScrolledToAnchor), with accessibility (B17).

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

  float LocalFrameView::DocumentToAbsoluteScaleFactor(Maybe<float> usedZoom) const noexcept
  {
    // If usedZoom is passed, it already factors in pageZoomFactor().
    return usedZoom ? *usedZoom : Frame().PageZoomFactor();
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
    return clientOrigin.Scaled(1 / Frame().PageZoomFactor());
  }

  FloatRect LocalFrameView::DocumentToClientRect(FloatRect rect) const noexcept
  {
    rect.Move(DocumentToClientOffset());
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
}
