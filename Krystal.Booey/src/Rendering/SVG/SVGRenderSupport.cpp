#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformState.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderAncestorIterator.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderGeometryMap.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGForeignObject.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGImage.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceClipper.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceMasker.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGShape.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGTransformableContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGViewportContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResources.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Booey/Style/Values/Shapes/PathComputation.hpp"
#include "Krystal.Booey/Style/Values/Shapes/WindRuleComputation.hpp"
#include "Krystal.Booey/SVG/SVGGeometryElement.hpp"
#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"

namespace krys::boo
{
  LayoutRect SVGRenderSupport::ClippedOverflowRectForRepaint(const RenderElement &renderer,
                                                             const RenderLayerModelObject *repaintContainer,
                                                             VisibleRectContext context) noexcept
  {
    // Return early for any cases where we don't actually paint
    if (renderer.IsInsideEntirelyHiddenLayer())
    {
      return {};
    }

    // Pass our local paint rect to computeFloatVisibleRectInContainer() which will
    // map to parent coords and recurse up the parent chain.
    return EnclosingLayoutRect(renderer.ComputeFloatRectForRepaint(
      renderer.RepaintRectInLocalCoordinates(context.GetRepaintRectCalculation()), repaintContainer));
  }

  Maybe<FloatRect>
    SVGRenderSupport::ComputeFloatVisibleRectInContainer(const RenderElement &renderer, const FloatRect &rect,
                                                         const RenderLayerModelObject *container,
                                                         VisibleRectContext context) noexcept
  {
    // Ensure our parent is an SVG object.
    krys_debug_assert(renderer.Parent());
    auto &parent = *renderer.Parent();
    if (!Is<svg::SVGElement>(parent.GetElement()))
    {
      return FloatRect();
    }

    FloatRect adjustedRect = rect;
    adjustedRect.Inflate(style::Evaluate<float>(renderer.Style().OutlineWidth(), style::ZoomNeeded {}));

    // Translate to coords in our parent renderer, and then call computeFloatVisibleRectInContainer() on our
    // parent.
    adjustedRect = renderer.LocalToParentTransform().MapRect(adjustedRect);

    return parent.ComputeFloatVisibleRectInContainer(adjustedRect, container, context);
  }

  const RenderElement &SVGRenderSupport::LocalToParentTransform(const RenderElement &renderer,
                                                                AffineTransform &transform) noexcept
  {
    krys_debug_assert(renderer.Parent());
    auto &parent = *renderer.Parent();

    // At the SVG/HTML boundary (aka LegacyRenderSVGRoot), we apply the localToBorderBoxTransform
    // to map an element from SVG viewport coordinates to CSS box coordinates.
    if (auto *svgRoot = DynamicDowncast<LegacyRenderSVGRoot>(parent))
    {
      transform = svgRoot->LocalToBorderBoxTransform() * renderer.LocalToParentTransform();
    }
    else
    {
      transform = renderer.LocalToParentTransform();
    }

    return parent;
  }

  void SVGRenderSupport::MapLocalToContainer(const RenderElement &renderer,
                                             const RenderLayerModelObject *ancestorContainer,
                                             TransformState &transformState, bool *wasFixed) noexcept
  {
    if (ancestorContainer == &renderer)
    {
      return;
    }

    AffineTransform transform;
    auto &parent = LocalToParentTransform(renderer, transform);

    transformState.ApplyTransform(transform);

    EnumSet<MapCoordinatesMode> mode = MapCoordinatesMode::UseTransforms;
    parent.MapLocalToContainer(ancestorContainer, transformState, mode, wasFixed);
  }

  const RenderElement *
    SVGRenderSupport::PushMappingToContainer(const RenderElement &renderer,
                                             const RenderLayerModelObject *ancestorToStopAt,
                                             RenderGeometryMap &geometryMap) noexcept
  {
    krys_debug_assert(ancestorToStopAt != &renderer);
    (void)ancestorToStopAt;

    AffineTransform transform;
    auto &parent = LocalToParentTransform(renderer, transform);

    geometryMap.Push(&renderer, TransformationMatrix(transform));
    return &parent;
  }

