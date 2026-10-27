#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResources.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCycleSolver.hpp"
#include "Krystal.Booey/SVG/SVGDocumentExtensions.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"

namespace krys::boo
{
  SVGResourcesCache::SVGResourcesCache() noexcept = default;

  SVGResourcesCache::~SVGResourcesCache() noexcept = default;

  void SVGResourcesCache::AddResourcesFromRenderer(RenderElement &renderer, const RenderStyle &style) noexcept
  {
    krys_debug_assert(!renderer.HasCachedSVGResource());
    krys_debug_assert(!_cache.contains(&renderer));

    // Build a list of all resources associated with the passed RenderObject
    auto newResources = SVGResources::BuildCachedResources(renderer, style);
    if (!newResources)
    {
      return;
    }

    // Put object in cache.
    auto &resources = *_cache.emplace(&renderer, krys::move(newResources)).first->second;
    renderer.SetHasCachedSVGResource(true);

    // Run cycle-detection _afterwards_, so self-references can be caught as well.
    SVGResourcesCycleSolver::ResolveCycles(renderer, resources);

    // Walk resources and register the render object at each resources.
    SVGResources::ResourceSet resourceSet;
    resources.BuildSetOfResources(resourceSet);

    for (auto *resourceContainer : resourceSet)
    {
      resourceContainer->AddClient(renderer);
    }
  }

  void SVGResourcesCache::RemoveResourcesFromRenderer(RenderElement &renderer) noexcept
  {
    if (!renderer.HasCachedSVGResource())
    {
      return;
    }

    auto node = _cache.extract(&renderer);
    renderer.SetHasCachedSVGResource(false);
    if (node.empty() || !node.mapped())
    {
      return;
    }
    auto resources = krys::move(node.mapped());

    // Walk resources and register the render object at each resources.
    SVGResources::ResourceSet resourceSet;
    resources->BuildSetOfResources(resourceSet);

    for (auto *resourceContainer : resourceSet)
    {
      resourceContainer->RemoveClient(renderer);
    }
  }

  namespace
  {
    KRYS_NODISCARD SVGResourcesCache &ResourcesCacheFromRenderer(const RenderElement &renderer) noexcept
    {
      return renderer.GetDocument().SVGExtensions().ResourcesCache();
    }

    KRYS_NODISCARD bool
      HasPaintResourceRequiringRemovalOnClientLayoutChange(LegacyRenderSVGResource *resource) noexcept
    {
      return resource && resource->ResourceType() == RenderSVGResourceType::PatternResourceType;
    }

    KRYS_NODISCARD bool HasResourcesRequiringRemovalOnClientLayoutChange(SVGResources &resources) noexcept
    {
      return resources.Masker() || resources.Filter()
             || HasPaintResourceRequiringRemovalOnClientLayoutChange(resources.Fill())
             || HasPaintResourceRequiringRemovalOnClientLayoutChange(resources.Stroke());
    }

    KRYS_NODISCARD bool RendererCanHaveResources(RenderObject &renderer) noexcept
    {
      return renderer.GetNode() && Is<svg::SVGElement>(*renderer.GetNode())
             && !renderer.IsRenderSVGInlineText();
    }
  }

  SVGResources *SVGResourcesCache::CachedResourcesForRenderer(const RenderElement &renderer) noexcept
  {
    if (!renderer.HasCachedSVGResource())
    {
      return nullptr;
    }
    auto &cache = ResourcesCacheFromRenderer(renderer)._cache;
    auto it = cache.find(&renderer);
    return it != cache.end() ? it->second.get() : nullptr;
  }

  void SVGResourcesCache::ClientLayoutChanged(RenderElement &renderer) noexcept
  {
    if (!renderer.HasCachedSVGResource())
    {
      return;
    }

    auto *resources = SVGResourcesCache::CachedResourcesForRenderer(renderer);
    if (!resources)
    {
      return;
    }

    // Invalidate the resources if either the RenderElement itself changed,
    // or we have filter resources, which could depend on the layout of children.
    if ((renderer.SelfNeedsLayout() || resources->Filter())
        && HasResourcesRequiringRemovalOnClientLayoutChange(*resources))
    {
      resources->RemoveClientFromCacheAndMarkForInvalidation(renderer, false);
    }
  }

