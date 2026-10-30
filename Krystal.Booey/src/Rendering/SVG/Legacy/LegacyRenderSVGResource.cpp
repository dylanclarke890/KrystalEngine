#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResource.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceClipper.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceFilter.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceMasker.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceSolidColor.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGShape.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResources.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGResourceElementClient.hpp"

namespace krys::boo
{
  namespace
  {
    LegacyRenderSVGResource *RequestPaintingResource(RenderSVGResourceMode mode, RenderElement &renderer,
                                                     const RenderStyle &style, Color &fallbackColor) noexcept
    {
      bool applyToFill = mode == RenderSVGResourceMode::ApplyToFill;

      // When rendering the mask for a LegacyRenderSVGResourceClipper, always use the initial fill paint
      // server.
      if (renderer.View().FrameView().GetPaintBehavior().Contains(PaintBehavior::RenderingSVGClipOrMask))
      {
        // Ignore stroke.
        if (!applyToFill)
        {
          return nullptr;
        }

        // But always use the initial fill paint server.
        auto *colorResource = LegacyRenderSVGResource::SharedSolidPaintingResource();
        colorResource->SetColor(
          style::ComputedStyle::InitialFill().ColorDisregardingType().GetResolvedColor());
        return colorResource;
      }

      const auto &paint = applyToFill ? style.Fill() : style.Stroke();

      // If we have no fill/stroke, return nullptr.
      if (paint.IsNone())
      {
        return nullptr;
      }

      Color color;
      if (auto paintColor = paint.TryAnyColor())
      {
        color = style.ColorResolvingCurrentColor(*paintColor);
      }

      if (style.GetInsideLink() == InsideLink::InsideVisited)
      {
        // FIXME: This code doesn't support the uri component of the visited link paint,
        // https://bugs.webkit.org/show_bug.cgi?id=70006
        const auto &visitedPaint = applyToFill ? style.VisitedLinkFill() : style.VisitedLinkStroke();

        // For `currentcolor`, 'color' already contains the 'visitedColor'.
        if (auto visitedPaintColor = visitedPaint.TryColor();
            visitedPaintColor && !visitedPaintColor->IsCurrentColor())
        {
          if (auto visitedColor = style.ColorResolvingCurrentColor(*visitedPaintColor);
              visitedColor.IsValid())
          {
            color = visitedColor.ColorWithAlpha(color.AlphaAsFloat());
          }
        }
      }

      // If the primary resource is just a color, return immediately.
      auto *colorResource = LegacyRenderSVGResource::SharedSolidPaintingResource();
      if (paint.IsColor())
      {
        colorResource->SetColor(color);
        return colorResource;
      }

      LegacyRenderSVGResource *uriResource = nullptr;
      if (auto *resources = SVGResourcesCache::CachedResourcesForRenderer(renderer))
      {
        uriResource = applyToFill ? resources->Fill() : resources->Stroke();
      }

      // If the requested resource is not available, return the color resource or 'none'.
      if (!uriResource)
      {
        // The fallback is 'none'. (SVG2 say 'none' is implied when no fallback is specified.)
        if (paint.IsURLNone())
        {
          return nullptr;
        }

        colorResource->SetColor(color);
        return colorResource;
      }

      // The paint server resource exists, though it may be invalid (pattern with width/height=0). Pass the
      // fallback color to our caller so it can use the solid color painting resource, if applyResource() on
      // the URI resource failed.
      fallbackColor = color;
      return uriResource;
    }
  }

  void LegacyRenderSVGResource::RemoveAllClientsFromCacheAndMarkForInvalidation(
    bool markForInvalidation) noexcept
  {
    VisitedRenderers visitedRenderers;
    RemoveAllClientsFromCacheAndMarkForInvalidationIfNeeded(markForInvalidation, &visitedRenderers);
  }

  LegacyRenderSVGResource *LegacyRenderSVGResource::FillPaintingResource(RenderElement &renderer,
                                                                         const RenderStyle &style,
                                                                         Color &fallbackColor) noexcept
  {
    return RequestPaintingResource(RenderSVGResourceMode::ApplyToFill, renderer, style, fallbackColor);
  }

  LegacyRenderSVGResource *LegacyRenderSVGResource::StrokePaintingResource(RenderElement &renderer,
                                                                           const RenderStyle &style,
                                                                           Color &fallbackColor) noexcept
  {
    return RequestPaintingResource(RenderSVGResourceMode::ApplyToStroke, renderer, style, fallbackColor);
  }

  LegacyRenderSVGResourceSolidColor *LegacyRenderSVGResource::SharedSolidPaintingResource() noexcept
  {
    static LegacyRenderSVGResourceSolidColor sharedSolidPaintingResource;
    return &sharedSolidPaintingResource;
  }

