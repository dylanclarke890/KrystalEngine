#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGModelObject.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/Style/Values/Borders/Shadow.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGLocatable.hpp"
#include "Krystal.Booey/SVG/SVGUseElement.hpp"

namespace krys::boo
{
  LegacyRenderSVGModelObject::LegacyRenderSVGModelObject(Type type, svg::SVGElement &element,
                                                         RenderStyle &&style,
                                                         EnumSet<SVGModelObjectFlag> typeFlags) noexcept
      : RenderElement(type, element, krys::move(style), {},
                      typeFlags | SVGModelObjectFlag::IsLegacy | SVGModelObjectFlag::UsesBoundaryCaching)
  {
    krys_debug_assert(IsLegacyRenderSVGModelObject());
    krys_debug_assert(!IsRenderSVGModelObject());
  }

  LegacyRenderSVGModelObject::~LegacyRenderSVGModelObject() noexcept = default;

  LayoutRect LegacyRenderSVGModelObject::ClippedOverflowRect(const RenderLayerModelObject *repaintContainer,
                                                             VisibleRectContext context) const noexcept
  {
    return SVGRenderSupport::ClippedOverflowRectForRepaint(*this, repaintContainer, context);
  }

  auto LegacyRenderSVGModelObject::RectsForRepaintingAfterLayout(
    const RenderLayerModelObject *repaintContainer, RepaintOutlineBounds repaintOutlineBounds) const noexcept
    -> RepaintRects
  {
    auto rects = RepaintRects {ClippedOverflowRect(repaintContainer, VisibleRectContextForRepaint())};
    if (repaintOutlineBounds == RepaintOutlineBounds::Yes)
    {
      rects.OutlineBoundsRect = OutlineBoundsForRepaint(repaintContainer);
    }

    return rects;
  }

  Maybe<FloatRect> LegacyRenderSVGModelObject::ComputeFloatVisibleRectInContainer(
    const FloatRect &rect, const RenderLayerModelObject *container, VisibleRectContext context) const noexcept
  {
    return SVGRenderSupport::ComputeFloatVisibleRectInContainer(*this, rect, container, context);
  }

  void LegacyRenderSVGModelObject::MapLocalToContainer(const RenderLayerModelObject *ancestorContainer,
                                                       TransformState &transformState,
                                                       EnumSet<MapCoordinatesMode>,
                                                       bool *wasFixed) const noexcept
  {
    SVGRenderSupport::MapLocalToContainer(*this, ancestorContainer, transformState, wasFixed);
  }

  const RenderElement *
    LegacyRenderSVGModelObject::PushMappingToContainer(const RenderLayerModelObject *ancestorToStopAt,
                                                       RenderGeometryMap &geometryMap) const noexcept
  {
    return SVGRenderSupport::PushMappingToContainer(*this, ancestorToStopAt, geometryMap);
  }

  namespace
  {
    void AdjustRectForOutlineAndShadow(const LegacyRenderSVGModelObject &renderer, LayoutRect &rect,
                                       const style::ZoomFactor &zoomFactor) noexcept
    {
      auto shadowRect = rect;
      if (renderer.Style().HasBoxShadow())
      {
        style::AdjustRectForShadow(shadowRect, renderer.Style().GetBoxShadow(), zoomFactor);
      }

      auto outlineRect = rect;
      auto outlineSize = LayoutUnit {renderer.OutlineStyleForRepaint().OutlineSize()};
      if (outlineSize)
      {
        outlineRect.Inflate(outlineSize);
      }

      rect = UnionRect(shadowRect, outlineRect);
    }
  }

  // Copied from RenderBox, this method likely requires further refactoring to work easily for both SVG and
  // CSS Box Model content.
  // FIXME: This may also need to move into SVGRenderSupport as the RenderBox version depends
  // on borderBoundingBox() which SVG RenderBox subclases (like SVGRenderBlock) do not implement.
  LayoutRect LegacyRenderSVGModelObject::OutlineBoundsForRepaint(
    const RenderLayerModelObject *repaintContainer) const noexcept
  {
    LayoutRect box = EnclosingLayoutRect(RepaintRectInLocalCoordinates());
    AdjustRectForOutlineAndShadow(*this, box, Style().UsedZoomForLength());

    FloatQuad containerRelativeQuad = LocalToContainerQuad(FloatRect(box), repaintContainer);
    return LayoutRect(SnapRectToDevicePixels(LayoutRect(containerRelativeQuad.BoundingBox()),
                                             GetDocument().DeviceScaleFactor()));
  }

  void LegacyRenderSVGModelObject::BoundingRects(List<LayoutRect> &rects,
                                                 const LayoutPoint &accumulatedOffset) const noexcept
  {
    auto rect = LayoutRect {StrokeBoundingBox()};
    rect.MoveBy(accumulatedOffset);
    rects.push_back(rect);
  }

  void LegacyRenderSVGModelObject::AbsoluteQuads(List<FloatQuad> &quads, bool *wasFixed) const noexcept
  {
    quads.push_back(LocalToAbsoluteQuad(StrokeBoundingBox(), MapCoordinatesMode::UseTransforms, wasFixed));
  }