  LayoutRepainter::CheckForRepaint
    SVGRenderSupport::CheckForSVGRepaintDuringLayout(const RenderElement &renderer) noexcept
  {
    if (!renderer.CheckForRepaintDuringLayout())
    {
      return LayoutRepainter::CheckForRepaint::No;
    }
    // When a parent container is transformed in SVG, all children will be painted automatically
    // so we are able to skip redundant repaint checks.
    if (auto *parent = DynamicDowncast<LegacyRenderSVGContainer>(renderer.Parent()))
    {
      if (parent->IsRepaintSuspendedForChildren() || parent->DidTransformToRootUpdate())
      {
        return LayoutRepainter::CheckForRepaint::No;
      }
    }
    return LayoutRepainter::CheckForRepaint::Yes;
  }

  namespace
  {
    // https://svgwg.org/svg2-draft/coords.html#BoundingBoxes
    KRYS_NODISCARD bool HasValidBoundingBoxForContainer(const RenderObject &object) noexcept
    {
      if (auto *shape = DynamicDowncast<LegacyRenderSVGShape>(object))
      {
        return !shape->IsRenderingDisabled();
      }

      if (auto *text = DynamicDowncast<RenderSVGText>(object))
      {
        return text->IsObjectBoundingBoxValid();
      }

      if (auto *container = DynamicDowncast<LegacyRenderSVGContainer>(object))
      {
        return !container->IsLegacyRenderSVGHiddenContainer();
      }

      if (auto *foreignObject = DynamicDowncast<LegacyRenderSVGForeignObject>(object))
      {
        return foreignObject->IsObjectBoundingBoxValid();
      }

      if (auto *image = DynamicDowncast<LegacyRenderSVGImage>(object))
      {
        return image->IsObjectBoundingBoxValid();
      }

      return false;
    }
  }

  auto SVGRenderSupport::ComputeContainerBoundingBoxes(const RenderElement &container,
                                                       RepaintRectCalculation repaintRectCalculation) noexcept
    -> ContainerBoundingBoxes
  {
    ContainerBoundingBoxes result;

    for (auto &current : ChildrenOfType<RenderObject>(container))
    {
      if (!HasValidBoundingBoxForContainer(current))
      {
        continue;
      }

      auto &transform = current.LocalToParentTransform();

      auto repaintRect = current.RepaintRectInLocalCoordinates(repaintRectCalculation);
      if (!transform.IsIdentity())
      {
        repaintRect = transform.MapRect(repaintRect);
      }
      if (repaintRect.IsNaN())
      {
        continue;
      }

      result.RepaintBoundingBox.Unite(repaintRect);

      if (auto *childContainer = DynamicDowncast<LegacyRenderSVGContainer>(current);
          childContainer && !childContainer->IsObjectBoundingBoxValid())
      {
        continue;
      }

      auto objectBounds = current.ObjectBoundingBox();
      if (!transform.IsIdentity())
      {
        objectBounds = transform.MapRect(objectBounds);
      }

      if (!result.ObjectBoundingBox)
      {
        result.ObjectBoundingBox = objectBounds;
      }
      else
      {
        result.ObjectBoundingBox->UniteEvenIfEmpty(objectBounds);
      }
    }

    krys_debug_assert(!result.RepaintBoundingBox.IsNaN());
    return result;
  }

