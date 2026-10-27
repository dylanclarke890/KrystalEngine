#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  LegacyRenderSVGResourceContainer::LegacyRenderSVGResourceContainer(Type type, svg::SVGElement &element,
                                                                     RenderStyle &&style) noexcept
      : LegacyRenderSVGHiddenContainer(type, element, krys::move(style),
                                       SVGModelObjectFlag::IsResourceContainer),
        _id(dom::DOMStringAtom(element.Id()))
  {
  }

  LegacyRenderSVGResourceContainer::~LegacyRenderSVGResourceContainer() noexcept = default;

  void LegacyRenderSVGResourceContainer::Layout() noexcept
  {
    // Invalidate all resources if our layout changed.
    if (SelfNeedsClientInvalidation())
    {
      LegacyRenderSVGRoot::AddResourceForClientInvalidation(this);
    }

    LegacyRenderSVGHiddenContainer::Layout();
  }

  void LegacyRenderSVGResourceContainer::WillBeDestroyed() noexcept
  {
    SVGResourcesCache::ResourceDestroyed(*this);

    if (_registered)
    {
      GetElement().TreeScopeForSVGReferences().RemoveSVGResource(_id);
      _registered = false;
    }

    LegacyRenderSVGHiddenContainer::WillBeDestroyed();
  }

  void LegacyRenderSVGResourceContainer::StyleDidChange(style::Difference diff,
                                                        const RenderStyle *oldStyle) noexcept
  {
    LegacyRenderSVGHiddenContainer::StyleDidChange(diff, oldStyle);

    if (!_registered)
    {
      _registered = true;
      RegisterResource();
    }
  }

  void LegacyRenderSVGResourceContainer::IdChanged() noexcept
  {
    // Invalidate all our current clients.
    RemoveAllClientsFromCacheAndMarkForInvalidation();

    // Remove old id, that is guaranteed to be present in cache.
    auto &treeScope = GetElement().TreeScopeForSVGReferences();
    treeScope.RemoveSVGResource(_id);
    _id = dom::DOMStringAtom(GetElement().Id());

    RegisterResource();
  }

  void LegacyRenderSVGResourceContainer::RemoveClientFromCacheAndMarkForInvalidation(
    RenderElement &client, bool markForInvalidation) noexcept
  {
    RemoveClientFromCache(client);
    MarkClientForInvalidation(client, markForInvalidation ? InvalidationMode::BoundariesInvalidation
                                                          : InvalidationMode::ParentOnlyInvalidation);
  }

  void LegacyRenderSVGResourceContainer::RemoveAllClientsFromCacheAndMarkForInvalidationIfNeeded(
    bool markForInvalidation, VisitedRenderers *visitedRenderers) noexcept
  {
    RemoveAllClientsFromCache();
    MarkAllClientsForInvalidationIfNeeded(markForInvalidation
                                            ? InvalidationMode::LayoutAndBoundariesInvalidation
                                            : InvalidationMode::ParentOnlyInvalidation,
                                          visitedRenderers);
  }

  void LegacyRenderSVGResourceContainer::MarkAllClientsForRepaint() noexcept
  {
    MarkAllClientsForInvalidation(InvalidationMode::RepaintInvalidation);
  }

  void LegacyRenderSVGResourceContainer::MarkAllClientsForInvalidation(InvalidationMode mode) noexcept
  {
    VisitedRenderers visitedRenderers;
    MarkAllClientsForInvalidationIfNeeded(mode, &visitedRenderers);
  }

  void LegacyRenderSVGResourceContainer::MarkAllClientsForInvalidationIfNeeded(
    InvalidationMode mode, VisitedRenderers *visitedRenderers) noexcept
  {
    // FIXME: Style invalidation should either be a pre-layout task or this function
    // should never get called while in layout. See webkit.org/b/208903.
    if (_clients.empty() || _isInvalidating)
    {
      return;
    }

    SetForScope isInvalidating(_isInvalidating, true);

    bool needsLayout = mode == InvalidationMode::LayoutAndBoundariesInvalidation;
    bool markForInvalidation = mode != InvalidationMode::ParentOnlyInvalidation;
    auto *root = SVGRenderSupport::FindTreeRootObject(*this);

    // Clients may leave the set while it's walked.
    auto clients = List<RenderElement *>(_clients.begin(), _clients.end());
    for (auto *client : clients)
    {
      if (!_clients.contains(client))
      {
        continue;
      }

      // We should not mark any client outside the current root for invalidation
      if (root != SVGRenderSupport::FindTreeRootObject(*client))
      {
        continue;
      }

      if (auto *container = DynamicDowncast<LegacyRenderSVGResourceContainer>(*client))
      {
        container->RemoveAllClientsFromCacheAndMarkForInvalidationIfNeeded(markForInvalidation,
                                                                           visitedRenderers);
        continue;
      }

      if (markForInvalidation)
      {
        MarkClientForInvalidation(*client, mode);
      }

      LegacyRenderSVGResource::MarkForLayoutAndParentResourceInvalidationIfNeeded(*client, needsLayout,
                                                                                  visitedRenderers);
    }

    // FIXME: markAllClientLayersForInvalidation, with clipping and masking from HTML (B11f).
  }

  void LegacyRenderSVGResourceContainer::MarkClientForInvalidation(RenderObject &client,
                                                                   InvalidationMode mode) noexcept
  {
    switch (mode)
    {
      case InvalidationMode::LayoutAndBoundariesInvalidation:
      case InvalidationMode::BoundariesInvalidation:
      {
        client.InvalidateCachedBoundaries();
        break;
      }
      case InvalidationMode::RepaintInvalidation:
      {
        if (!client.RenderTreeBeingDestroyed())
        {
          client.Repaint();
        }
        break;
      }
      case InvalidationMode::ParentOnlyInvalidation:
      {
        break;
      }
    }
  }

  void LegacyRenderSVGResourceContainer::AddClient(RenderElement &client) noexcept
  {
    _clients.insert(&client);
  }

  void LegacyRenderSVGResourceContainer::RemoveClient(RenderElement &client) noexcept
  {
    RemoveClientFromCacheAndMarkForInvalidation(client, false);
    _clients.erase(&client);
  }

  void LegacyRenderSVGResourceContainer::RegisterResource() noexcept
  {
    auto &treeScope = GetElement().TreeScopeForSVGReferences();
    if (!treeScope.IsIdOfPendingSVGResource(_id))
    {
      treeScope.AddSVGResource(_id, *this);
      return;
    }

    auto elements = treeScope.RemovePendingSVGResource(_id);

    treeScope.AddSVGResource(_id, *this);

    // Update cached resources of pending clients.
    for (auto &weakClient : elements)
    {
      RefPtr<svg::SVGElement> client = ShareRefPtr(weakClient.get());
      if (!client)
      {
        continue;
      }
      krys_debug_assert(client->HasPendingResources());
      treeScope.ClearHasPendingSVGResourcesIfPossible(*client);
      auto *renderer = client->Renderer();
      if (!renderer)
      {
        continue;
      }
      SVGResourcesCache::ClientStyleChanged(*renderer, style::DifferenceResult::Layout, nullptr,
                                            renderer->Style());
      renderer->SetNeedsLayout();
    }
  }

  // FIXME: This does not belong here.
  AffineTransform LegacyRenderSVGResourceContainer::TransformOnNonScalingStroke(
    RenderObject *object, const AffineTransform &resourceTransform) noexcept
  {
    if (!object->IsLegacyRenderSVGShape())
    {
      return resourceTransform;
    }

    RefPtr<svg::SVGGraphicsElement> element =
      ShareRefPtr(&Downcast<svg::SVGGraphicsElement>(Downcast<svg::SVGElement>(*object->GetNode())));
    AffineTransform transform = element->GetScreenCTM(svg::SVGLocatable::DisallowStyleUpdate);
    transform *= resourceTransform;
    return transform;
  }
}