  void SVGResourcesCache::ClientStyleChanged(RenderElement &renderer, style::Difference diff,
                                             const RenderStyle *oldStyle,
                                             const RenderStyle &newStyle) noexcept
  {
    krys_debug_assert(!renderer.GetElement() || Is<svg::SVGElement>(*renderer.GetElement()));

    if (!renderer.Parent())
    {
      return;
    }

    // For filter primitives, when diff is Repaint or RepaintIsText, the
    // SVGFE*Element will decide whether the modified CSS properties require a
    // relayout or repaint.
    //
    // Since diff can be Equal even if we have have a filter property change
    // (due to how RenderElement::adjustStyleDifference works), in general we
    // want to continue to the comparison of oldStyle and newStyle below, and
    // so we don't return early just when diff == Style::DifferenceResult::Equal. But
    // this isn't necessary for filter primitives, to which the filter property
    // doesn't apply, so we check for it here too.
    if (renderer.IsLegacyRenderSVGResourceFilterPrimitive()
        && (diff == style::DifferenceResult::Equal || diff == style::DifferenceResult::Repaint
            || diff == style::DifferenceResult::RepaintIfText))
    {
      return;
    }

    auto hasStyleDifferencesAffectingResources = [&]
    {
      if (!RendererCanHaveResources(renderer))
      {
        return false;
      }

      if (!oldStyle)
      {
        return true;
      }

      if (oldStyle->GetClipPath() != newStyle.GetClipPath())
      {
        return true;
      }

      // RenderSVGResourceMarker only supports SVG <mask> references.
      if (oldStyle->GetMaskLayers().UsedFirst().GetImage() != newStyle.GetMaskLayers().UsedFirst().GetImage())
      {
        return true;
      }

      if (oldStyle->GetFilter() != newStyle.GetFilter())
      {
        return true;
      }

      if (oldStyle->Fill().UrlDisregardingType() != newStyle.Fill().UrlDisregardingType())
      {
        return true;
      }

      if (oldStyle->Stroke().UrlDisregardingType() != newStyle.Stroke().UrlDisregardingType())
      {
        return true;
      }

      if (oldStyle->MarkerStart() != newStyle.MarkerStart())
      {
        return true;
      }

      if (oldStyle->MarkerMid() != newStyle.MarkerMid())
      {
        return true;
      }

      if (oldStyle->MarkerEnd() != newStyle.MarkerEnd())
      {
        return true;
      }

      return false;
    };

    if (hasStyleDifferencesAffectingResources())
    {
      auto &cache = ResourcesCacheFromRenderer(renderer);
      cache.RemoveResourcesFromRenderer(renderer);
      cache.AddResourcesFromRenderer(renderer, newStyle);
    }

    LegacyRenderSVGResource::MarkForLayoutAndParentResourceInvalidation(renderer, false);
  }

  void SVGResourcesCache::ClientWasAddedToTree(RenderObject &renderer) noexcept
  {
    if (renderer.IsAnonymous())
    {
      return;
    }

    LegacyRenderSVGResource::MarkForLayoutAndParentResourceInvalidation(renderer, false);

    if (!RendererCanHaveResources(renderer))
    {
      return;
    }
    auto &elementRenderer = Downcast<RenderElement>(renderer);
    ResourcesCacheFromRenderer(elementRenderer)
      .AddResourcesFromRenderer(elementRenderer, elementRenderer.Style());
  }

  void SVGResourcesCache::ClientWillBeRemovedFromTree(RenderObject &renderer) noexcept
  {
    if (!RendererCanHaveResources(renderer))
    {
      return;
    }
    auto &elementRenderer = Downcast<RenderElement>(renderer);

    if (!elementRenderer.HasCachedSVGResource())
    {
      return;
    }

    if (renderer.IsAnonymous())
    {
      return;
    }

    LegacyRenderSVGResource::MarkForLayoutAndParentResourceInvalidation(renderer, false);
    ResourcesCacheFromRenderer(elementRenderer).RemoveResourcesFromRenderer(elementRenderer);
  }

  void SVGResourcesCache::ClientDestroyed(RenderElement &renderer) noexcept
  {
    if (!renderer.HasCachedSVGResource())
    {
      return;
    }

    if (auto *resources = SVGResourcesCache::CachedResourcesForRenderer(renderer))
    {
      resources->RemoveClientFromCacheAndMarkForInvalidation(renderer);
      ResourcesCacheFromRenderer(renderer).RemoveResourcesFromRenderer(renderer);
    }
  }

  void SVGResourcesCache::ResourceDestroyed(LegacyRenderSVGResourceContainer &resource) noexcept
  {
    auto &cache = ResourcesCacheFromRenderer(resource);

    // The resource itself may have clients, that need to be notified.
    cache.RemoveResourcesFromRenderer(resource);

    for (auto &[client, resources] : cache._cache)
    {
      if (resources->ResourceDestroyed(resource))
      {
        // Mark users of destroyed resources as pending resolution based on the id of the old resource.
        Ref<svg::SVGElement> clientElement = ShareRef(Downcast<svg::SVGElement>(*client->GetElement()));
        clientElement->TreeScopeForSVGReferences().AddPendingSVGResource(
          dom::DOMStringAtom(resource.GetElement().Id()), *clientElement);
      }
    }
  }

  SVGResourcesCache::SetStyleForScope::SetStyleForScope(RenderElement &renderer,
                                                        const RenderStyle &scopedStyle,
                                                        const RenderStyle &newStyle) noexcept
      : _renderer(renderer), _scopedStyle(scopedStyle),
        _needsNewStyle(scopedStyle != newStyle && RendererCanHaveResources(renderer))
  {
    SetStyle(newStyle);
  }

  SVGResourcesCache::SetStyleForScope::~SetStyleForScope() noexcept
  {
    SetStyle(_scopedStyle);
  }

  void SVGResourcesCache::SetStyleForScope::SetStyle(const RenderStyle &style) noexcept
  {
    if (!_needsNewStyle)
    {
      return;
    }

    auto &cache = ResourcesCacheFromRenderer(_renderer);
    cache.RemoveResourcesFromRenderer(_renderer);
    cache.AddResourcesFromRenderer(_renderer, style);
  }
}