  FloatRect SVGRenderSupport::ComputeContainerStrokeBoundingBox(const RenderElement &container) noexcept
  {
    krys_debug_assert(container.IsLegacyRenderSVGRoot() || container.IsLegacyRenderSVGContainer());
    FloatRect strokeBoundingBox;

    for (auto &current : ChildrenOfType<RenderObject>(container))
    {
      if (current.IsLegacyRenderSVGHiddenContainer())
      {
        continue;
      }

      // Don't include elements in the union that do not render.
      if (auto *shape = DynamicDowncast<LegacyRenderSVGShape>(current); shape && shape->IsRenderingDisabled())
      {
        continue;
      }

      FloatRect childStrokeBoundingBox = current.StrokeBoundingBox();
      krys_debug_assert(!childStrokeBoundingBox.IsNaN());

      if (auto *currentElement = DynamicDowncast<RenderElement>(current))
      {
        SVGRenderSupport::IntersectRepaintRectWithResources(*currentElement, childStrokeBoundingBox,
                                                            RepaintRectCalculation::Accurate);
      }
      const AffineTransform &transform = current.LocalToParentTransform();
      if (!transform.IsIdentity())
      {
        childStrokeBoundingBox = transform.MapRect(childStrokeBoundingBox);
      }

      if (!childStrokeBoundingBox.IsNaN())
      {
        strokeBoundingBox.Unite(childStrokeBoundingBox);
      }
    }

    krys_debug_assert(!strokeBoundingBox.IsNaN());
    return strokeBoundingBox;
  }

  bool SVGRenderSupport::PaintInfoIntersectsRepaintRect(const FloatRect &localRepaintRect,
                                                        const AffineTransform &localTransform,
                                                        const PaintInfo &paintInfo) noexcept
  {
    if (localTransform.IsIdentity())
    {
      return localRepaintRect.Intersects(paintInfo.Rect);
    }

    return localTransform.MapRect(localRepaintRect).Intersects(paintInfo.Rect);
  }

  LegacyRenderSVGRoot *SVGRenderSupport::FindTreeRootObject(RenderElement &start) noexcept
  {
    return LineageOfType<LegacyRenderSVGRoot>(start).First();
  }

  const LegacyRenderSVGRoot *SVGRenderSupport::FindTreeRootObject(const RenderElement &start) noexcept
  {
    return LineageOfType<LegacyRenderSVGRoot>(start).First();
  }

  namespace
  {
    void InvalidateResourcesOfChildren(RenderElement &renderer) noexcept
    {
      krys_debug_assert(!renderer.NeedsLayout());
      if (auto *resources = SVGResourcesCache::CachedResourcesForRenderer(renderer))
      {
        resources->RemoveClientFromCacheAndMarkForInvalidation(renderer, false);
      }

      for (auto &child : ChildrenOfType<RenderElement>(renderer))
      {
        InvalidateResourcesOfChildren(child);
      }
    }

    KRYS_NODISCARD bool LayoutSizeOfNearestViewportChanged(const RenderElement &renderer) noexcept
    {
      for (auto *start = &renderer; start; start = start->Parent())
      {
        if (auto *svgRoot = DynamicDowncast<LegacyRenderSVGRoot>(*start))
        {
          return svgRoot->IsLayoutSizeChanged();
        }
        if (auto *container = DynamicDowncast<LegacyRenderSVGViewportContainer>(*start))
        {
          return container->IsLayoutSizeChanged();
        }
      }
      krys_debug_assert(false);
      return false;
    }
  }

  bool SVGRenderSupport::TransformToRootChanged(RenderElement *ancestor) noexcept
  {
    while (ancestor && !ancestor->IsLegacyRenderSVGRoot())
    {
      if (auto *container = DynamicDowncast<LegacyRenderSVGTransformableContainer>(*ancestor))
      {
        return container->DidTransformToRootUpdate();
      }
      if (auto *container = DynamicDowncast<LegacyRenderSVGViewportContainer>(*ancestor))
      {
        return container->DidTransformToRootUpdate();
      }
      ancestor = ancestor->Parent();
    }

    return false;
  }

  void SVGRenderSupport::LayoutDifferentRootIfNeeded(const RenderElement &renderer) noexcept
  {
    if (auto *resources = SVGResourcesCache::CachedResourcesForRenderer(renderer))
    {
      resources->LayoutDifferentRootIfNeeded(renderer);
    }
  }

