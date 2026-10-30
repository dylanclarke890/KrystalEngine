#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/Region.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformState.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLHtmlElement.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationFormattingContextLayout.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInitialContainingBlock.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderCounter.hpp"
#include "Krystal.Booey/Rendering/RenderGeometryMap.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerBacking.hpp"
#include "Krystal.Booey/Rendering/RenderLayerCompositor.hpp"
#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/FillLayers.hpp"

namespace krys::boo
{
  RenderView::RenderView(dom::Document &document, RenderStyle &&style) noexcept
      : RenderBlockFlow(Type::View, document, krys::move(style)), _frameView(*document.View()),
        _initialContainingBlock(CreateUnique<layout::InitialContainingBlock>(RenderStyle::Clone(Style()))),
        _layoutState(CreateUnique<layout::LayoutState>(
          document, *_initialContainingBlock, layout::LayoutState::Type::Primary,
          layout_integration::LayoutWithFormattingContextForBox,
          layout_integration::FormattingContextRootLogicalWidthForType,
          layout_integration::FormattingContextRootLogicalHeightForType,
          layout_integration::LayoutWithFormattingContextForBlockInInline)),
        _selection(*this)
  {
    // FIXME: We should find a way to enforce this at compile time.
    krys_debug_assert(document.View());

    // init RenderObject attributes
    SetInline(false);

    // FIXME: The preferred widths.

    SetPositionState(PositionType::Absolute); // to 0,0 :)

    krys_debug_assert(IsRenderView());
  }

  RenderView::~RenderView() noexcept
  {
    krys_debug_assert_msg(!_rendererCount, "All renderers should be in the process of being deleted.");
  }

  bool RenderView::IsChildAllowed(const RenderObject &child, const RenderStyle &) const noexcept
  {
    return child.IsRenderBox();
  }

  float RenderView::ZoomFactor() const noexcept
  {
    return _frameView.Frame().PageZoomFactor();
  }

  FloatSize RenderView::SizeForCSSSmallViewportUnits() const noexcept
  {
    return _frameView.SizeForCSSSmallViewportUnits();
  }

  FloatSize RenderView::SizeForCSSLargeViewportUnits() const noexcept
  {
    return _frameView.SizeForCSSLargeViewportUnits();
  }

  FloatSize RenderView::SizeForCSSDynamicViewportUnits() const noexcept
  {
    return _frameView.SizeForCSSDynamicViewportUnits();
  }

  FloatSize RenderView::SizeForCSSDefaultViewportUnits() const noexcept
  {
    return _frameView.SizeForCSSDefaultViewportUnits();
  }

  void RenderView::UpdateQuirksMode() noexcept
  {
    _layoutState->UpdateQuirksMode(GetDocument());
  }

  void RenderView::UpdateInitialContainingBlockSize() noexcept
  {
    // Initial containing block has no margin/padding/border.
    _layoutState->EnsureGeometryForBox(*_initialContainingBlock).SetContentBoxSize(FrameView().Size());
  }

  void RenderView::AddCounterNeedingUpdate(RenderCounter &renderer) noexcept
  {
    _countersNeedingUpdate.try_emplace(&renderer, CreateWeakPtr(&renderer));
  }

  RenderCounterSet RenderView::TakeCountersNeedingUpdate() noexcept
  {
    return std::exchange(_countersNeedingUpdate, {});
  }

#pragma region Painting

  void RenderView::UnregisterPositionTryBox(const RenderBox &box) noexcept
  {
    _positionTryBoxes.erase(&box);

    // Explicitly forget the last successful position option here, so if the box ever comes back (i.e display:
    // none to non-none), we don't accidentally reuse the last successful position option.
    if (auto styleable = style::Styleable::FromRenderer(box))
    {
      GetDocument().StyleScope().ForgetLastSuccessfulPositionOptionIndex(*styleable);
    }
  }

