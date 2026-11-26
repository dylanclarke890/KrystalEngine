/*
 * Copyright (C) 2004, 2005, 2007, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007, 2008 Rob Buis <buis@kde.org>
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 * Copyright (C) 2011 Dirk Schulze <krit@webkit.org>
 * Copyright (C) 2022 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Web/Rendering/SVG/Legacy/LegacyRenderSVGResourceClipper.hpp"
#include "Krystal.Web/Graphics/GraphicsContext.hpp"
#include "Krystal.Web/Graphics/ImageBuffers/ImageBuffer.hpp"
#include "Krystal.Web/Page/FrameView.hpp"
#include "Krystal.Web/Rendering/HitTesting/HitTestRequest.hpp"
#include "Krystal.Web/Rendering/HitTesting/HitTestResult.hpp"
#include "Krystal.Web/Rendering/RenderView.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Web/Rendering/SVG/Legacy/LegacyRenderSVGShape.hpp"
#include "Krystal.Web/Rendering/SVG/Legacy/SVGResources.hpp"
#include "Krystal.Web/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Web/Rendering/SVG/SVGRenderingContext.hpp"
#include "Krystal.Web/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Web/Rendering/SVG/SVGVisitedRendererTracking.hpp"
#include "Krystal.Web/Svg/Elements/SVGClipPathElement.hpp"
#include "Krystal.Web/Svg/Elements/SVGUseElement.hpp"
#include "Krystal.Web/Svg/SVGNames.hpp"

namespace krys::web
{
  namespace
  {
    KRYS_NODISCARD SVGGraphicsElement *GraphicsElement(Node &node) noexcept
    {
      auto *element = DynamicDowncast<SVGElement>(node);
      return element ? DynamicDowncast<SVGGraphicsElement>(*element) : nullptr;
    }
  }

  bool ClipperData::Invalidate(const Inputs &inputs) noexcept
  {
    if (LastInputs != inputs)
    {
      ImageBuffer = nullptr;
      LastInputs = inputs;
    }
    return !ImageBuffer;
  }

  LegacyRenderSVGResourceClipper::LegacyRenderSVGResourceClipper(SVGClipPathElement &element,
                                                                 RenderStyle &&style) noexcept
      : LegacyRenderSVGResourceContainer(Type::LegacySVGResourceClipper, element, krys::move(style))
  {
  }

  LegacyRenderSVGResourceClipper::~LegacyRenderSVGResourceClipper() noexcept = default;

  SVGClipPathElement &LegacyRenderSVGResourceClipper::ClipPathElement() const noexcept
  {
    return Downcast<SVGClipPathElement>(LegacyRenderSVGResourceContainer::GetElement());
  }

  SVGUnitTypes::SVGUnitType LegacyRenderSVGResourceClipper::ClipPathUnits() const noexcept
  {
    return ClipPathElement().ClipPathUnits();
  }

  void LegacyRenderSVGResourceClipper::RemoveAllClientsFromCache() noexcept
  {
    _clipBoundaries.fill(RectF {});
    _clipperMap.clear();
  }

  void LegacyRenderSVGResourceClipper::RemoveClientFromCache(RenderElement &client) noexcept
  {
    _clipperMap.erase(&client);
  }

  auto LegacyRenderSVGResourceClipper::ApplyResource(
    RenderElement &renderer, const RenderStyle &, GraphicsContext *&context,
    KRYS_MAYBE_UNUSED EnumSet<RenderSVGResourceMode> resourceMode) noexcept -> EnumSet<ApplyResult>
  {
    krys_debug_assert(context);
    krys_debug_assert(resourceMode.empty());

    auto repaintRect = renderer.RepaintRectInLocalCoordinates();
    if (repaintRect.IsEmpty())
    {
      return {ApplyResult::ResourceApplied};
    }

    auto boundingBox = renderer.ObjectBoundingBox();
    return ApplyClippingToContext(*context, renderer, boundingBox, boundingBox);
  }

  auto LegacyRenderSVGResourceClipper::PathOnlyClipping(GraphicsContext &context,
                                                        const RenderElement &renderer,
                                                        const AffineTransform &animatedLocalTransform,
                                                        const RectF &objectBoundingBox,
                                                        float usedZoom) noexcept -> EnumSet<ApplyResult>
  {
    // If the current clip-path gets clipped itself, we have to fall back to masking.
    if (Style().HasClipPath())
    {
      return {};
    }

    WindRule clipRule = WindRule::NonZero;
    Path clipPath;

    auto rendererRequiresMaskClipping = [&clipPath](const RenderElement &renderer)
    {
      // Only shapes or paths are supported for direct clipping. We need to fall back to masking for texts.
      if (renderer.IsRenderSVGText())
      {
        return true;
      }
      auto &style = renderer.Style();
      if (style.Display() == DisplayType::None || style.UsedVisibility() != Visibility::Visible)
      {
        return false;
      }
      // Current shape in clip-path gets clipped too. Fall back to masking.
      if (style.HasClipPath())
      {
        return true;
      }
      // Fall back to masking if there is more than one clipping path.
      if (!clipPath.IsEmpty())
      {
        return true;
      }
      return false;
    };

    // If clip-path only contains one visible shape or path, we can use path-based clipping. Invisible
    // shapes don't affect the clipping and can be ignored. If clip-path contains more than one
    // visible shape, the additive clipping may not work, caused by the clipRule. EvenOdd
    // as well as NonZero can cause self-clipping of the elements.
    // See also http://www.w3.org/TR/SVG/painting.html#FillRuleProperty
    for (auto *childNode = ClipPathElement().FirstChild(); childNode; childNode = childNode->NextSibling())
    {
      auto *graphicsElement = GraphicsElement(*childNode);
      if (!graphicsElement)
      {
        continue;
      }
      auto *childRenderer = graphicsElement->Renderer();
      if (!childRenderer)
      {
        continue;
      }
      if (rendererRequiresMaskClipping(*childRenderer))
      {
        return {};
      }

      // For <use> elements, delegate the decision whether to use mask clipping or not to the referenced
      // element.
      if (auto *useElement = DynamicDowncast<SVGUseElement>(*graphicsElement))
      {
        auto *clipChildRenderer = useElement->RendererClipChild();
        if (clipChildRenderer && rendererRequiresMaskClipping(*clipChildRenderer))
        {
          return {};
        }
      }

      clipPath = graphicsElement->ToClipPath();
      clipRule = childRenderer->Style().ClipRule();
    }

    // Only one visible shape/path was found. Directly continue clipping and transform the content to
    // userspace if necessary.
    if (ClipPathElement().ClipPathUnits() == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
    {
      AffineTransform transform;
      transform.Translate(objectBoundingBox.Location());
      transform.Scale(objectBoundingBox.Size());
      clipPath.Transform(transform);
    }
    else if (usedZoom != 1)
    {
      AffineTransform transform;
      transform.Scale(usedZoom);
      clipPath.Transform(transform);
    }

    // Transform path by animatedLocalTransform.
    clipPath.Transform(animatedLocalTransform);

    // The SVG specification wants us to clip everything, if clip-path doesn't have a child.
    if (clipPath.IsEmpty())
    {
      clipPath.AddRect(RectF());
    }

    EnumSet<ApplyResult> result = {ApplyResult::ResourceApplied};
    if (auto *shapeRenderer = DynamicDowncast<LegacyRenderSVGShape>(renderer);
        shapeRenderer && shapeRenderer->GetShapeType() == LegacyRenderSVGShape::ShapeType::Rectangle)
    {
      // When clipping a rect with a path, if we know the path is entirely inside the rect, we can skip a clip
      // when filling the rect.
      auto clipBounds = clipPath.FastBoundingRect();
      if (objectBoundingBox.Contains(clipBounds))
      {
        result.Add(ApplyResult::ClipContainsRendererContent);
      }
    }

    context.ClipPath(clipPath, clipRule);
    return result;
  }

  ClipperData::Inputs LegacyRenderSVGResourceClipper::ComputeInputs(const GraphicsContext &context,
                                                                    const RenderElement &renderer,
                                                                    const RectF &objectBoundingBox,
                                                                    const RectF &clippedContentBounds,
                                                                    float usedZoom) noexcept
  {
    AffineTransform absoluteTransform =
      SVGRenderingContext::CalculateTransformationToOutermostCoordinateSystem(renderer);

    // Ignore 2D rotation, as it doesn't affect the size of the mask.
    SizeF scale(static_cast<float>(absoluteTransform.XScale()),
                static_cast<float>(absoluteTransform.YScale()));

    // Determine scale factor for the clipper. The size of intermediate ImageBuffers shouldn't be bigger than
    // kMaxFilterSize.
    (void)ImageBuffer::SizeNeedsClamping(objectBoundingBox.Size(), scale);

    return {objectBoundingBox, clippedContentBounds, scale, usedZoom, context.PaintingDisabled()};
  }

  auto LegacyRenderSVGResourceClipper::ApplyClippingToContext(GraphicsContext &context,
                                                              RenderElement &renderer,
                                                              const RectF &objectBoundingBox,
                                                              const RectF &clippedContentBounds,
                                                              float usedZoom) noexcept -> EnumSet<ApplyResult>
  {
    AffineTransform animatedLocalTransform = ClipPathElement().AnimatedLocalTransform();

    auto clipResult =
      PathOnlyClipping(context, renderer, animatedLocalTransform, objectBoundingBox, usedZoom);
    if (ResourceWasApplied(clipResult))
    {
      if (auto it = _clipperMap.find(&renderer); it != _clipperMap.end())
      {
        it->second->ImageBuffer = nullptr;
      }

      return clipResult;
    }

    auto &clipperDataPointer = _clipperMap[&renderer];
    if (!clipperDataPointer)
    {
      clipperDataPointer = CreateUnique<ClipperData>();
    }
    auto &clipperData = *clipperDataPointer;

    if (clipperData.Invalidate(
          ComputeInputs(context, renderer, objectBoundingBox, clippedContentBounds, usedZoom)))
    {
      clipperData.ImageBuffer =
        context.CreateScaledImageBuffer(clippedContentBounds, clipperData.LastInputs.Scale);
      if (!clipperData.ImageBuffer)
      {
        return {};
      }

      GraphicsContext &maskContext = clipperData.ImageBuffer->Context();
      maskContext.ConcatCTM(animatedLocalTransform);

      // clipPath can also be clipped by another clipPath.
      auto *resources = SVGResourcesCache::CachedResourcesForRenderer(*this);
      LegacyRenderSVGResourceClipper *clipper = nullptr;
      bool succeeded;
      if (resources && (clipper = resources->Clipper()))
      {
        GraphicsContextStateSaver stateSaver(maskContext);

        if (!ResourceWasApplied(
              clipper->ApplyClippingToContext(maskContext, *this, objectBoundingBox, clippedContentBounds)))
        {
          return {};
        }

        succeeded = DrawContentIntoMaskImage(*clipperData.ImageBuffer, objectBoundingBox, usedZoom);
        // The context restore applies the clipping on non-CG platforms.
      }
      else
      {
        succeeded = DrawContentIntoMaskImage(*clipperData.ImageBuffer, objectBoundingBox, usedZoom);
      }

      if (!succeeded)
      {
        clipperData = {};
      }
    }

    if (!clipperData.ImageBuffer)
    {
      return {};
    }

    SVGRenderingContext::ClipToImageBuffer(context, clippedContentBounds, clipperData.LastInputs.Scale,
                                           clipperData.ImageBuffer, true);
    return {ApplyResult::ResourceApplied};
  }

  bool LegacyRenderSVGResourceClipper::DrawContentIntoMaskImage(ImageBuffer &maskImageBuffer,
                                                                const RectF &objectBoundingBox,
                                                                float usedZoom) noexcept
  {
    GraphicsContext &maskContext = maskImageBuffer.Context();

    AffineTransform maskContentTransformation;
    if (ClipPathElement().ClipPathUnits() == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
    {
      maskContentTransformation.Translate(objectBoundingBox.Location());
      maskContentTransformation.Scale(objectBoundingBox.Size());
      maskContext.ConcatCTM(maskContentTransformation);
    }
    else if (usedZoom != 1)
    {
      maskContentTransformation.Scale(usedZoom);
      maskContext.ConcatCTM(maskContentTransformation);
    }

    // Switch to a paint behavior where all children of this <clipPath> will be rendered using special
    // constraints:
    // - fill-opacity/stroke-opacity/opacity set to 1
    // - masker/filter not applied when rendering the children
    // - fill is set to the initial fill paint server (solid, black)
    // - stroke is set to the initial stroke paint server (none)
    auto &frameView = View().FrameView();
    auto oldBehavior = frameView.GetPaintBehavior();
    auto newBehavior = oldBehavior;
    newBehavior.Add(PaintBehavior::RenderingSVGClipOrMask);
    frameView.SetPaintBehavior(newBehavior);

    // Draw all clipPath children into a global mask.
    for (auto *childNode = ClipPathElement().FirstChild(); childNode; childNode = childNode->NextSibling())
    {
      auto *child = DynamicDowncast<SVGElement>(*childNode);
      if (!child)
      {
        continue;
      }
      auto *renderer = child->Renderer();
      if (!renderer)
      {
        continue;
      }
      if (renderer->NeedsLayout())
      {
        frameView.SetPaintBehavior(oldBehavior);
        return false;
      }
      const RenderStyle &style = renderer->Style();
      if (style.Display() == DisplayType::None || style.UsedVisibility() != Visibility::Visible)
      {
        continue;
      }

      WindRule newClipRule = style.ClipRule();
      auto *useElement = DynamicDowncast<SVGUseElement>(*child);
      if (useElement)
      {
        renderer = useElement->RendererClipChild();
        if (!renderer)
        {
          continue;
        }
        if (!useElement->HasAttribute(SVGNames::ClipRuleAttr().LocalName()))
        {
          newClipRule = renderer->Style().ClipRule();
        }
      }

      // Only shapes, paths and texts are allowed for clipping.
      if (!renderer->IsLegacyRenderSVGShape() && !renderer->IsRenderSVGText())
      {
        continue;
      }

      maskContext.SetFillRule(newClipRule);

      // In the case of a <use> element, we obtained its renderere above, to retrieve its clipRule.
      // We have to pass the <use> renderer itself to renderSubtreeToContext() to apply it's
      // x/y/transform/etc. values when rendering. So if useElement is non-null, refetch the
      // childNode->renderer(), as renderer got overridden above.
      SVGRenderingContext::RenderSubtreeToContext(maskContext, useElement ? *child->Renderer() : *renderer,
                                                  maskContentTransformation);
    }

    frameView.SetPaintBehavior(oldBehavior);
    return true;
  }

  void LegacyRenderSVGResourceClipper::CalculateClipContentRepaintRect(
    RepaintRectCalculation repaintRectCalculation) noexcept
  {
    auto &clipBoundaries = _clipBoundaries[krys::ToUnderlying(repaintRectCalculation)];
    // This is a rough heuristic to appraise the clip size and doesn't consider clip on clip.
    for (auto *childNode = ClipPathElement().FirstChild(); childNode; childNode = childNode->NextSibling())
    {
      auto *renderer = DynamicDowncast<RenderElement>(childNode->Renderer());
      if (!renderer || !Is<SVGElement>(*childNode))
      {
        continue;
      }
      if (!renderer->IsLegacyRenderSVGShape() && !renderer->IsRenderSVGText()
          && !Is<SVGUseElement>(Downcast<SVGElement>(*childNode)))
      {
        continue;
      }
      const RenderStyle &style = renderer->Style();
      if (style.Display() == DisplayType::None || style.UsedVisibility() != Visibility::Visible)
      {
        continue;
      }
      clipBoundaries.Unite(renderer->LocalToParentTransform().MapRect(
        renderer->RepaintRectInLocalCoordinates(repaintRectCalculation)));
    }
    clipBoundaries = ClipPathElement().AnimatedLocalTransform().MapRect(clipBoundaries);
  }

  bool LegacyRenderSVGResourceClipper::HitTestClipContent(const RectF &objectBoundingBox,
                                                          const PointF &nodeAtPoint) noexcept
  {
    static SVGVisitedRendererTracking::VisitedSet visitedSet;

    SVGVisitedRendererTracking recursionTracking(visitedSet);
    if (recursionTracking.IsVisiting(*this))
    {
      return false;
    }

    SVGVisitedRendererTracking::Scope recursionScope(recursionTracking, *this);

    PointF point = nodeAtPoint;
    if (!SVGRenderSupport::PointInClippingArea(*this, point))
    {
      return false;
    }

    if (ClipPathElement().ClipPathUnits() == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
    {
      AffineTransform transform;
      transform.Translate(objectBoundingBox.Location());
      transform.Scale(objectBoundingBox.Size());
      point = transform.Inverse().value_or(AffineTransform {}).MapPoint(point);
    }

    point = ClipPathElement().AnimatedLocalTransform().Inverse().value_or(AffineTransform {}).MapPoint(point);

    for (auto *childNode = ClipPathElement().FirstChild(); childNode; childNode = childNode->NextSibling())
    {
      auto *renderer = childNode->Renderer();
      if (!Is<SVGElement>(*childNode) || !renderer)
      {
        continue;
      }
      if (!renderer->IsLegacyRenderSVGShape() && !renderer->IsRenderSVGText()
          && !Is<SVGUseElement>(Downcast<SVGElement>(*childNode)))
      {
        continue;
      }

      HitTestResult result {LayoutPoint {}};
      HitTestRequest request(HitTestSource::User, {HitTestRequest::Type::SVGClipContent,
                                                   HitTestRequest::Type::DisallowUserAgentShadowContent});
      if (renderer->NodeAtPointF(request, result, point, HitTestForeground))
      {
        return true;
      }
    }

    return false;
  }

  RectF LegacyRenderSVGResourceClipper::ResourceBoundingBox(
    const RenderObject &object, RepaintRectCalculation repaintRectCalculation) noexcept
  {
    // Resource was not layouted yet. Give back the boundingBox of the object.
    if (SelfNeedsLayout())
    {
      // For SelfNeedsClientInvalidation().
      auto &clipperData = _clipperMap[&object];
      if (!clipperData)
      {
        clipperData = CreateUnique<ClipperData>();
      }
      return object.ObjectBoundingBox();
    }

    auto &clipBoundaries = _clipBoundaries[krys::ToUnderlying(repaintRectCalculation)];
    if (clipBoundaries.IsEmpty())
    {
      CalculateClipContentRepaintRect(repaintRectCalculation);
    }

    if (ClipPathElement().ClipPathUnits() == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
    {
      RectF objectBoundingBox = object.ObjectBoundingBox();
      AffineTransform transform;
      transform.Translate(objectBoundingBox.Location());
      transform.Scale(objectBoundingBox.Size());
      return transform.MapRect(clipBoundaries);
    }

    return clipBoundaries;
  }
}