  void SVGRenderSupport::LayoutChildren(RenderElement &start, bool selfNeedsLayout) noexcept
  {
    bool layoutSizeChanged = LayoutSizeOfNearestViewportChanged(start);
    bool transformChanged = TransformToRootChanged(&start);
    List<WeakPtr<RenderElement>> elementsThatDidNotReceiveLayout;

    for (auto &child : ChildrenOfType<RenderObject>(start))
    {
      bool needsLayout = selfNeedsLayout;
      bool childEverHadLayout = child.EverHadLayout();

      if (transformChanged)
      {
        // If the transform changed we need to update the text metrics (note: this also happens for
        // layoutSizeChanged=true).
        if (auto *text = DynamicDowncast<RenderSVGText>(child))
        {
          text->SetNeedsTextMetricsUpdate();
        }
        needsLayout = true;
      }

      if (layoutSizeChanged)
      {
        // When selfNeedsLayout is false and the layout size changed, we have to check whether this child uses
        // relative lengths
        if (auto *element = DynamicDowncast<svg::SVGElement>(child.GetNode());
            element && element->HasRelativeLengths())
        {
          // When the layout size changed and when using relative values tell the LegacyRenderSVGShape to
          // update its shape object
          if (auto *shape = DynamicDowncast<LegacyRenderSVGShape>(child))
          {
            shape->SetNeedsShapeUpdate();
          }
          else if (auto *svgText = DynamicDowncast<RenderSVGText>(child))
          {
            svgText->SetNeedsTextMetricsUpdate();
            svgText->SetNeedsPositioningValuesUpdate();
          }

          child.SetNeedsTransformUpdate();
          needsLayout = true;
        }
      }

      if (needsLayout)
      {
        child.SetNeedsLayout(RenderObject::MarkingBehavior::MarkOnlyThis);
      }

      if (child.NeedsLayout())
      {
        auto &childElement = Downcast<RenderElement>(child);
        LayoutDifferentRootIfNeeded(childElement);
        childElement.Layout();
        // Renderers are responsible for repainting themselves when changing, except
        // for the initial paint to avoid potential double-painting caused by non-sensical "old" bounds.
        // We could handle this in the individual objects, but for now it's easier to have
        // parent containers call repaint().  (RenderBlock::layout* has similar logic.)
        if (!childEverHadLayout)
        {
          child.Repaint();
        }
      }
      else if (layoutSizeChanged)
      {
        if (auto *childElement = DynamicDowncast<RenderElement>(child))
        {
          elementsThatDidNotReceiveLayout.push_back(WeakPtr<RenderElement>::WithRef(*childElement));
        }
      }

      krys_debug_assert(!child.NeedsLayout());
    }

    if (!layoutSizeChanged)
    {
      krys_debug_assert(elementsThatDidNotReceiveLayout.empty());
      return;
    }

    // If the layout size changed, invalidate all resources of all children that didn't go through the
    // layout() code path.
    for (auto &element : elementsThatDidNotReceiveLayout)
    {
      if (element)
      {
        InvalidateResourcesOfChildren(*element);
      }
    }
  }

  bool SVGRenderSupport::IsOverflowHidden(const RenderElement &renderer) noexcept
  {
    // LegacyRenderSVGRoot should never query for overflow state - it should always clip itself to the initial
    // viewport size.
    krys_debug_assert(!renderer.IsDocumentElementRenderer());

    return IsNonVisibleOverflow(renderer.Style().OverflowX());
  }

  void SVGRenderSupport::IntersectRepaintRectWithResources(
    const RenderElement &renderer, FloatRect &repaintRect,
    RepaintRectCalculation repaintRectCalculation) noexcept
  {
    auto *resources = SVGResourcesCache::CachedResourcesForRenderer(renderer);
    if (!resources)
    {
      return;
    }

    // FIXME: The filter's resource bounding box, with filters (B11h).

    if (auto *clipper = resources->Clipper())
    {
      repaintRect.Intersect(clipper->ResourceBoundingBox(renderer, repaintRectCalculation));
    }

    if (auto *masker = resources->Masker())
    {
      repaintRect.Intersect(masker->ResourceBoundingBox(renderer, repaintRectCalculation));
    }
  }