  void RenderView::Paint(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    // If we ever require layout but receive a paint anyway, something has gone horribly wrong.
    krys_debug_assert(!NeedsLayout());
    // RenderViews should never be called to paint with an offset not on device pixels.
    krys_debug_assert(LayoutPoint(IntPoint(paintOffset.X().ToInt(), paintOffset.Y().ToInt())) == paintOffset);

    // This avoids painting garbage between columns if there is a column gap.
    if (FrameView().GetPagination().Mode != PaginationMode::Unpaginated
        && paintInfo.ShouldPaintWithinRoot(*this))
    {
      paintInfo.Context().FillRect(FloatRect(paintInfo.Rect), FrameView().BaseBackgroundColor());
    }

    PaintObject(paintInfo, paintOffset);
  }

  RenderElement *RenderView::RendererForRootBackground() const noexcept
  {
    auto *firstChild = FirstChild();
    if (!firstChild)
    {
      return nullptr;
    }

    auto &documentRenderer = Downcast<RenderElement>(*firstChild);
    if (documentRenderer.HasBackground())
    {
      return &documentRenderer;
    }

    // We propagate the background only for HTML content.
    if (!Is<html::HTMLHtmlElement>(documentRenderer.GetElement()))
    {
      return &documentRenderer;
    }

    if (documentRenderer.ShouldApplyAnyContainment())
    {
      return nullptr;
    }

    if (auto body = GetDocument().Body())
    {
      if (auto *renderer = body->Renderer())
      {
        if (!renderer->ShouldApplyAnyContainment())
        {
          return renderer;
        }
      }
    }
    return &documentRenderer;
  }

  namespace
  {
    KRYS_NODISCARD bool RendererObscuresBackground(const RenderElement &rootElement) noexcept
    {
      const auto &style = rootElement.Style();
      if (style.UsedVisibility() != Visibility::Visible || !style.GetOpacity().IsOpaque()
          || style.HasTransform())
      {
        return false;
      }

      if (style.HasBorderRadius())
      {
        return false;
      }

      if (rootElement.IsComposited())
      {
        return false;
      }

      // FIXME: SVG roots with a clip path (isRenderOrLegacyRenderSVGRoot).

      auto *rendererForBackground = rootElement.View().RendererForRootBackground();
      if (!rendererForBackground)
      {
        return false;
      }

      if (rendererForBackground->Style().GetBackgroundLayers().UsedFirst().GetClip() == FillBox::Text)
      {
        return false;
      }

      return true;
    }
  }

  void RenderView::PaintBoxDecorations(PaintInfo &paintInfo, const LayoutPoint &) noexcept
  {
    if (!paintInfo.ShouldPaintWithinRoot(*this))
    {
      return;
    }

    // FIXME: Frames whose enclosing layers can't blit to the window (cannotBlitToWindow), once there are
    // subframes.

    if (!ShouldPaintBaseBackground())
    {
      return;
    }

    if (paintInfo.SkipRootBackground())
    {
      return;
    }

    bool rootFillsViewport = false;
    bool rootObscuresBackground = false;
    if (auto documentElement = GetDocument().DocumentElement())
    {
      if (auto *rootRenderer = documentElement->Renderer())
      {
        // The document element's renderer is currently forced to be a block, but may not always be.
        auto *rootBox = DynamicDowncast<RenderBox>(*rootRenderer);
        rootFillsViewport = rootBox && !rootBox->X() && !rootBox->Y() && rootBox->Width() >= Width()
                            && rootBox->Height() >= Height();
        rootObscuresBackground = RendererObscuresBackground(*rootRenderer);
      }
    }

    Compositor().RootBackgroundColorOrTransparencyChanged();

    auto *page = GetDocument().GetPage();
    float pageScaleFactor = page ? page->PageScaleFactor() : 1;

    // If painting will entirely fill the view, no need to fill the background.
    if (rootFillsViewport && rootObscuresBackground && pageScaleFactor >= 1
        && RootElementShouldPaintBaseBackground())
    {
      return;
    }

    // This code typically only executes if the root element's visibility has been set to hidden,
    // if there is a transform on the <html>, or if there is a page scale factor less than 1.
    // Only fill with a background color (typically white) if we're the root document,
    // since iframes/frames with no background in the child document should show the parent's background.
    // FIXME: The document's background colour when the background extends beyond the page
    // (backgroundShouldExtendBeyondPage), a setting that isn't ported.
    auto &frameView = FrameView();
    if (!frameView.IsTransparent())
    {
      const auto backgroundColor = frameView.BaseBackgroundColor();
      if (backgroundColor.IsVisible())
      {
        CompositeOperator previousOperator = paintInfo.Context().CompositeOperation();
        paintInfo.Context().SetCompositeOperation(CompositeOperator::Copy);
        paintInfo.Context().FillRect(FloatRect(paintInfo.Rect), backgroundColor);
        paintInfo.Context().SetCompositeOperation(previousOperator);
      }
      else
      {
        paintInfo.Context().ClearRect(FloatRect(paintInfo.Rect));
      }
    }
  }

