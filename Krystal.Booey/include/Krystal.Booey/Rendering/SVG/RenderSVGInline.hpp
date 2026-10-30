#pragma once

#include "Krystal.Booey/Rendering/RenderInline.hpp"

namespace krys::boo::svg
{
  class SVGGraphicsElement;
}

namespace krys::boo
{
  /// @brief An inline SVG text content element (`<tspan>`, `<textPath>`, `<a>` inside text), laid out by its
  /// `<text>`.
  class RenderSVGInline : public RenderInline
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderSVGInline);

  public:
    RenderSVGInline(Type, svg::SVGGraphicsElement &, RenderStyle &&) noexcept;
    ~RenderSVGInline() noexcept override;

    KRYS_NODISCARD svg::SVGGraphicsElement &GraphicsElement() const noexcept;

    KRYS_NODISCARD bool IsChildAllowed(const RenderObject &, const RenderStyle &) const noexcept override;

  private:
    void GetElement() const = delete;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGInline";
    }
    KRYS_NODISCARD bool RequiresLayer() const noexcept final
    {
      return false;
    }

    void UpdateFromStyle() noexcept final;

    // Chapter 10.4 of the SVG Specification say that we should use the
    // object bounding box of the parent text element.
    // We search for the root text element and take its bounding box.
    // It is also necessary to take the stroke and repaint rect of
    // this element, since we need it for filters.
    KRYS_NODISCARD FloatRect ObjectBoundingBox() const noexcept final;
    KRYS_NODISCARD FloatRect StrokeBoundingBox() const noexcept final;
    KRYS_NODISCARD FloatRect RepaintRectInLocalCoordinates(
      RepaintRectCalculation = RepaintRectCalculation::Fast) const noexcept final;
    KRYS_NODISCARD FloatRect DecoratedBoundingBox() const noexcept final;

    KRYS_NODISCARD LayoutRect ClippedOverflowRect(const RenderLayerModelObject *repaintContainer,
                                                  VisibleRectContext) const noexcept final;
    KRYS_NODISCARD RepaintRects RectsForRepaintingAfterLayout(const RenderLayerModelObject *repaintContainer,
                                                              RepaintOutlineBounds) const noexcept final;

    KRYS_NODISCARD Maybe<FloatRect>
      ComputeFloatVisibleRectInContainer(const FloatRect &, const RenderLayerModelObject *container,
                                         VisibleRectContext) const noexcept final;

    void MapLocalToContainer(const RenderLayerModelObject *ancestorContainer, TransformState &,
                             EnumSet<MapCoordinatesMode>, bool *wasFixed) const noexcept final;
    const RenderElement *PushMappingToContainer(const RenderLayerModelObject *ancestorToStopAt,
                                                RenderGeometryMap &) const noexcept final;
    void AbsoluteQuads(List<FloatQuad> &, bool *wasFixed) const noexcept final;

    KRYS_NODISCARD UniquePtr<LegacyInlineFlowBox> CreateInlineFlowBox() noexcept final;

    void WillBeDestroyed() noexcept final;
    void StyleDidChange(style::Difference, const RenderStyle *oldStyle) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderSVGInline)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderSVGInline();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
