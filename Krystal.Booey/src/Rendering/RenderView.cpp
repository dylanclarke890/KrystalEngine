#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformState.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLHtmlElement.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationFormattingContextLayout.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInitialContainingBlock.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderCounter.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"

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
          layout_integration::LayoutWithFormattingContextForBlockInInline))
  {
    // FIXME: We should find a way to enforce this at compile time.
    krys_debug_assert(document.View());

    // init RenderObject attributes
    SetInline(false);

    // FIXME: The preferred widths, once there's layout.

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

      // FIXME: Composited roots and SVG roots with a clip path (isComposited, isRenderOrLegacyRenderSVGRoot),
      // once there are either.

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

    // FIXME: The page scale factor, once pages scale.
    float pageScaleFactor = 1;

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
          // FIXME: Backdrop roots (isBackdropRoot), once there are backdrop filters.
          if (layer->IsolatesBlending())
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
        // FIXME: SVG roots too, once there's SVG.
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
}
