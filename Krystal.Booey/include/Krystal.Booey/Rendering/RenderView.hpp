#pragma once

#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::layout
{
  class InitialContainingBlock;
  class LayoutState;
}

namespace krys::boo
{
  class CachedImage;
  class LocalFrameView;
  class RenderCounter;

  class Region;

  /// @brief Counters whose text needs updating, keyed by address; the weak pointer tells whether the counter
  /// is still alive.
  using RenderCounterSet = HashMap<const RenderCounter *, WeakPtr<RenderCounter>>;

  /// @brief The root of a document's render tree, shown in the document's view.
  // FIXME: Selection, printing and the compositing bookkeeping come with selection, printing and compositing.
  class RenderView final : public RenderBlockFlow
  {
    KRYS_NON_COPY_MOVABLE(RenderView);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderView);

  private:
    LocalFrameView &_frameView;
    // Note that currently RenderView::layoutBox(), if it exists, is a child of _initialContainingBlock.
    const UniquePtr<layout::InitialContainingBlock> _initialContainingBlock;
    const UniquePtr<layout::LayoutState> _layoutState;
    uint64 _rendererCount {1};
    bool _hasQuotesNeedingUpdate {false};
    RenderCounterSet _countersNeedingUpdate;
    // Keyed by address: a box unregisters itself when it's destroyed or stops being a query container.
    HashSet<const RenderBox *> _containerQueryBoxes;
    // Renderers remove themselves when they're destroyed or stop being an image's client.
    HashMap<RenderElement *, List<CachedImage *>> _renderersWithPausedImageAnimation;
    mutable UniquePtr<Region> _accumulatedRepaintRegion;

  public:
    RenderView(dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderView() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderView";
    }

    KRYS_NODISCARD bool IsChildAllowed(const RenderObject &child,
                                       const RenderStyle &) const noexcept override;

    KRYS_NODISCARD float ZoomFactor() const noexcept;

    void UpdateHitTestResult(HitTestResult &result, const LayoutPoint &point) const noexcept override;
    /// @brief The root element.
    KRYS_NODISCARD dom::Node *NodeForHitTest() const noexcept override;

    KRYS_NODISCARD LocalFrameView &FrameView() const noexcept
    {
      return _frameView;
    }

    KRYS_NODISCARD layout::InitialContainingBlock &GetInitialContainingBlock() noexcept
    {
      return *_initialContainingBlock;
    }
    KRYS_NODISCARD const layout::InitialContainingBlock &GetInitialContainingBlock() const noexcept
    {
      return *_initialContainingBlock;
    }
    KRYS_NODISCARD layout::LayoutState &GetLayoutState() noexcept
    {
      return *_layoutState;
    }

    void UpdateQuirksMode() noexcept;

    KRYS_NODISCARD uint64 RendererCount() const noexcept
    {
      return _rendererCount;
    }
    void DidCreateRenderer() noexcept
    {
      ++_rendererCount;
    }
    void WillDestroyRenderer() noexcept
    {
      --_rendererCount;
    }

    void Layout() noexcept override;

#pragma region Painting

    void Paint(PaintInfo &, const LayoutPoint &) noexcept override;
    void PaintBoxDecorations(PaintInfo &, const LayoutPoint &) noexcept override;

    /// @brief The renderer whose background paints the canvas: the root element's, or the body's when the
    /// root has none (in HTML).
    KRYS_NODISCARD RenderElement *RendererForRootBackground() const noexcept;

    /// @brief The rect the root background is painted over.
    KRYS_NODISCARD LayoutRect BackgroundRect() const noexcept;
    KRYS_NODISCARD LayoutRect UnextendedBackgroundRect() const noexcept;
    KRYS_NODISCARD IntRect UnscaledDocumentRect() const noexcept;
    /// @brief The document's rect, transformed by the view's transform if it has one.
    KRYS_NODISCARD IntRect DocumentRect() const noexcept;

    /// @brief Whether the view fills with the base background colour first.
    KRYS_NODISCARD bool ShouldPaintBaseBackground() const noexcept;
    KRYS_NODISCARD bool RootElementShouldPaintBaseBackground() const noexcept;

#pragma endregion

    KRYS_NODISCARD bool RequiresLayer() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD LogicalExtentComputedValues
      ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept override;
    void UpdateLogicalWidth() noexcept override;
    KRYS_NODISCARD LayoutUnit
      AvailableLogicalHeight(AvailableLogicalHeightType heightType) const noexcept override;

    // The same as the FrameView's layoutHeight/layoutWidth but with null check guards.
    KRYS_NODISCARD int32 ViewHeight() const noexcept;
    KRYS_NODISCARD int32 ViewWidth() const noexcept;
    KRYS_NODISCARD int32 ViewLogicalWidth() const noexcept;
    KRYS_NODISCARD int32 ViewLogicalHeight() const noexcept;