  bool SVGRenderSupport::FiltersForceContainerLayout(const RenderElement &renderer) noexcept
  {
    // If any of this container's children need to be laid out, and a filter is applied
    // to the container, we need to repaint the entire container.
    if (!renderer.NormalChildNeedsLayout())
    {
      return false;
    }

    // FIXME: Whether the container has a filter resource (SVGResourcesCache), with filters (B11h).
    return false;
  }

  namespace
  {
    KRYS_NODISCARD FloatRect ClipPathReferenceBox(const RenderElement &renderer, CSSBoxType boxType) noexcept
    {
      FloatRect referenceBox;
      switch (boxType)
      {
        case CSSBoxType::BorderBox:
        case CSSBoxType::MarginBox:
        case CSSBoxType::StrokeBox:
        case CSSBoxType::BoxMissing:
        {
          // FIXME: strokeBoundingBox() takes dasharray into account but shouldn't.
          referenceBox = renderer.StrokeBoundingBox();
          break;
        }
        case CSSBoxType::ViewBox:
        {
          if (renderer.GetElement())
          {
            auto viewportSize =
              svg::SVGLengthContext(Downcast<svg::SVGElement>(renderer.GetElement())).ViewportSize();
            if (viewportSize)
            {
              referenceBox.SetSize(*viewportSize);
            }
            break;
          }
          [[fallthrough]];
        }
        case CSSBoxType::ContentBox:
        case CSSBoxType::FillBox:
        case CSSBoxType::PaddingBox:
        {
          referenceBox = renderer.ObjectBoundingBox();
          break;
        }
      }
      return referenceBox;
    }

    KRYS_NODISCARD bool IsPointInCSSClippingArea(const RenderElement &renderer,
                                                 const FloatPoint &point) noexcept
    {
      const auto &clipPath = renderer.Style().GetClipPath();
      if (auto basicShape = clipPath.TryBasicShape())
      {
        auto referenceBox = ClipPathReferenceBox(renderer, basicShape->ReferenceBox());
        if (!referenceBox.Contains(point))
        {
          return false;
        }
        return style::ComputePath(basicShape->GetShape(), referenceBox)
          .Contains(point, style::ComputeWindRule(basicShape->GetShape()));
      }
      if (auto box = clipPath.TryBox())
      {
        auto referenceBox = ClipPathReferenceBox(renderer, box->ReferenceBox());
        if (!referenceBox.Contains(point))
        {
          return false;
        }
        return FloatRoundedRect {referenceBox}.GetPath().Contains(point);
      }
      return true;
    }
  }

  void SVGRenderSupport::ClipContextToCSSClippingArea(GraphicsContext &context,
                                                      const RenderElement &renderer) noexcept
  {
    const auto &clipPath = renderer.Style().GetClipPath();
    if (auto basicShape = clipPath.TryBasicShape())
    {
      auto localToParentTransform = renderer.LocalToParentTransform();

      auto referenceBox = ClipPathReferenceBox(renderer, basicShape->ReferenceBox());
      referenceBox = localToParentTransform.MapRect(referenceBox);

      auto path = style::ComputePath(basicShape->GetShape(), referenceBox);
      path.Transform(localToParentTransform.Inverse().value_or(AffineTransform {}));

      context.ClipPath(path, style::ComputeWindRule(basicShape->GetShape()));
    }
    else if (auto box = clipPath.TryBox())
    {
      auto referenceBox = ClipPathReferenceBox(renderer, box->ReferenceBox());
      context.ClipPath(FloatRoundedRect {referenceBox}.GetPath());
    }
  }

  bool SVGRenderSupport::PointInClippingArea(const RenderElement &renderer, const FloatPoint &point) noexcept
  {
    const auto &clipPath = renderer.Style().GetClipPath();
    if (clipPath.TryBasicShape() || clipPath.TryBox())
    {
      return IsPointInCSSClippingArea(renderer, point);
    }

    // We just take clippers into account to determine if a point is on the node. The Specification may
    // change later and we also need to check maskers.
    auto *resources = SVGResourcesCache::CachedResourcesForRenderer(renderer);
    if (!resources)
    {
      return true;
    }

    if (auto *clipper = resources->Clipper())
    {
      return clipper->HitTestClipContent(renderer.ObjectBoundingBox(), point);
    }

    return true;
  }

