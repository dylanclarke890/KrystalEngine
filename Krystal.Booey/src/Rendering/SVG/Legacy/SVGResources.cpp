#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResources.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/SVG/SVGGradientElement.hpp"
#include "Krystal.Booey/SVG/SVGPatternElement.hpp"
#include "Krystal.Booey/SVG/SVGURIReference.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  SVGResources::SVGResources() noexcept = default;

  namespace
  {
    // FIXME: The clipper, filter and masker tags (clipperFilterMaskerTags), with clipping and masking (B11f)
    // and filters (B11h); the marker tags (markerTags), with markers (B11f).

    KRYS_NODISCARD bool IsFillAndStrokeTag(html::TagName tagName) noexcept
    {
      using html::TagName;

      switch (tagName)
      {
        case TagName::circle:
        case TagName::ellipse:
        case TagName::line:
        case TagName::path:
        case TagName::polygon:
        case TagName::polyline:
        case TagName::rect:
        case TagName::text:
        case TagName::textPath:
        case TagName::tspan:
        {
          return true;
        }
        default:
        {
          return false;
        }
      }
    }

    // FIXME: <filter>, with filters (B11h).
    KRYS_NODISCARD bool IsChainableResourceTag(html::TagName tagName) noexcept
    {
      using html::TagName;

      switch (tagName)
      {
        case TagName::linearGradient:
        case TagName::pattern:
        case TagName::radialGradient:
        {
          return true;
        }
        default:
        {
          return false;
        }
      }
    }

    KRYS_NODISCARD dom::DOMStringAtom TargetReferenceFromResource(svg::SVGElement &element) noexcept
    {
      dom::DOMString target;
      if (auto *svgPattern = DynamicDowncast<svg::SVGPatternElement>(element))
      {
        target = svgPattern->Href();
      }
      else if (auto *svgGradient = DynamicDowncast<svg::SVGGradientElement>(element))
      {
        target = svgGradient->Href();
      }
      else
      {
        krys_debug_assert(false);
      }

      return svg::SVGURIReference::FragmentIdentifierFromIRIString(target, element.NodeDocument());
    }

    KRYS_NODISCARD bool IsChainableResource(const svg::SVGElement &element,
                                            const svg::SVGElement &linkedResource) noexcept
    {
      if (Is<svg::SVGPatternElement>(element))
      {
        return Is<svg::SVGPatternElement>(linkedResource);
      }

      if (Is<svg::SVGGradientElement>(element))
      {
        return Is<svg::SVGGradientElement>(linkedResource);
      }

      krys_debug_assert(false);
      return false;
    }

    KRYS_NODISCARD LegacyRenderSVGResourceContainer *
      PaintingResourceFromSVGPaint(dom::DocumentOrShadowRoot &treeScope, const style::SVGPaint &paint,
                                   dom::DOMStringAtom &id, bool &hasPendingResource) noexcept
    {
      auto paintURL = paint.TryAnyURL();
      if (!paintURL)
      {
        return nullptr;
      }

      id =
        svg::SVGURIReference::FragmentIdentifierFromIRIString(*paintURL, treeScope.RootNode().NodeDocument());
      auto *container = GetRenderSVGResourceContainerById(treeScope, id);
      if (!container)
      {
        hasPendingResource = true;
        return nullptr;
      }

      auto resourceType = container->ResourceType();
      if (resourceType != RenderSVGResourceType::PatternResourceType
          && resourceType != RenderSVGResourceType::LinearGradientResourceType
          && resourceType != RenderSVGResourceType::RadialGradientResourceType)
      {
        return nullptr;
      }

      return container;
    }
  }

  UniquePtr<SVGResources> SVGResources::BuildCachedResources(const RenderElement &renderer,
                                                             const RenderStyle &style) noexcept
  {
    krys_debug_assert(renderer.GetElement());
    if (!renderer.GetElement())
    {
      return nullptr;
    }

    Ref<svg::SVGElement> element = ShareRef(Downcast<svg::SVGElement>(*renderer.GetElement()));

    auto &treeScope = element->TreeScopeForSVGReferences();
    auto &document = treeScope.RootNode().NodeDocument();

    auto tagName = element->TagQualifiedName().TagName();

    auto ensureResources = [](UniquePtr<SVGResources> &resources) -> SVGResources &
    {
      if (!resources)
      {
        resources = CreateUnique<SVGResources>();
      }
      return *resources;
    };

    UniquePtr<SVGResources> foundResources;
    // FIXME: The clipper, filter and masker (clip-path, filter and mask-image references), with clipping and
    // masking (B11f) and filters (B11h); the markers, with markers (B11f).

    if (IsFillAndStrokeTag(tagName))
    {
      if (style.HasFill())
      {
        bool hasPendingResource = false;
        auto id = dom::DOMStringAtom::Null();
        if (auto *fill = PaintingResourceFromSVGPaint(treeScope, style.Fill(), id, hasPendingResource))
        {
          ensureResources(foundResources).SetFill(fill);
        }
        else if (hasPendingResource)
        {
          treeScope.AddPendingSVGResource(id, *element);
        }
      }

      if (style.HasStroke())
      {
        bool hasPendingResource = false;
        auto id = dom::DOMStringAtom::Null();
        if (auto *stroke = PaintingResourceFromSVGPaint(treeScope, style.Stroke(), id, hasPendingResource))
        {
          ensureResources(foundResources).SetStroke(stroke);
        }
        else if (hasPendingResource)
        {
          treeScope.AddPendingSVGResource(id, *element);
        }
      }
    }

    if (IsChainableResourceTag(tagName))
    {
      auto id = TargetReferenceFromResource(*element);
      auto *linkedResource = GetRenderSVGResourceContainerById(document, id);
      if (!linkedResource)
      {
        treeScope.AddPendingSVGResource(id, *element);
      }
      else if (IsChainableResource(*element, linkedResource->GetElement()))
      {
        ensureResources(foundResources).SetLinkedResource(linkedResource);
      }
    }

    return foundResources;
  }

  void SVGResources::LayoutDifferentRootIfNeeded(const RenderElement &resourcesClient) noexcept
  {
    if (_inLayoutForDifferentRoot)
    {
      return;
    }

    SetForScope inLayoutForDifferentRoot(_inLayoutForDifferentRoot, true);
    // FIXME: Lay out the clipper, masker, filter and markers when they're in another root, with clipping
    // and masking (B11f) and filters (B11h).
    (void)resourcesClient;
  }

  bool SVGResources::IsEmpty() const noexcept
  {
    return !_fillStrokeData && !_linkedResource;
  }

  LegacyRenderSVGResourceContainer *SVGResources::Fill() const noexcept
  {
    return _fillStrokeData ? _fillStrokeData->Fill.get() : nullptr;
  }

  LegacyRenderSVGResourceContainer *SVGResources::Stroke() const noexcept
  {
    return _fillStrokeData ? _fillStrokeData->Stroke.get() : nullptr;
  }

  LegacyRenderSVGResourceContainer *SVGResources::LinkedResource() const noexcept
  {
    return _linkedResource.get();
  }

  void SVGResources::RemoveClientFromCacheAndMarkForInvalidation(RenderElement &renderer,
                                                                 bool markForInvalidation) const noexcept
  {
    if (IsEmpty())
    {
      return;
    }

    if (_linkedResource)
    {
      krys_debug_assert(!_fillStrokeData);
      _linkedResource->RemoveClientFromCacheAndMarkForInvalidation(renderer, markForInvalidation);
      return;
    }

    if (_fillStrokeData)
    {
      if (auto *fill = _fillStrokeData->Fill.get())
      {
        fill->RemoveClientFromCacheAndMarkForInvalidation(renderer, markForInvalidation);
      }
      if (auto *stroke = _fillStrokeData->Stroke.get())
      {
        stroke->RemoveClientFromCacheAndMarkForInvalidation(renderer, markForInvalidation);
      }
    }
  }

  bool SVGResources::ResourceDestroyed(LegacyRenderSVGResourceContainer &resource) noexcept
  {
    if (IsEmpty())
    {
      return false;
    }

    if (_linkedResource == &resource)
    {
      krys_debug_assert(!_fillStrokeData);
      _linkedResource->RemoveAllClientsFromCacheAndMarkForInvalidation();
      _linkedResource = nullptr;
      return true;
    }

    bool foundResources = false;
    switch (resource.ResourceType())
    {
      case RenderSVGResourceType::PatternResourceType:
      case RenderSVGResourceType::LinearGradientResourceType:
      case RenderSVGResourceType::RadialGradientResourceType:
      {
        if (!_fillStrokeData)
        {
          break;
        }
        if (_fillStrokeData->Fill == &resource)
        {
          resource.RemoveAllClientsFromCacheAndMarkForInvalidation();
          _fillStrokeData->Fill = nullptr;
          foundResources = true;
        }
        if (_fillStrokeData->Stroke == &resource)
        {
          resource.RemoveAllClientsFromCacheAndMarkForInvalidation();
          _fillStrokeData->Stroke = nullptr;
          foundResources = true;
        }
        break;
      }
      // FIXME: Maskers, markers, filters and clippers, with clipping and masking (B11f) and filters (B11h).
      case RenderSVGResourceType::MaskerResourceType:
      case RenderSVGResourceType::MarkerResourceType:
      case RenderSVGResourceType::FilterResourceType:
      case RenderSVGResourceType::ClipperResourceType:
      {
        break;
      }
      case RenderSVGResourceType::SolidColorResourceType:
      {
        krys_debug_assert(false);
        break;
      }
    }
    return foundResources;
  }

  void SVGResources::BuildSetOfResources(ResourceSet &set) noexcept
  {
    if (IsEmpty())
    {
      return;
    }

    if (auto *linkedResource = _linkedResource.get())
    {
      krys_debug_assert(!_fillStrokeData);
      set.insert(linkedResource);
      return;
    }

    if (_fillStrokeData)
    {
      if (auto *fill = _fillStrokeData->Fill.get())
      {
        set.insert(fill);
      }
      if (auto *stroke = _fillStrokeData->Stroke.get())
      {
        set.insert(stroke);
      }
    }
  }

  bool SVGResources::SetFill(LegacyRenderSVGResourceContainer *fill) noexcept
  {
    krys_debug_assert(fill);
    krys_debug_assert(fill->ResourceType() == RenderSVGResourceType::PatternResourceType
                      || fill->ResourceType() == RenderSVGResourceType::LinearGradientResourceType
                      || fill->ResourceType() == RenderSVGResourceType::RadialGradientResourceType);

    if (!_fillStrokeData)
    {
      _fillStrokeData = CreateUnique<FillStrokeData>();
    }

    _fillStrokeData->Fill = CreateWeakPtr(fill);
    return true;
  }

  void SVGResources::ResetFill() noexcept
  {
    krys_debug_assert(_fillStrokeData);
    krys_debug_assert(_fillStrokeData->Fill);
    _fillStrokeData->Fill = nullptr;
  }

  bool SVGResources::SetStroke(LegacyRenderSVGResourceContainer *stroke) noexcept
  {
    krys_debug_assert(stroke);
    krys_debug_assert(stroke->ResourceType() == RenderSVGResourceType::PatternResourceType
                      || stroke->ResourceType() == RenderSVGResourceType::LinearGradientResourceType
                      || stroke->ResourceType() == RenderSVGResourceType::RadialGradientResourceType);

    if (!_fillStrokeData)
    {
      _fillStrokeData = CreateUnique<FillStrokeData>();
    }

    _fillStrokeData->Stroke = CreateWeakPtr(stroke);
    return true;
  }

  void SVGResources::ResetStroke() noexcept
  {
    krys_debug_assert(_fillStrokeData);
    krys_debug_assert(_fillStrokeData->Stroke);
    _fillStrokeData->Stroke = nullptr;
  }

  bool SVGResources::SetLinkedResource(LegacyRenderSVGResourceContainer *linkedResource) noexcept
  {
    krys_debug_assert(linkedResource);
    _linkedResource = CreateWeakPtr(linkedResource);
    return true;
  }

  void SVGResources::ResetLinkedResource() noexcept
  {
    krys_debug_assert(_linkedResource);
    _linkedResource = nullptr;
  }
}