  namespace
  {
    void RemoveFromCacheAndInvalidateDependencies(
      RenderElement &renderer, bool needsLayout,
      LegacyRenderSVGResource::VisitedRenderers *visitedRenderers) noexcept
    {
      if (auto *resources = SVGResourcesCache::CachedResourcesForRenderer(renderer))
      {
        if (auto *filter = resources->Filter())
        {
          filter->RemoveClientFromCacheAndMarkForInvalidation(renderer);
        }

        if (auto *masker = resources->Masker())
        {
          masker->RemoveClientFromCacheAndMarkForInvalidation(renderer);
        }

        if (auto *clipper = resources->Clipper())
        {
          clipper->RemoveClientFromCacheAndMarkForInvalidation(renderer);
        }
      }

      auto *svgElement = DynamicDowncast<svg::SVGElement>(renderer.GetElement());
      if (!svgElement)
      {
        return;
      }

      for (auto &element : svgElement->ReferencingElements())
      {
        if (auto *elementRenderer = element->Renderer())
        {
          // We allow cycles in SVGDocumentExtensions reference sets in order to avoid expensive
          // reference graph adjustments on changes, so we need to break possible cycles here.
          static HashSet<const svg::SVGElement *> invalidatingDependencies;
          if (!invalidatingDependencies.insert(element.get()).second) KRYS_UNLIKELY
          {
            // Reference cycle: we are in process of invalidating this dependant.
            continue;
          }
          LegacyRenderSVGResource::MarkForLayoutAndParentResourceInvalidationIfNeeded(
            *elementRenderer, needsLayout, visitedRenderers);
          invalidatingDependencies.erase(element.get());
        }
      }

      for (auto &cssClient : svgElement->ReferencingCSSClients())
      {
        if (!cssClient)
        {
          continue;
        }
        cssClient->ResourceChanged(*svgElement);
      }
    }
  }

  void LegacyRenderSVGResource::MarkForLayoutAndParentResourceInvalidation(RenderObject &object,
                                                                           bool needsLayout) noexcept
  {
    VisitedRenderers visitedRenderers;
    MarkForLayoutAndParentResourceInvalidationIfNeeded(object, needsLayout, &visitedRenderers);
  }

  void LegacyRenderSVGResource::MarkForLayoutAndParentResourceInvalidationIfNeeded(
    RenderObject &object, bool needsLayout, VisitedRenderers *visitedRenderers) noexcept
  {
    krys_debug_assert(object.GetNode());

    if (visitedRenderers)
    {
      if (!visitedRenderers->insert(&object).second)
      {
        return;
      }
    }

    if (needsLayout && !object.RenderTreeBeingDestroyed())
    {
      // If we are inside the layout of an LegacyRenderSVGRoot, do not cross the SVG boundary to
      // invalidate the ancestor renderer because it may have finished its layout already.
      if (auto *svgRoot = DynamicDowncast<LegacyRenderSVGRoot>(object); svgRoot && svgRoot->IsInLayout())
      {
        svgRoot->SetNeedsLayout(RenderObject::MarkingBehavior::MarkOnlyThis);
      }
      else
      {
        if (auto *element = DynamicDowncast<RenderElement>(object))
        {
          auto *treeRoot = SVGRenderSupport::FindTreeRootObject(*element);
          if (!treeRoot || !treeRoot->IsInLayout())
          {
            element->SetNeedsLayout(RenderObject::MarkingBehavior::MarkContainingBlockChain);
          }
          else
          {
            // We just want to re-layout the ancestors up to the RenderSVGRoot.
            element->SetNeedsLayout(RenderObject::MarkingBehavior::MarkOnlyThis);
            for (auto *current = element->Parent(); current != treeRoot; current = current->Parent())
            {
              current->SetNeedsLayout(RenderObject::MarkingBehavior::MarkOnlyThis);
            }
            treeRoot->SetNeedsLayout(RenderObject::MarkingBehavior::MarkOnlyThis);
          }
        }
        else
        {
          object.SetNeedsLayout(RenderObject::MarkingBehavior::MarkOnlyThis);
        }
      }
    }

    if (auto *element = DynamicDowncast<RenderElement>(object))
    {
      RemoveFromCacheAndInvalidateDependencies(*element, needsLayout, visitedRenderers);
    }

    // Invalidate resources in ancestor chain, if needed.
    auto *current = object.Parent();
    while (current)
    {
      RemoveFromCacheAndInvalidateDependencies(*current, needsLayout, visitedRenderers);

      if (auto *container = DynamicDowncast<LegacyRenderSVGResourceContainer>(*current))
      {
        // This will process the rest of the ancestors.
        container->RemoveAllClientsFromCacheAndMarkForInvalidationIfNeeded(true, visitedRenderers);
        break;
      }

      current = current->Parent();
    }
  }

  void LegacyRenderSVGResource::FillAndStrokePathOrShape(GraphicsContext &context,
                                                         EnumSet<RenderSVGResourceMode> resourceMode,
                                                         const Path *path,
                                                         const RenderElement *shape) noexcept
  {
    if (shape)
    {
      krys_debug_assert(shape->IsLegacyRenderSVGShape());

      if (resourceMode.Contains(RenderSVGResourceMode::ApplyToFill))
      {
        if (auto *svgShape = DynamicDowncast<LegacyRenderSVGShape>(shape))
        {
          svgShape->FillShape(context);
        }
      }

      if (resourceMode.Contains(RenderSVGResourceMode::ApplyToStroke))
      {
        if (auto *svgShape = DynamicDowncast<LegacyRenderSVGShape>(shape))
        {
          svgShape->StrokeShape(context);
        }
      }

      return;
    }

    if (!path)
    {
      return;
    }

    if (resourceMode.Contains(RenderSVGResourceMode::ApplyToFill))
    {
      context.FillPath(*path);
    }
    if (resourceMode.Contains(RenderSVGResourceMode::ApplyToStroke))
    {
      context.StrokePath(*path);
    }
  }
}
