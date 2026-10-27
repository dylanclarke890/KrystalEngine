#include "Krystal.Booey/Rendering/SVG/RenderSVGBlock.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/Style/Values/Borders/Shadow.hpp"
#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"

namespace krys::boo
{
  RenderSVGBlock::RenderSVGBlock(Type type, svg::SVGGraphicsElement &element, RenderStyle &&style) noexcept
      : RenderBlockFlow(type, element, krys::move(style), {BlockFlowFlag::IsSVGBlock})
  {
  }

  RenderSVGBlock::~RenderSVGBlock() noexcept = default;

  svg::SVGGraphicsElement &RenderSVGBlock::GraphicsElement() const noexcept
  {
    return Downcast<svg::SVGGraphicsElement>(Downcast<svg::SVGElement>(*RenderBlockFlow::GetElement()));
  }

  void RenderSVGBlock::UpdateFromStyle() noexcept
  {
    RenderBlockFlow::UpdateFromStyle();

    // RenderSVGlock, used by Render(SVGText|ForeignObject), is not allowed to call
    // setHasNonVisibleOverflow(true). RenderBlock assumes a layer to be present when the overflow clip
    // functionality is requested. Both Render(SVGText|ForeignObject) return 'false' on 'requiresLayer'. Fine
    // for RenderSVGText.
    //
    // If we want to support overflow rules for <foreignObject> we can choose between two solutions:
    // a) make LegacyRenderSVGForeignObject require layers and SVG layer aware
    // b) refactor overflow logic out of RenderLayer (as suggested by dhyatt), which is a large task
    //
    // Until this is resolved, disable overflow support. Opera/FF don't support it as well at the moment (Feb
    // 2010).
    //
    // Note: This does NOT affect overflow handling on outer/inner <svg> elements - this is handled
    // manually by LegacyRenderSVGRoot - which owns the documents enclosing root layer and thus works fine.
    SetHasNonVisibleOverflow(false);
  }

  void RenderSVGBlock::BoundingRects(List<LayoutRect> &, const LayoutPoint &) const noexcept
  {
    // This code path should never be taken for SVG, as we're assuming useTransforms=true everywhere,
    // absoluteQuads should be used.
    krys_debug_assert_msg(false, "SVG blocks are mapped with their transforms (AbsoluteQuads)");
  }

  void RenderSVGBlock::AbsoluteQuads(List<FloatQuad> &quads, bool *wasFixed) const noexcept
  {
    quads.push_back(LocalToAbsoluteQuad(FloatRect {{}, Size()}, MapCoordinatesMode::UseTransforms, wasFixed));
  }

  void RenderSVGBlock::WillBeDestroyed() noexcept
  {
    // FIXME: Tell the resources cache the renderer was destroyed (SVGResourcesCache::clientDestroyed), with
    // resources (B11e).
    RenderBlockFlow::WillBeDestroyed();
  }

  void RenderSVGBlock::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    if (diff == style::DifferenceResult::Layout)
    {
      InvalidateCachedBoundaries();
    }
    RenderBlockFlow::StyleDidChange(diff, oldStyle);
    // FIXME: Tell the resources cache the style changed (SVGResourcesCache::clientStyleChanged), with
    // resources (B11e).
  }

  void RenderSVGBlock::ComputeOverflow(LayoutUnit oldClientAfterEdge,
                                       EnumSet<ComputeOverflowOptions> options) noexcept
  {
    RenderBlockFlow::ComputeOverflow(oldClientAfterEdge, options);

    const auto &textShadow = Style().GetTextShadow();
    if (textShadow.IsNone())
    {
      return;
    }

    auto borderRect = BorderBoxRect();
    style::AdjustRectForShadow(borderRect, textShadow, Style().UsedZoomForLength());
    AddVisualOverflow(LayoutRect(SnappedIntRect(borderRect)));
  }

  LayoutRect RenderSVGBlock::ClippedOverflowRect(const RenderLayerModelObject *repaintContainer,
                                                 VisibleRectContext context) const noexcept
  {
    return SVGRenderSupport::ClippedOverflowRectForRepaint(*this, repaintContainer, context);
  }

  auto RenderSVGBlock::RectsForRepaintingAfterLayout(const RenderLayerModelObject *repaintContainer,
                                                     RepaintOutlineBounds repaintOutlineBounds) const noexcept
    -> RepaintRects
  {
    auto rects = RepaintRects {SVGRenderSupport::ClippedOverflowRectForRepaint(
      *this, repaintContainer, VisibleRectContextForRepaint())};
    if (repaintOutlineBounds == RepaintOutlineBounds::Yes)
    {
      rects.OutlineBoundsRect = OutlineBoundsForRepaint(repaintContainer);
    }

    return rects;
  }

  auto RenderSVGBlock::ComputeVisibleRectsInContainer(const RepaintRects &rects,
                                                      const RenderLayerModelObject *container,
                                                      VisibleRectContext context) const noexcept
    -> Maybe<RepaintRects>
  {
    // FIXME: computeFloatVisibleRectInContainer() needs to be merged with computeVisibleRectsInContainer().
    auto adjustedRect =
      ComputeFloatVisibleRectInContainer(FloatRect(rects.ClippedOverflowRect), container, context);
    if (adjustedRect)
    {
      return RepaintRects {EnclosingLayoutRect(*adjustedRect)};
    }

    return null;
  }

  Maybe<FloatRect> RenderSVGBlock::ComputeFloatVisibleRectInContainer(
    const FloatRect &rect, const RenderLayerModelObject *container, VisibleRectContext context) const noexcept
  {
    return SVGRenderSupport::ComputeFloatVisibleRectInContainer(*this, rect, container, context);
  }

  void RenderSVGBlock::MapLocalToContainer(const RenderLayerModelObject *ancestorContainer,
                                           TransformState &transformState, EnumSet<MapCoordinatesMode>,
                                           bool *wasFixed) const noexcept
  {
    SVGRenderSupport::MapLocalToContainer(*this, ancestorContainer, transformState, wasFixed);
  }

  const RenderElement *RenderSVGBlock::PushMappingToContainer(const RenderLayerModelObject *ancestorToStopAt,
                                                              RenderGeometryMap &geometryMap) const noexcept
  {
    return SVGRenderSupport::PushMappingToContainer(*this, ancestorToStopAt, geometryMap);
  }

  LayoutSize RenderSVGBlock::OffsetFromContainer(const RenderElement &container, const LayoutPoint &,
                                                 bool *) const noexcept
  {
    krys_debug_assert(&container == Container());
    krys_debug_assert(!IsInFlowPositioned());
    krys_debug_assert(!IsAbsolutelyPositioned());
    krys_debug_assert(!IsInline());
    (void)container;
    return LocationOffset();
  }
}
