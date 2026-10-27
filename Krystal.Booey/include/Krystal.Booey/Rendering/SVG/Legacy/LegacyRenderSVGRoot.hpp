#pragma once

#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Rendering/RenderReplaced.hpp"
#include "Krystal.Core/Types/Markable.hpp"

namespace krys::boo::svg
{
  class SVGSvgElement;
}

namespace krys::boo
{
  /// @brief The renderer of an outermost `<svg>`: a replaced box that lays out and paints its SVG children.
  class LegacyRenderSVGRoot final : public RenderReplaced
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGRoot);

  private:
    IntSize _containerSize {};
    FloatRect _repaintBoundingBox {};
    Markable<FloatRect> _objectBoundingBox {};
    mutable Markable<FloatRect> _strokeBoundingBox {};
    mutable Markable<FloatRect> _accurateRepaintBoundingBox {};
    mutable AffineTransform _localToParentTransform {};
    AffineTransform _localToBorderBoxTransform {};
    // FIXME: The resources needing to invalidate their clients (m_resourcesNeedingToInvalidateClients), with
    // resources (B11e).

    bool _inLayout {false};
    bool _isLayoutSizeChanged : 1 {false};
    bool _needsBoundariesOrTransformUpdate : 1 {true};
    bool _hasBoxDecorations : 1 {false};

  public:
    LegacyRenderSVGRoot(svg::SVGSvgElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGRoot() noexcept override;

    KRYS_NODISCARD svg::SVGSvgElement &SvgSvgElement() const noexcept;

    KRYS_NODISCARD bool IsEmbeddedThroughSVGImage() const noexcept;

    KRYS_NODISCARD FloatSize ComputeIntrinsicSize() const noexcept final;
    KRYS_NODISCARD FloatSize PreferredAspectRatio() const noexcept final;
    KRYS_NODISCARD bool HasIntrinsicAspectRatio() const noexcept final;

    KRYS_NODISCARD bool IsLayoutSizeChanged() const noexcept
    {
      return _isLayoutSizeChanged;
    }
    KRYS_NODISCARD bool IsInLayout() const noexcept
    {
      return _inLayout;
    }
    void SetNeedsBoundariesUpdate() noexcept override
    {
      _needsBoundariesOrTransformUpdate = true;
    }
    void SetNeedsTransformUpdate() noexcept override
    {
      _needsBoundariesOrTransformUpdate = true;
    }

    KRYS_NODISCARD IntSize ContainerSize() const noexcept
    {
      return _containerSize;
    }
    void SetContainerSize(const IntSize &containerSize) noexcept
    {
      _containerSize = containerSize;
    }

    KRYS_NODISCARD bool HasRelativeDimensions() const noexcept override;

    // localToBorderBoxTransform maps local SVG viewport coordinates to local CSS box coordinates.
    KRYS_NODISCARD const AffineTransform &LocalToBorderBoxTransform() const noexcept
    {
      return _localToBorderBoxTransform;
    }

    // FIXME: addResourceForClientInvalidation, with resources (B11e).

  private:
    void GetElement() const = delete;

    // Intentially left 'RenderSVGRoot' instead of 'LegacyRenderSVGRoot', to avoid breaking layout tests.
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGRoot";
    }

    KRYS_NODISCARD LayoutUnit ComputeReplacedLogicalWidth(
      ShouldComputePreferred = ShouldComputePreferred::ComputeActual) const noexcept override;
    KRYS_NODISCARD LayoutUnit
      ComputeReplacedLogicalHeight(Maybe<LayoutUnit> estimatedUsedWidth = null) const noexcept override;
    void Layout() noexcept override;
    void PaintReplaced(PaintInfo &, const LayoutPoint &) noexcept override;

    void WillBeDestroyed() noexcept override;

    void InsertedIntoTree() noexcept override;
    void WillBeRemovedFromTree() noexcept override;

    void StyleDidChange(style::Difference, const RenderStyle *oldStyle) noexcept override;

    KRYS_NODISCARD const AffineTransform &LocalToParentTransform() const noexcept override;

    KRYS_NODISCARD FloatRect ObjectBoundingBox() const noexcept override
    {
      return _objectBoundingBox.value_or(FloatRect());
    }
    KRYS_NODISCARD FloatRect StrokeBoundingBox() const noexcept override;
    KRYS_NODISCARD FloatRect RepaintRectInLocalCoordinates(
      RepaintRectCalculation = RepaintRectCalculation::Fast) const noexcept override;
    KRYS_NODISCARD FloatRect DecoratedBoundingBox() const noexcept override;

    bool NodeAtPoint(const HitTestRequest &, HitTestResult &, const HitTestLocation &locationInContainer,
                     const LayoutPoint &accumulatedOffset, HitTestAction) noexcept override;

    KRYS_NODISCARD LayoutRect ClippedOverflowRect(const RenderLayerModelObject *repaintContainer,
                                                  VisibleRectContext) const noexcept override;
    KRYS_NODISCARD RepaintRects RectsForRepaintingAfterLayout(const RenderLayerModelObject *repaintContainer,
                                                              RepaintOutlineBounds) const noexcept override;

    KRYS_NODISCARD LayoutRect LocalClippedOverflowRect(RepaintRectCalculation) const noexcept;

    KRYS_NODISCARD LayoutRect ComputeContentsInkOverflow() const noexcept;

    KRYS_NODISCARD Maybe<FloatRect>
      ComputeFloatVisibleRectInContainer(const FloatRect &, const RenderLayerModelObject *container,
                                         VisibleRectContext) const noexcept override;

    void MapLocalToContainer(const RenderLayerModelObject *ancestorContainer, TransformState &,
                             EnumSet<MapCoordinatesMode>, bool *wasFixed) const noexcept override;
    const RenderElement *PushMappingToContainer(const RenderLayerModelObject *ancestorToStopAt,
                                                RenderGeometryMap &) const noexcept override;

    KRYS_NODISCARD bool CanBeSelectionLeaf() const noexcept override
    {
      return false;
    }
    KRYS_NODISCARD bool CanHaveChildren() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD bool ShouldApplyViewportClip() const noexcept;
    void UpdateCachedBoundaries() noexcept;
    void BuildLocalToBorderBoxTransform() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGRoot)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGRoot();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