  void SVGRenderSupport::StyleChanged(RenderElement &renderer, const RenderStyle *oldStyle) noexcept
  {
    if (renderer.GetElement() && Is<svg::SVGElement>(*renderer.GetElement())
        && (!oldStyle || renderer.Style().HasBlendMode() != oldStyle->HasBlendMode()))
    {
      SVGRenderSupport::UpdateMaskedAncestorShouldIsolateBlending(renderer);
    }
  }

  bool SVGRenderSupport::IsolatesBlending(const RenderStyle &style) noexcept
  {
    return style.HasPositionedMask() || style.HasFilter() || style.HasBlendMode() || style.HasOpacity();
  }

  void SVGRenderSupport::UpdateMaskedAncestorShouldIsolateBlending(const RenderElement &renderer) noexcept
  {
    auto *element = renderer.GetElement();
    krys_debug_assert(element);
    krys_debug_assert(Is<svg::SVGElement>(*element));
    for (auto *ancestorElement = element->ParentElement(); ancestorElement;
         ancestorElement = ancestorElement->ParentElement())
    {
      auto *ancestorSVGElement = DynamicDowncast<svg::SVGElement>(*ancestorElement);
      auto *ancestor =
        ancestorSVGElement ? DynamicDowncast<svg::SVGGraphicsElement>(*ancestorSVGElement) : nullptr;
      if (!ancestor)
      {
        continue;
      }
      auto *style = ancestor->ComputedStyle();
      if (!style || !IsolatesBlending(*style))
      {
        continue;
      }
      if (style->HasPositionedMask())
      {
        ancestor->SetShouldIsolateBlending(renderer.Style().HasBlendMode());
      }
      return;
    }
  }

  void SVGRenderSupport::ApplyStrokeStyleToContext(GraphicsContext &context, const RenderStyle &style,
                                                   const RenderElement &renderer) noexcept
  {
    auto *element = DynamicDowncast<svg::SVGElement>(renderer.GetElement());
    if (!element)
    {
      krys_debug_assert(false);
      return;
    }

    svg::SVGLengthContext lengthContext(element);
    context.SetStrokeThickness(lengthContext.ValueForLength(style.GetStrokeWidth(), style::ZoomNeeded {}));
    context.SetLineCap(style.CapStyle());
    context.SetLineJoin(style.JoinStyle());
    if (style.JoinStyle() == LineJoin::Miter)
    {
      context.SetMiterLimit(style.StrokeMiterLimit().Value.Value);
    }

    const auto &dashes = style.StrokeDashArray();
    if (dashes.IsNone())
    {
      context.SetStrokeStyle(StrokeStyle::SolidStroke);
    }
    else
    {
      float scaleFactor = 1;

      if (auto *geometryElement = DynamicDowncast<svg::SVGGeometryElement>(*element))
      {
        krys_debug_assert(renderer.IsLegacyRenderSVGShape());
        // FIXME: A value of zero is valid. Need to differentiate this case from being unspecified.
        if (float pathLength = geometryElement->PathLength())
        {
          if (auto *shape = DynamicDowncast<LegacyRenderSVGShape>(renderer))
          {
            scaleFactor = shape->GetTotalLength() / pathLength;
          }
        }
      }

      bool canSetLineDash = false;
      auto dashArray =
        DashArray::map(dashes,
                       [&](const auto &dash) -> DashArrayElement
                       {
                         auto value = lengthContext.ValueForLength(dash, style::ZoomNeeded {}) * scaleFactor;
                         if (value > 0)
                         {
                           canSetLineDash = true;
                         }
                         return value;
                       });

      if (canSetLineDash)
      {
        context.SetLineDash(dashArray,
                            lengthContext.ValueForLength(style.StrokeDashOffset(), style::ZoomNeeded {})
                              * scaleFactor);
      }
      else
      {
        context.SetStrokeStyle(StrokeStyle::SolidStroke);
      }
    }
  }

