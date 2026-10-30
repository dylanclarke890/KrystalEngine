#pragma once

#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"

namespace krys::boo::svg
{
  class SVGGraphicsElement;
}

namespace krys::boo
{
  /// @brief A block inside an SVG renderer tree (a `<foreignObject>`, and later SVG text), positioned and
  /// repainted in SVG coordinates.
  class RenderSVGBlock : public RenderBlockFlow
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderSVGBlock);

  public:
    KRYS_NODISCARD svg::SVGGraphicsElement &GraphicsElement() const noexcept;

  protected:
    RenderSVGBlock(Type, svg::SVGGraphicsElement &, RenderStyle &&) noexcept;
    ~RenderSVGBlock() noexcept override;

    void WillBeDestroyed() noexcept override;

    void ComputeOverflow(LayoutUnit oldClientAfterEdge,
                         EnumSet<ComputeOverflowOptions> options = {}) noexcept override;

    void UpdateFromStyle() noexcept override;
    void StyleDidChange(style::Difference, const RenderStyle *oldStyle) noexcept override;

  private:
    void GetElement() const = delete;

    void BoundingRects(List<LayoutRect> &, const LayoutPoint &accumulatedOffset) const noexcept override;
    void AbsoluteQuads(List<FloatQuad> &, bool *wasFixed) const noexcept override;

    KRYS_NODISCARD LayoutRect ClippedOverflowRect(const RenderLayerModelObject *repaintContainer,
                                                  VisibleRectContext) const noexcept final;
    KRYS_NODISCARD RepaintRects RectsForRepaintingAfterLayout(const RenderLayerModelObject *repaintContainer,
                                                              RepaintOutlineBounds) const noexcept final;

    KRYS_NODISCARD Maybe<FloatRect>
      ComputeFloatVisibleRectInContainer(const FloatRect &, const RenderLayerModelObject *container,
                                         VisibleRectContext) const noexcept final;
    KRYS_NODISCARD Maybe<RepaintRects>
      ComputeVisibleRectsInContainer(const RepaintRects &, const RenderLayerModelObject *container,
                                     VisibleRectContext) const noexcept final;

    void MapLocalToContainer(const RenderLayerModelObject *ancestorContainer, TransformState &,
                             EnumSet<MapCoordinatesMode>, bool *wasFixed) const noexcept final;
    const RenderElement *PushMappingToContainer(const RenderLayerModelObject *ancestorToStopAt,
                                                RenderGeometryMap &) const noexcept final;
    KRYS_NODISCARD LayoutSize
      OffsetFromContainer(const RenderElement &, const LayoutPoint &,
                          bool *offsetDependsOnPoint = nullptr) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderSVGBlock)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderSVGBlock();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
