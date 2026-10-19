#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo
{
  KRYS_NODISCARD static const css::CSSOMStringAtom &ScreenAtom() noexcept
  {
    static MainThreadNeverDestroyed<css::CSSOMStringAtom> screen {u8"screen"};
    return screen;
  }

  LocalFrameView::LocalFrameView(LocalFrame &frame) noexcept
      : _frame(WeakPtr<LocalFrame>::WithRef(frame)), _mediaType(ScreenAtom()), _layoutContext(*this)
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
      AvailableContentSizeChanged();
    }

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
      AvailableContentSizeChanged();
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
      AvailableContentSizeChanged();
    }
  }

  IntRect LocalFrameView::VisibleContentRect() const noexcept
  {
    return UnobscuredContentRect();
  }

  IntRect LocalFrameView::UnobscuredContentRect() const noexcept
  {
    return IntRect(IntPoint(), Size().ExpandedTo(IntSize()));
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

  void LocalFrameView::AvailableContentSizeChanged() noexcept
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

    // FIXME: Update the layout viewport, once there's scrolling.
    _layoutContext.SetNeedsLayoutAfterViewConfigurationChange();
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
                                           KRYS_MAYBE_UNUSED bool insideFixed,
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

    // FIXME: Scroll the frame itself (scrollRectToVisibleInTopLevelView, scrollRectToVisibleInChildView),
    // once the frame view scrolls.
    return true;
  }

  void LocalFrameView::ScheduleScrollToFocusedElement(SelectionRevealMode selectionRevealMode) noexcept
  {
    if (selectionRevealMode == SelectionRevealMode::DoNotReveal)
    {
      return;
    }

    auto *document = Frame().Document();
    if (!document)
    {
      return;
    }

    document->UpdateLayoutIgnorePendingStylesheets();

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
    options.RevealMode = selectionRevealMode;
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

  DoublePoint LocalFrameView::WindowToContents(const DoublePoint &point) const noexcept
  {
    // FIXME: The scroll position, once the frame view scrolls; until then the view and its contents share an
    // origin.
    return point;
  }
}
