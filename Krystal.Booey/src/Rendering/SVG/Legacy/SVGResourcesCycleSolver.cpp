#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCycleSolver.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceClipper.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceMarker.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceMasker.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Core/Utils/ScopeExit.hpp"

namespace krys::boo
{
  bool SVGResourcesCycleSolver::ResourceContainsCycles(LegacyRenderSVGResourceContainer &resource,
                                                       SVGResources::ResourceSet &activeResources,
                                                       SVGResources::ResourceSet &acyclicResources) noexcept
  {
    if (acyclicResources.contains(&resource))
    {
      return false;
    }

    activeResources.insert(&resource);
    auto activeResourceAcquisition = MakeScopeExit([&] { activeResources.erase(&resource); });

    RenderObject *node = &resource;
    while (node)
    {
      if (node != &resource && node->IsLegacyRenderSVGResourceContainer())
      {
        node = node->NextInPreOrderAfterChildren(&resource);
        continue;
      }
      if (auto *element = DynamicDowncast<RenderElement>(*node))
      {
        if (auto *resources = SVGResourcesCache::CachedResourcesForRenderer(*element))
        {
          SVGResources::ResourceSet resourceSet;
          resources->BuildSetOfResources(resourceSet);

          for (auto *usedResource : resourceSet)
          {
            if (activeResources.contains(usedResource)
                || ResourceContainsCycles(*usedResource, activeResources, acyclicResources))
            {
              return true;
            }
          }
        }
      }

      node = node->NextInPreOrder(&resource);
    }

    acyclicResources.insert(&resource);
    return false;
  }

  void SVGResourcesCycleSolver::ResolveCycles(RenderElement &renderer, SVGResources &resources) noexcept
  {
    SVGResources::ResourceSet localResources;
    resources.BuildSetOfResources(localResources);

    SVGResources::ResourceSet activeResources;
    SVGResources::ResourceSet acyclicResources;

    if (auto *container = DynamicDowncast<LegacyRenderSVGResourceContainer>(renderer))
    {
      activeResources.insert(container);
    }

    // The job of this function is to determine wheter any of the 'resources' associated with the given
    // 'renderer' references us (or whether any of its kids references us) -> that's a cycle, we need to find
    // and break it.
    for (auto *resource : localResources)
    {
      if (activeResources.contains(resource)
          || ResourceContainsCycles(*resource, activeResources, acyclicResources))
      {
        BreakCycle(*resource, resources);
      }
    }
  }

  void SVGResourcesCycleSolver::BreakCycle(LegacyRenderSVGResourceContainer &resourceLeadingToCycle,
                                           SVGResources &resources) noexcept
  {
    if (&resourceLeadingToCycle == resources.LinkedResource())
    {
      resources.ResetLinkedResource();
      return;
    }

    switch (resourceLeadingToCycle.ResourceType())
    {
      case RenderSVGResourceType::MaskerResourceType:
      {
        krys_debug_assert(&resourceLeadingToCycle == resources.Masker());
        resources.ResetMasker();
        break;
      }
      case RenderSVGResourceType::MarkerResourceType:
      {
        krys_debug_assert(&resourceLeadingToCycle == resources.MarkerStart()
                          || &resourceLeadingToCycle == resources.MarkerMid()
                          || &resourceLeadingToCycle == resources.MarkerEnd());
        if (resources.MarkerStart() == &resourceLeadingToCycle)
        {
          resources.ResetMarkerStart();
        }
        if (resources.MarkerMid() == &resourceLeadingToCycle)
        {
          resources.ResetMarkerMid();
        }
        if (resources.MarkerEnd() == &resourceLeadingToCycle)
        {
          resources.ResetMarkerEnd();
        }
        break;
      }
      case RenderSVGResourceType::PatternResourceType:
      case RenderSVGResourceType::LinearGradientResourceType:
      case RenderSVGResourceType::RadialGradientResourceType:
      {
        krys_debug_assert(&resourceLeadingToCycle == resources.Fill()
                          || &resourceLeadingToCycle == resources.Stroke());
        if (resources.Fill() == &resourceLeadingToCycle)
        {
          resources.ResetFill();
        }
        if (resources.Stroke() == &resourceLeadingToCycle)
        {
          resources.ResetStroke();
        }
        break;
      }
      case RenderSVGResourceType::FilterResourceType:
      {
        krys_debug_assert(&resourceLeadingToCycle == resources.Filter());
        resources.ResetFilter();
        break;
      }
      case RenderSVGResourceType::ClipperResourceType:
      {
        krys_debug_assert(&resourceLeadingToCycle == resources.Clipper());
        resources.ResetClipper();
        break;
      }
      case RenderSVGResourceType::SolidColorResourceType:
      {
        krys_debug_assert(false);
        break;
      }
    }
  }
}