  IntRect RenderView::UnscaledDocumentRect() const noexcept
  {
    LayoutRect overflowRect(LayoutOverflowRect());
    FlipForWritingMode(overflowRect);
    return SnappedIntRect(overflowRect);
  }

  IntRect RenderView::DocumentRect() const noexcept
  {
    FloatRect overflowRect(UnscaledDocumentRect());
    if (HasTransformRelatedProperty())
    {
      overflowRect = Layer()->CurrentTransform().MapRect(overflowRect);
    }
    return IntRect(overflowRect);
  }

  bool RenderView::ShouldPaintBaseBackground() const noexcept
  {
    // Fill with a base color if we're the root document.
    // FIXME: Subframes (frames, iframes whose colour scheme differs from their owner's, transparent SVG
    // documents), once there are any; every view is its page's main frame.
    return !FrameView().IsTransparent();
  }

  bool RenderView::RootElementShouldPaintBaseBackground() const noexcept
  {
    if (auto documentElement = GetDocument().DocumentElement())
    {
      if (auto *rootRenderer = documentElement->Renderer())
      {
        // The document element's renderer is currently forced to be a block, but may not always be.
        auto *rootBox = DynamicDowncast<RenderBox>(*rootRenderer);
        if (rootBox && rootBox->HasLayer())
        {
          RenderLayer *layer = rootBox->Layer();
          if (layer->IsolatesBlending() || layer->IsBackdropRoot())
          {
            return false;
          }
        }
      }
    }
    return ShouldPaintBaseBackground();
  }

  LayoutRect RenderView::UnextendedBackgroundRect() const noexcept
  {
    // FIXME: What is this? Need to patch for new columns?
    return UnscaledDocumentRect();
  }

  LayoutRect RenderView::BackgroundRect() const noexcept
  {
    // FIXME: New columns care about this?
    if (FrameView().HasExtendedBackgroundRectForPainting())
    {
      return FrameView().ExtendedBackgroundRectForPainting();
    }

    return UnextendedBackgroundRect();
  }

#pragma endregion

  void RenderView::Layout() noexcept
  {
    // FIXME: Printing (the page size and the preferred widths it sets), once there is any.

    // Use calcWidth/Height to get the new width/height, since this will take the full page zoom factor into
    // account.
    bool relayoutChildren =
      !ShouldUsePrintingLayout() && (Width() != ViewWidth() || Height() != ViewHeight());
    if (relayoutChildren)
    {
      SetChildNeedsLayout(MarkingBehavior::MarkOnlyThis);

      for (auto &box : ChildrenOfType<RenderBox>(*this))
      {
        // FIXME: SVG roots too.
        if (box.HasRelativeLogicalHeight() || box.Style().LogicalHeight().IsPercentOrCalculated()
            || box.Style().LogicalMinHeight().IsPercentOrCalculated()
            || box.Style().LogicalMaxHeight().IsPercentOrCalculated())
        {
          box.SetChildNeedsLayout(MarkingBehavior::MarkOnlyThis);
        }
      }
    }

    krys_debug_assert(!FrameView().LayoutContext().LayoutState());
    if (!NeedsLayout())
    {
      return;
    }

    // FIXME: The page height when printing and whether it changed, once there's printing.
    LayoutStateMaintainer statePusher(*this, {}, false);

    // FIXME: This should be called only when frame view (or the canvas we render onto) size changes.
    UpdateInitialContainingBlockSize();
    RenderBlockFlow::Layout();

#if KRYS_DEBUG
    FrameView().LayoutContext().CheckLayoutState();
#endif
  }

