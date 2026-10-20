#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLHtmlElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Platform/ScrollAlignment.hpp"
#include "Krystal.Booey/Platform/ScrollbarTheme.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Values/Scrollbars/ScrollbarWidth.hpp"
#include "Krystal.Core/Assert.hpp"
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
        _delayedScrollToFocusedElementTimer(*this, &LocalFrameView::ScrollToFocusedElementTimerFired)
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

    if (!_useFixedLayout && oldRect.Size() != newRect.Size())
    {
      AvailableContentSizeChanged(AvailableSizeChangeReason::AreaSizeChanged);
    }

    ViewportContentsChanged();

    // FIXME: Tell the document its view changed size (resize events) and update the viewport-constrained
    // objects and intersection observers, once there's a window and layout.
  }

  void LocalFrameView::Resize(const IntSize &size) noexcept
  {
    SetFrameRect(IntRect(_frameRect.Location(), size));
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
    // FIXME: Lay the view out again after the configuration change and update compositing, once there's
    // compositing (setNeedsLayoutAfterViewConfigurationChange, setNeedsCompositingConfigurationUpdate).
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
    if (_horizontalScrollbar)
    {
      _horizontalScrollbar->Paint(context, rect);
    }
    if (_verticalScrollbar)
    {
      _verticalScrollbar->Paint(context, rect);
    }

    PaintScrollCorner(context, ScrollCornerRect());
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

    // FIXME: A frameset body lays its children out, once there are framesets; the resize event's viewport
    // size and zoom and the first layout callback, once there's a window.
    auto firstLayout = !_layoutContext.DidFirstLayout();
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

    // FIXME: The scroll position anchor and the scheduled scrolls (the focused element's scroll and the text
    // fragment indicator) are cleared, once they're kept; snapping, once there's scroll snapping; animated
    // scrolls (scrollToPositionWithAnimation), once there are scroll animations.
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

    // FIXME: Telling the compositor and the inspector a scrollbar was added or removed
    // (addedOrRemovedScrollbar), once there are those.
    (void)scrollbarAddedOrRemoved;

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

    // FIXME: Repositioning the scrollbar layers and repainting the scroll corner, once there are compositing
    // layers and repaint tracking.

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

    // completeUpdatesAfterScrollTo: the fixed layers move with the scroll position.
    // FIXME: Repainting the scrolled contents (scrollContents), once repaints are tracked (the whole page
    // repaints), and the compositing layers, once there's compositing.
    if (!LayoutContext().IsInRenderTreeLayout())
    {
      if (auto *renderView = const_cast<RenderView *>(GetRenderView()); renderView && renderView->Layer())
      {
        renderView->Layer()->UpdateLayerPositionsAfterDocumentScroll();
      }
    }

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

    // FIXME: The compositor, the layout viewport (updateLayoutViewport, with visual viewports) and the
    // editor, once there are those.
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
    // FIXME: Scrolls to a fragment's anchor, once there's navigation.
    _shouldScrollToFocusedElement = false;
    _delayedScrollToFocusedElementTimer.Stop();
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

    // FIXME: Scroll to the shadow host of an element in a user agent shadow root, and to the element a
    // focused element delegates its focus appearance to, once there are user agent shadow roots and form
    // controls.
    auto *renderer = focusedElement->Renderer();
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