  void LegacyRenderSVGModelObject::WillBeDestroyed() noexcept
  {
    SVGResourcesCache::ClientDestroyed(*this);
    RenderElement::WillBeDestroyed();
  }

  void LegacyRenderSVGModelObject::StyleDidChange(style::Difference diff,
                                                  const RenderStyle *oldStyle) noexcept
  {
    if (diff == style::DifferenceResult::Layout)
    {
      InvalidateCachedBoundaries();
      if (Style().AffectsTransform() || (oldStyle && oldStyle->AffectsTransform()))
      {
        SetNeedsTransformUpdate();
      }
    }
    RenderElement::StyleDidChange(diff, oldStyle);
    SVGResourcesCache::ClientStyleChanged(*this, diff, oldStyle, Style());
  }

  bool LegacyRenderSVGModelObject::NodeAtPoint(const HitTestRequest &, HitTestResult &,
                                               const HitTestLocation &, const LayoutPoint &,
                                               HitTestAction) noexcept
  {
    krys_debug_assert(false);
    return false;
  }

  namespace
  {
    void GetElementCTM(svg::SVGElement *element, AffineTransform &transform) noexcept
    {
      krys_debug_assert(element);

      auto *stopAtElement = svg::SVGLocatable::NearestViewportElement(element);
      krys_debug_assert(stopAtElement);

      AffineTransform localTransform;
      dom::Node *current = element;

      while (auto *currentElement = DynamicDowncast<svg::SVGElement>(current))
      {
        localTransform = currentElement->Renderer()->LocalToParentTransform();
        transform = localTransform.Multiply(transform);
        // For getCTM() computation, stop at the nearest viewport element
        if (currentElement == stopAtElement)
        {
          break;
        }

        current = current->ParentOrShadowHostNode();
      }
    }

    // FloatRect::intersects does not consider horizontal or vertical lines (because of isEmpty()).
    // So special-case handling of such lines.
    KRYS_NODISCARD bool IntersectsAllowingEmpty(const FloatRect &r, const FloatRect &other) noexcept
    {
      if (r.IsEmpty() && other.IsEmpty())
      {
        return false;
      }
      if (r.IsEmpty() && !other.IsEmpty())
      {
        return (other.Contains(r.X(), r.Y()) && !other.Contains(r.MaxX(), r.MaxY()))
               || (!other.Contains(r.X(), r.Y()) && other.Contains(r.MaxX(), r.MaxY()));
      }
      if (other.IsEmpty() && !r.IsEmpty())
      {
        return IntersectsAllowingEmpty(other, r);
      }
      return r.Intersects(other);
    }

    // One of the element types that can cause graphics to be drawn onto the target canvas. Specifically:
    // circle, ellipse, image, line, path, polygon, polyline, rect, text and use.
    KRYS_NODISCARD bool IsGraphicsElement(const RenderElement &renderer) noexcept
    {
      return renderer.IsLegacyRenderSVGShape() || renderer.IsRenderSVGText()
             || renderer.IsLegacyRenderSVGImage()
             || Is<svg::SVGUseElement>(Downcast<svg::SVGElement>(*renderer.GetElement()));
    }
  }

  bool LegacyRenderSVGModelObject::CheckIntersection(RenderElement *renderer, const FloatRect &rect) noexcept
  {
    if (!renderer || renderer->Style().UsedPointerEvents() == PointerEvents::None)
    {
      return false;
    }
    if (!IsGraphicsElement(*renderer))
    {
      return false;
    }
    AffineTransform ctm;
    auto *svgElement = Downcast<svg::SVGElement>(renderer->GetElement());
    GetElementCTM(svgElement, ctm);
    krys_debug_assert(svgElement->Renderer());
    // FIXME: [SVG] checkEnclosure implementation is inconsistent
    // https://bugs.webkit.org/show_bug.cgi?id=262709
    return IntersectsAllowingEmpty(rect, ctm.MapRect(svgElement->Renderer()->RepaintRectInLocalCoordinates(
                                           RepaintRectCalculation::Accurate)));
  }

  bool LegacyRenderSVGModelObject::CheckEnclosure(RenderElement *renderer, const FloatRect &rect) noexcept
  {
    if (!renderer || renderer->Style().UsedPointerEvents() == PointerEvents::None)
    {
      return false;
    }
    if (!IsGraphicsElement(*renderer))
    {
      return false;
    }
    AffineTransform ctm;
    auto *svgElement = Downcast<svg::SVGElement>(renderer->GetElement());
    GetElementCTM(svgElement, ctm);
    krys_debug_assert(svgElement->Renderer());
    // FIXME: [SVG] checkEnclosure implementation is inconsistent
    // https://bugs.webkit.org/show_bug.cgi?id=262709
    return rect.Contains(
      ctm.MapRect(svgElement->Renderer()->RepaintRectInLocalCoordinates(RepaintRectCalculation::Accurate)));
  }

  void LegacyRenderSVGModelObject::AddFocusRingRects(List<LayoutRect> &, const LayoutPoint &,
                                                     const RenderLayerModelObject *) const noexcept
  {
  }

  svg::SVGElement &LegacyRenderSVGModelObject::GetElement() const noexcept
  {
    return Downcast<svg::SVGElement>(NodeForNonAnonymous());
  }
}