  RenderBox::LogicalExtentComputedValues RenderView::ComputeLogicalHeight(LayoutUnit logicalHeight,
                                                                          LayoutUnit) const noexcept
  {
    return {!ShouldUsePrintingLayout() ? LayoutUnit(ViewLogicalHeight()) : logicalHeight, 0_lu,
            ComputedMarginValues()};
  }

  int32 RenderView::ViewLogicalWidth() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? ViewWidth() : ViewHeight();
  }

  void RenderView::UpdateLogicalWidth() noexcept
  {
    // FIXME: The page width when printing, once there is any.
    SetLogicalWidth(LayoutUnit(ViewLogicalWidth()));
  }

  LayoutUnit RenderView::AvailableLogicalHeight(AvailableLogicalHeightType) const noexcept
  {
    // Make sure block progression pagination for percentages uses the column extent and
    // not the view's extent. See https://bugs.webkit.org/show_bug.cgi?id=135204.
    // FIXME: The column height of a multi-column view, once there are columns.

    auto layoutSize = _frameView.GetLayoutSize();
    return LayoutUnit(IsHorizontalWritingMode() ? layoutSize.Height() : layoutSize.Width());
  }

  LayoutUnit RenderView::PageOrViewLogicalHeight() const noexcept
  {
    // FIXME: The page height when printing, and the pagination's page length in a multi-column view, once
    // there are any.
    return LayoutUnit(ViewLogicalHeight());
  }

  int32 RenderView::ViewHeight() const noexcept
  {
    int32 height = 0;
    if (!ShouldUsePrintingLayout())
    {
      height = _frameView.LayoutHeight();
      height = _frameView.UseFixedLayout()
                 ? static_cast<int32>(std::ceil(Style().UsedZoom() * static_cast<float>(height)))
                 : height;
    }
    return height;
  }

  int32 RenderView::ViewWidth() const noexcept
  {
    int32 width = 0;
    if (!ShouldUsePrintingLayout())
    {
      width = _frameView.LayoutWidth();
      width = _frameView.UseFixedLayout()
                ? static_cast<int32>(std::ceil(Style().UsedZoom() * static_cast<float>(width)))
                : width;
    }
    return width;
  }

  int32 RenderView::ViewLogicalHeight() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? ViewHeight() : ViewWidth();
  }

  LayoutUnit RenderView::ClientLogicalWidthForFixedPosition() const noexcept
  {
    // FIXME: Fixed elements that lay out relative to the frame, and the layout viewport
    // (visualViewportEnabled), once the frame view scrolls.
    return ClientLogicalWidth();
  }

  LayoutUnit RenderView::ClientLogicalHeightForFixedPosition() const noexcept
  {
    // FIXME: Fixed elements that lay out relative to the frame, and the layout viewport
    // (visualViewportEnabled), once the frame view scrolls.
    return ClientLogicalHeight();
  }

  void RenderView::MapLocalToContainer(KRYS_MAYBE_UNUSED const RenderLayerModelObject *ancestorContainer,
                                       TransformState &transformState, EnumSet<MapCoordinatesMode> mode,
                                       KRYS_MAYBE_UNUSED bool *wasFixed) const noexcept
  {
    // If a container was specified, and was not nullptr or the RenderView,
    // then we should have found it by now.
    krys_debug_assert(!ancestorContainer || ancestorContainer == this);
    krys_debug_assert(!wasFixed || *wasFixed == mode.Contains(MapCoordinatesMode::IsFixed));

    if (mode.Contains(MapCoordinatesMode::IsFixed))
    {
      transformState.Move(ToLayoutSize(FrameView().ScrollPositionRespectingCustomFixedPosition()));
    }

    if (!ancestorContainer && mode.Contains(MapCoordinatesMode::UseTransforms)
        && ShouldUseTransformFromContainer(nullptr))
    {
      TransformationMatrix t;
      GetTransformFromContainer(LayoutSize(), t);
      transformState.ApplyTransform(t);
    }
  }

  const RenderElement *RenderView::PushMappingToContainer(const RenderLayerModelObject *ancestorToStopAt,
                                                          RenderGeometryMap &geometryMap) const noexcept
  {
    // If a container was specified, and was not nullptr or the RenderView, then we should have found it by
    // now.
    krys_debug_assert(!ancestorToStopAt || ancestorToStopAt == this);
    (void)ancestorToStopAt;

    LayoutPoint scrollPosition = FrameView().ScrollPositionRespectingCustomFixedPosition();

    if (!ancestorToStopAt && ShouldUseTransformFromContainer(nullptr))
    {
      TransformationMatrix t;
      GetTransformFromContainer(LayoutSize(), t);
      geometryMap.PushView(this, ToLayoutSize(scrollPosition), &t);
    }
    else
    {
      geometryMap.PushView(this, ToLayoutSize(scrollPosition));
    }

    return nullptr;
  }

  void RenderView::MapAbsoluteToLocalPoint(EnumSet<MapCoordinatesMode> mode,
                                           TransformState &transformState) const noexcept
  {
    if (mode.Contains(MapCoordinatesMode::UseTransforms) && ShouldUseTransformFromContainer(nullptr))
    {
      TransformationMatrix t;
      GetTransformFromContainer(LayoutSize(), t);
      transformState.ApplyTransform(t);
    }

    if (mode.Contains(MapCoordinatesMode::IsFixed))
    {
      transformState.Move(ToLayoutSize(FrameView().ScrollPositionRespectingCustomFixedPosition()));
    }
  }

  void RenderView::BoundingRects(List<LayoutRect> &rects, const LayoutPoint &accumulatedOffset) const noexcept
  {
    // FIXME: It's weird that this gets is size from the layer.
    rects.push_back(LayoutRect {accumulatedOffset, LayoutSize(Layer()->Size())});
  }

  void RenderView::AbsoluteQuads(List<FloatQuad> &quads, bool *wasFixed) const noexcept
  {
    if (wasFixed)
    {
      *wasFixed = false;
    }
    quads.push_back(FloatRect(FloatPoint(), FloatSize(Layer()->Size())));
  }

  dom::Node *RenderView::NodeForHitTest() const noexcept
  {
    return GetDocument().DocumentElement().get();
  }

  void RenderView::UpdateHitTestResult(HitTestResult &result, const LayoutPoint &point) const noexcept
  {
    if (result.InnerNode())
    {
      return;
    }

    // FIXME: Multi-column flows (their first column set's hit test result) come with multi-column layout.

    if (auto *node = NodeForHitTest())
    {
      result.SetInnerNode(node);
      if (!result.InnerNonSharedNode())
      {
        result.SetInnerNonSharedNode(node);
      }

      LayoutPoint adjustedPoint = point;
      OffsetForContents(adjustedPoint);

      result.SetLocalPoint(adjustedPoint);
    }
  }

  void RenderView::AddRendererWithPausedImageAnimations(RenderElement &renderer, CachedImage &image) noexcept
  {
    krys_debug_assert(!renderer.HasPausedImageAnimations()
                      || _renderersWithPausedImageAnimation.contains(&renderer));

    renderer.SetHasPausedImageAnimations(true);
    auto &images = _renderersWithPausedImageAnimation[&renderer];
    if (std::ranges::find(images, &image) == images.end())
    {
      images.push_back(&image);
    }
  }

  void RenderView::RemoveRendererWithPausedImageAnimations(RenderElement &renderer) noexcept
  {
    krys_debug_assert(renderer.HasPausedImageAnimations());
    krys_debug_assert(_renderersWithPausedImageAnimation.contains(&renderer));

    renderer.SetHasPausedImageAnimations(false);
    _renderersWithPausedImageAnimation.erase(&renderer);
  }

  void RenderView::RemoveRendererWithPausedImageAnimations(RenderElement &renderer,
                                                           CachedImage &image) noexcept
  {
    krys_debug_assert(renderer.HasPausedImageAnimations());

    auto it = _renderersWithPausedImageAnimation.find(&renderer);
    krys_debug_assert(it != _renderersWithPausedImageAnimation.end());

    auto &images = it->second;
    auto found = std::ranges::find(images, &image);
    if (found == images.end())
    {
      return;
    }

    if (images.size() == 1)
    {
      RemoveRendererWithPausedImageAnimations(renderer);
    }
    else
    {
      images.erase(found);
    }
  }