  FloatRect SVGRenderSupport::CalculateApproximateStrokeBoundingBox(const RenderElement &renderer) noexcept
  {
    auto calculateApproximateScalingStrokeBoundingBox = [&](const LegacyRenderSVGShape &shape,
                                                            FloatRect fillBoundingBox) -> FloatRect
    {
      // Implementation of
      // https://drafts.fxtf.org/css-masking/#compute-stroke-bounding-box
      // except that we ignore whether the stroke is none.

      krys_debug_assert(shape.Style().HasStroke());

      auto strokeBoundingBox = fillBoundingBox;
      const float strokeWidth = shape.StrokeWidth();
      if (strokeWidth <= 0)
      {
        return strokeBoundingBox;
      }

      float delta = strokeWidth / 2;
      switch (shape.GetShapeType())
      {
        case LegacyRenderSVGShape::ShapeType::Empty:
        {
          // Spec: "A negative value is illegal. A value of zero disables rendering of the element."
          return strokeBoundingBox;
        }
        case LegacyRenderSVGShape::ShapeType::Ellipse:
        case LegacyRenderSVGShape::ShapeType::Circle:
        case LegacyRenderSVGShape::ShapeType::Rectangle:
        case LegacyRenderSVGShape::ShapeType::RoundedRectangle: break;
        case LegacyRenderSVGShape::ShapeType::Path:
        case LegacyRenderSVGShape::ShapeType::Line:
        {
          const auto &style = shape.Style();
          if (shape.GetShapeType() == LegacyRenderSVGShape::ShapeType::Path
              && style.JoinStyle() == LineJoin::Miter)
          {
            auto miter = style.StrokeMiterLimit().Value.Value;
            if (miter < std::numbers::sqrt2_v<float> && style.CapStyle() == LineCap::Square)
            {
              delta *= std::numbers::sqrt2_v<float>;
            }
            else
            {
              delta *= std::max(miter, 1.0f);
            }
          }
          else if (style.CapStyle() == LineCap::Square)
          {
            delta *= std::numbers::sqrt2_v<float>;
          }
          break;
        }
      }

      strokeBoundingBox.Inflate(delta);
      return strokeBoundingBox;
    };

    auto calculateApproximateNonScalingStrokeBoundingBox = [&](const LegacyRenderSVGShape &shape,
                                                               FloatRect fillBoundingBox) -> FloatRect
    {
      krys_debug_assert(shape.HasPath());
      krys_debug_assert(shape.Style().HasStroke());
      krys_debug_assert(shape.HasNonScalingStroke());

      auto strokeBoundingBox = fillBoundingBox;
      auto nonScalingTransform = shape.NonScalingStrokeTransform();
      if (auto inverse = nonScalingTransform.Inverse())
      {
        auto *usePath = shape.NonScalingStrokePath(&shape.GetPath(), nonScalingTransform);
        auto strokeBoundingRect =
          calculateApproximateScalingStrokeBoundingBox(shape, usePath->FastBoundingRect());
        strokeBoundingRect = inverse.value().MapRect(strokeBoundingRect);
        if (!strokeBoundingRect.IsNaN())
        {
          strokeBoundingBox.Unite(strokeBoundingRect);
        }
      }
      return strokeBoundingBox;
    };

    auto calculate = [&](const LegacyRenderSVGShape &shape)
    {
      if (!shape.Style().HasStroke())
      {
        return shape.ObjectBoundingBox();
      }
      if (shape.HasNonScalingStroke())
      {
        return calculateApproximateNonScalingStrokeBoundingBox(shape, shape.ObjectBoundingBox());
      }
      return calculateApproximateScalingStrokeBoundingBox(shape, shape.ObjectBoundingBox());
    };

    const auto &shape = Downcast<LegacyRenderSVGShape>(renderer);
    return shape.AdjustStrokeBoundingBoxForMarkersAndZeroLengthLinecaps(RepaintRectCalculation::Fast,
                                                                        calculate(shape));
  }
}
