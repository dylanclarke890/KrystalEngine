#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Platform/ScrollAlignment.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
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

  IntRect LocalFrameView::VisibleContentRectInternal(VisibleContentRectIncludesScrollbars) const noexcept
  {
    // FIXME: Without the scrollbars, once there are scrollbars.
    return UnobscuredContentRect();
  }

  IntRect LocalFrameView::UnobscuredContentRect() const noexcept
  {
    return IntRect(_scrollPosition, Size().ExpandedTo(IntSize()));
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

    // FIXME: The overhang areas, the scrollbars and the pan scroll icon, once there are rubber-banding,
    // scrollbars and pan scrolling.
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

    // FIXME: Not while rubber-banding, once there's rubber-banding.
    auto adjustedScrollPosition = AdjustScrollPositionWithinRange(desiredPosition);
    if (adjustedScrollPosition != GetScrollPosition() || ScrollOriginChanged())
    {
      (void)ScrollToPositionWithoutAnimation(adjustedScrollPosition);
      ResetScrollOriginChanged();
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

    // FIXME: The compositor, the layout viewport (updateLayoutViewport, with visual viewports), what's
    // relevant to the viewport (viewportContentsChanged: content visibility, lazy loading, intersection
    // observers) and the editor, once there are those.
  }

  void LocalFrameView::ScheduleScrollEvent() noexcept
  {
    SetIsAwaitingScrollend(true);
    Frame().GetEventHandler().ScheduleScrollEvent();
    // FIXME: A fake mouse move soon (dispatchFakeMouseMoveEventSoon), so the hovered element follows the
    // content under a still mouse.
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