#pragma region Repaint

  bool RenderView::ShouldRepaint(const LayoutRect &rect) const noexcept
  {
    // FIXME: Nothing repaints while printing, once there's printing.
    return !rect.IsEmpty();
  }

  void RenderView::RepaintRootContents() noexcept
  {
    if (Layer()->IsComposited())
    {
      Layer()->SetBackingNeedsRepaint(GraphicsLayerShouldClipToLayer::DoNotClip);
      return;
    }

    // Always use layoutOverflowRect() to fix rdar://problem/27182267.
    // This should be cleaned up via webkit.org/b/159913 and webkit.org/b/159914.
    const auto *repaintContainer = ContainerForRepaint().Renderer;
    RepaintUsingContainer(repaintContainer, ComputeRectForRepaint(LayoutOverflowRect(), repaintContainer));
  }

  void RenderView::RepaintViewRectangle(const LayoutRect &repaintRect) noexcept
  {
    if (!ShouldRepaint(repaintRect))
    {
      return;
    }

    // FIXME: enclosingRect is needed as long as we integral snap ScrollView/FrameView/RenderWidget
    // size/position.
    auto enclosingRect = EnclosingIntRect(repaintRect);
    auto viewRect = ViewRect();

    // FIXME: A frame inside another frame repaints its owner's box, once there are iframes.

    if (_accumulatedRepaintRegion)
    {
      AccumulateRepaintRect(enclosingRect, EnclosingIntRect(viewRect));
    }
    else
    {
      FrameView().RepaintContentRectangle(enclosingRect);
    }
  }

  bool RenderView::AccumulateRepaintRect(IntRect rect, IntRect viewRect) noexcept
  {
    _accumulatedRepaintRegion->Unite(Region(rect));

    bool entireViewDirty = false;
    // Region will get slow if it gets too complex. Merge all rects so far to bounds if this happens.
    // FIXME: Maybe there should be a region type that does this automatically.
    constexpr uint32 maximumRepaintRegionGridSize = 16 * 16;
    if (_accumulatedRepaintRegion->GridSize() > maximumRepaintRegionGridSize)
    {
      auto dirtyBounds = _accumulatedRepaintRegion->Bounds();
      auto dirtyArea = static_cast<uint64>(dirtyBounds.Width()) * static_cast<uint64>(dirtyBounds.Height());
      auto viewArea = static_cast<uint64>(viewRect.Width()) * static_cast<uint64>(viewRect.Height());
      constexpr auto dirtyEntireViewThreshold = 0.75; // Derived empirically.
      if (static_cast<double>(dirtyArea) >= static_cast<double>(viewArea) * dirtyEntireViewThreshold)
      {
        dirtyBounds = viewRect;
        entireViewDirty = true;
      }

      _accumulatedRepaintRegion = CreateUnique<Region>(dirtyBounds);
    }
    else
    {
      entireViewDirty = rect.Contains(viewRect);
    }

    return entireViewDirty;
  }

  void RenderView::FlushAccumulatedRepaintRegion() const noexcept
  {
    // FIXME: A frame inside another frame repaints its owner's box, once there are iframes.

    krys_debug_assert(_accumulatedRepaintRegion);
    auto repaintRects = _accumulatedRepaintRegion->Rects();
    for (const auto &rect : repaintRects)
    {
      FrameView().RepaintContentRectangle(rect);
    }
    _accumulatedRepaintRegion = nullptr;
  }

  void RenderView::RepaintViewAndCompositedLayers() noexcept
  {
    RepaintRootContents();

    RenderLayerCompositor &compositor = Compositor();
    if (compositor.UsesCompositing())
    {
      compositor.RepaintCompositedLayers();
    }
  }

  bool RenderView::RootBackgroundIsEntirelyFixed() const noexcept
  {
    if (auto *rootBackgroundRenderer = RendererForRootBackground())
    {
      return style::HasEntirelyFixedBackground(rootBackgroundRenderer->Style().GetBackgroundLayers());
    }
    return false;
  }

  bool RenderView::UsesCompositing() const noexcept
  {
    return _compositor && _compositor->UsesCompositing();
  }

  RenderLayerCompositor &RenderView::Compositor() noexcept
  {
    if (!_compositor)
    {
      _compositor = CreateUnique<RenderLayerCompositor>(*this);
    }

    return *_compositor;
  }

  void RenderView::SetIsInWindow(bool isInWindow) noexcept
  {
    if (_compositor)
    {
      _compositor->SetIsInWindow(isInWindow);
    }
  }

  LayoutRect RenderView::ViewRect() const noexcept
  {
    if (ShouldUsePrintingLayout())
    {
      return LayoutRect(LayoutPoint(), Size());
    }
    return LayoutRect(FrameView().VisibleContentRect());
  }

  auto RenderView::ComputeVisibleRectsInContainer(const RepaintRects &rects,
                                                  KRYS_MAYBE_UNUSED const RenderLayerModelObject *container,
                                                  VisibleRectContext context) const noexcept
    -> Maybe<RepaintRects>
  {
    // If a container was specified, and was not nullptr or the RenderView,
    // then we should have found it by now.
    krys_debug_assert(!container || container == this);

    // FIXME: Rects stay as they are while printing, once there's printing.

    auto adjustedRects = rects;
    if (GetWritingMode().IsBlockFlipped())
    {
      // We have to flip by hand since the view's logical height has not been determined.  We
      // can use the viewport width and height.
      adjustedRects.FlipForWritingMode(LayoutSize(ViewWidth(), ViewHeight()),
                                       GetWritingMode().IsHorizontal());
    }

    if (context.HasPositionFixedDescendant)
    {
      adjustedRects.MoveBy(FrameView().ScrollPositionRespectingCustomFixedPosition());
    }

    // Apply our transform if we have one (because of full page zooming).
    if (!container && HasLayer() && Layer()->Transform())
    {
      adjustedRects.Transform(*Layer()->Transform(), GetDocument().DeviceScaleFactor());
    }

    return adjustedRects;
  }

  RenderView::RepaintRegionAccumulator::RepaintRegionAccumulator(RenderView *view) noexcept
  {
    if (!view)
    {
      return;
    }

    _wasAccumulatingRepaintRegion = !!view->_accumulatedRepaintRegion;
    if (!_wasAccumulatingRepaintRegion)
    {
      view->_accumulatedRepaintRegion = CreateUnique<Region>();
    }
    _view = CreateWeakPtr(view);
  }

  RenderView::RepaintRegionAccumulator::~RepaintRegionAccumulator() noexcept
  {
    if (_wasAccumulatingRepaintRegion)
    {
      return;
    }
    if (!_view)
    {
      return;
    }
    _view->FlushAccumulatedRepaintRegion();
  }

#pragma endregion

  void RenderView::ResumePausedImageAnimationsIfNeeded(const IntRect &visibleRect) noexcept
  {
    List<Pair<RenderElement *, CachedImage *>> toRemove;
    for (auto &[renderer, images] : _renderersWithPausedImageAnimation)
    {
      for (auto *image : images)
      {
        if (renderer->RepaintForPausedImageAnimationsIfNeeded(visibleRect, *image))
        {
          toRemove.emplace_back(renderer, image);
        }
      }
    }
    for (auto &[renderer, image] : toRemove)
    {
      RemoveRendererWithPausedImageAnimations(*renderer, *image);
    }

    // FIXME: SVG elements with paused animations.
  }
}
