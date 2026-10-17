#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformState.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationFormattingContextLayout.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInitialContainingBlock.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
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

    // FIXME: Fixed boxes move with the view's scroll position (scrollPositionRespectingCustomFixedPosition),
    // once the frame view scrolls.

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

    // FIXME: Fixed boxes move with the view's scroll position, once the frame view scrolls.
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
}