    KRYS_NODISCARD LayoutUnit PageOrViewLogicalHeight() const noexcept;

    KRYS_NODISCARD LayoutUnit ClientLogicalWidthForFixedPosition() const noexcept;
    KRYS_NODISCARD LayoutUnit ClientLogicalHeightForFixedPosition() const noexcept;

    KRYS_NODISCARD bool ShouldUsePrintingLayout() const noexcept
    {
      // FIXME: Printing, once there is any.
      return false;
    }

    KRYS_NODISCARD bool HasQuotesNeedingUpdate() const noexcept
    {
      return _hasQuotesNeedingUpdate;
    }
    void SetHasQuotesNeedingUpdate(bool b) noexcept
    {
      _hasQuotesNeedingUpdate = b;
    }

    void AddCounterNeedingUpdate(RenderCounter &renderer) noexcept;
    KRYS_NODISCARD RenderCounterSet TakeCountersNeedingUpdate() noexcept;

    KRYS_NODISCARD FloatSize SizeForCSSSmallViewportUnits() const noexcept;
    KRYS_NODISCARD FloatSize SizeForCSSLargeViewportUnits() const noexcept;
    KRYS_NODISCARD FloatSize SizeForCSSDynamicViewportUnits() const noexcept;
    KRYS_NODISCARD FloatSize SizeForCSSDefaultViewportUnits() const noexcept;

    void RegisterContainerQueryBox(const RenderBox &box) noexcept
    {
      _containerQueryBoxes.insert(&box);
    }
    void UnregisterContainerQueryBox(const RenderBox &box) noexcept
    {
      _containerQueryBoxes.erase(&box);
    }
    /// @brief The boxes of the query containers.
    KRYS_NODISCARD const HashSet<const RenderBox *> &ContainerQueryBoxes() const noexcept
    {
      return _containerQueryBoxes;
    }

    void MapLocalToContainer(const RenderLayerModelObject *ancestorContainer, TransformState &transformState,
                             EnumSet<MapCoordinatesMode> mode, bool *wasFixed) const noexcept override;
    void MapAbsoluteToLocalPoint(EnumSet<MapCoordinatesMode> mode,
                                 TransformState &transformState) const noexcept override;
    void BoundingRects(List<LayoutRect> &rects, const LayoutPoint &accumulatedOffset) const noexcept override;
    void AbsoluteQuads(List<FloatQuad> &quads, bool *wasFixed = nullptr) const noexcept override;

#pragma region Repaint

    KRYS_NODISCARD Maybe<RepaintRects>
      ComputeVisibleRectsInContainer(const RepaintRects &rects, const RenderLayerModelObject *container,
                                     VisibleRectContext context) const noexcept override;
    /// @brief Repaints the whole of the view's contents (its layout overflow).
    void RepaintRootContents() noexcept;
    /// @brief Repaints @p repaintRect, in the view's (the contents') coordinates.
    void RepaintViewRectangle(const LayoutRect &repaintRect) noexcept;
    void RepaintViewAndCompositedLayers() noexcept;
    /// @brief The part of the contents the view shows.
    KRYS_NODISCARD LayoutRect ViewRect() const noexcept;

    /// @brief While one exists, repaints in the view are united into a region, and the frame view is told of
    /// it all at once when the outermost goes away.
    class RepaintRegionAccumulator
    {
      KRYS_NON_COPY_MOVABLE(RepaintRegionAccumulator);

    private:
      bool _wasAccumulatingRepaintRegion {false};
      WeakPtr<RenderView> _view;

    public:
      explicit RepaintRegionAccumulator(RenderView *view) noexcept;
      ~RepaintRegionAccumulator() noexcept;
    };

  private:
    KRYS_NODISCARD bool ShouldRepaint(const LayoutRect &rect) const noexcept;
    /// @return Whether the whole view is now dirty.
    bool AccumulateRepaintRect(IntRect rect, IntRect viewRect) noexcept;
    void FlushAccumulatedRepaintRegion() const noexcept;

  public:
#pragma endregion

    /// @brief Resumes the paused image animations of the renderers now visible in @p visibleRect.
    void ResumePausedImageAnimationsIfNeeded(const IntRect &visibleRect) noexcept;
    void AddRendererWithPausedImageAnimations(RenderElement &renderer, CachedImage &image) noexcept;
    void RemoveRendererWithPausedImageAnimations(RenderElement &renderer) noexcept;
    void RemoveRendererWithPausedImageAnimations(RenderElement &renderer, CachedImage &image) noexcept;

  private:
    void UpdateInitialContainingBlockSize() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderView)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderView();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
