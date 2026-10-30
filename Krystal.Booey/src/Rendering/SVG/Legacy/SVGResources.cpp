#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResources.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/ReferenceFilterOperation.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceClipper.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceFilter.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceMarker.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceMasker.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/Filter.hpp"
#include "Krystal.Booey/SVG/SVGFilterElement.hpp"
#include "Krystal.Booey/SVG/SVGGradientElement.hpp"
#include "Krystal.Booey/SVG/SVGMarkerElement.hpp"
#include "Krystal.Booey/SVG/SVGPatternElement.hpp"
#include "Krystal.Booey/SVG/SVGURIReference.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  SVGResources::SVGResources() noexcept = default;

  namespace
  {
    KRYS_NODISCARD bool IsClipperFilterMaskerTag(html::TagName tagName) noexcept
    {
      using html::TagName;

      switch (tagName)
      {
        // "container elements": http://www.w3.org/TR/SVG11/intro.html#TermContainerElement
        // "graphics elements" : http://www.w3.org/TR/SVG11/intro.html#TermGraphicsElement
        case TagName::a:
        case TagName::circle:
        case TagName::ellipse:
        case TagName::g:
        case TagName::image:
        case TagName::line:
        case TagName::marker:
        case TagName::mask:
        case TagName::path:
        case TagName::polygon:
        case TagName::polyline:
        case TagName::rect:
        case TagName::svg:
        case TagName::switch_:
        case TagName::text:
        case TagName::use:

        // Not listed in the definitions is the clipPath element, the SVG spec says though:
        // The "clipPath" element or any of its children can specify property "clip-path".
        // So we have to add clipPathTag here, otherwhise clip-path on clipPath will fail.
        // (Already mailed SVG WG, waiting for a solution)
        case TagName::clipPath:

        // Not listed in the definitions are the text content elements, though filter/clipper/masker on
        // tspan/text/.. is allowed. (Already mailed SVG WG, waiting for a solution)
        case TagName::textPath:
        case TagName::tspan:

        // Not listed in the definitions is the foreignObject element, but clip-path
        // is a supported attribute.
        case TagName::foreignObject:
        {
          return true;
        }

        // Elements that we ignore, as it doesn't make any sense.
        // defs, pattern (FIXME: Mail SVG WG about these)
        // symbol (is converted to a svg element, when referenced by use, we can safely ignore it.)
        default:
        {
          return false;
        }
      }
    }

    KRYS_NODISCARD bool IsMarkerTag(html::TagName tagName) noexcept
    {
      using html::TagName;

      switch (tagName)
      {
        case TagName::line:
        case TagName::path:
        case TagName::polygon:
        case TagName::polyline:
        {
          return true;
        }
        default:
        {
          return false;
        }
      }
    }

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

    KRYS_NODISCARD bool IsChainableResourceTag(html::TagName tagName) noexcept
    {
      using html::TagName;

      switch (tagName)
      {
        case TagName::filter:
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
      else if (auto *svgFilter = DynamicDowncast<svg::SVGFilterElement>(element))
      {
        target = svgFilter->Href();
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

      if (Is<svg::SVGFilterElement>(element))
      {
        return Is<svg::SVGFilterElement>(linkedResource);
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
    if (IsClipperFilterMaskerTag(tagName))
    {
      if (auto clipPath = style.GetClipPath().TryReference())
      {
        // FIXME: -webkit-clip-path should support external resources
        // https://bugs.webkit.org/show_bug.cgi?id=127032
        if (auto *clipper =
              GetRenderSVGResourceById<LegacyRenderSVGResourceClipper>(treeScope, clipPath->Fragment()))
        {
          ensureResources(foundResources).SetClipper(clipper);
        }
        else
        {
          treeScope.AddPendingSVGResource(clipPath->Fragment(), *element);
        }
      }

      if (style.HasFilter())
      {
        const auto &filterOperations = style.GetFilter();
        if (filterOperations.size() == 1)
        {
          if (auto *referenceFilterOperation =
                DynamicDowncast<style::ReferenceFilterOperation>(filterOperations.begin()->Get()))
          {
            const auto &id = referenceFilterOperation->Fragment();
            if (auto *filter = GetRenderSVGResourceById<LegacyRenderSVGResourceFilter>(treeScope, id))
            {
              ensureResources(foundResources).SetFilter(filter);
            }
            else
            {
              treeScope.AddPendingSVGResource(id, *element);
            }
          }
        }
      }

      if (style.HasPositionedMask())
      {
        // FIXME: We should support all the values in the CSS mask property, but for now just use the first
        // mask-image if it's a reference.
        if (auto maskImage = style.GetMaskLayers().UsedFirst().GetImage().TryStyleImage())
        {
          auto resourceID = svg::SVGURIReference::FragmentIdentifierFromIRIString(maskImage->Url(), document);
          if (auto *masker = GetRenderSVGResourceById<LegacyRenderSVGResourceMasker>(treeScope, resourceID))
          {
            ensureResources(foundResources).SetMasker(masker);
          }
          else
          {
            treeScope.AddPendingSVGResource(resourceID, *element);
          }
        }
      }
    }

    if (IsMarkerTag(tagName) && style.HasMarkers())
    {
      auto buildCachedMarkerResource = [&](const style::SVGMarkerResource &markerResource,
                                           bool (SVGResources::*setMarker)(LegacyRenderSVGResourceMarker *))
      {
        auto markerId = svg::SVGURIReference::FragmentIdentifierFromIRIString(markerResource, document);
        if (auto *marker = GetRenderSVGResourceById<LegacyRenderSVGResourceMarker>(treeScope, markerId))
        {
          (ensureResources(foundResources).*setMarker)(marker);
        }
        else
        {
          treeScope.AddPendingSVGResource(markerId, *element);
        }
      };
      buildCachedMarkerResource(style.MarkerStart(), &SVGResources::SetMarkerStart);
      buildCachedMarkerResource(style.MarkerMid(), &SVGResources::SetMarkerMid);
      buildCachedMarkerResource(style.MarkerEnd(), &SVGResources::SetMarkerEnd);
    }

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
    const LegacyRenderSVGRoot *clientRoot = nullptr;

    auto layoutDifferentRootIfNeeded = [&](LegacyRenderSVGResourceContainer *container)
    {
      if (!container)
      {
        return;
      }

      auto *root = SVGRenderSupport::FindTreeRootObject(*container);
      if (root->IsInLayout())
      {
        return;
      }

      if (!clientRoot)
      {
        clientRoot = SVGRenderSupport::FindTreeRootObject(resourcesClient);
      }

      if (clientRoot == root)
      {
        return;
      }

      container->LayoutIfNeeded();
    };

    if (_clipperFilterMaskerData)
    {
      layoutDifferentRootIfNeeded(_clipperFilterMaskerData->Clipper.get());
      layoutDifferentRootIfNeeded(_clipperFilterMaskerData->Masker.get());
      layoutDifferentRootIfNeeded(_clipperFilterMaskerData->Filter.get());
    }

    if (_markerData)
    {
      layoutDifferentRootIfNeeded(_markerData->MarkerStart.get());
      layoutDifferentRootIfNeeded(_markerData->MarkerMid.get());
      layoutDifferentRootIfNeeded(_markerData->MarkerEnd.get());
    }
  }

  bool SVGResources::MarkerReverseStart() const noexcept
  {
    return _markerData && _markerData->MarkerStart
           && _markerData->MarkerStart->MarkerElement().OrientType() == svg::SVGMarkerOrientAutoStartReverse;
  }

  bool SVGResources::IsEmpty() const noexcept
  {
    return !_clipperFilterMaskerData && !_markerData && !_fillStrokeData && !_linkedResource;
  }

  LegacyRenderSVGResourceClipper *SVGResources::Clipper() const noexcept
  {
    return _clipperFilterMaskerData ? _clipperFilterMaskerData->Clipper.get() : nullptr;
  }

  LegacyRenderSVGResourceMasker *SVGResources::Masker() const noexcept
  {
    return _clipperFilterMaskerData ? _clipperFilterMaskerData->Masker.get() : nullptr;
  }

  LegacyRenderSVGResourceFilter *SVGResources::Filter() const noexcept
  {
    return _clipperFilterMaskerData ? _clipperFilterMaskerData->Filter.get() : nullptr;
  }

  LegacyRenderSVGResourceMarker *SVGResources::MarkerStart() const noexcept
  {
    return _markerData ? _markerData->MarkerStart.get() : nullptr;
  }

  LegacyRenderSVGResourceMarker *SVGResources::MarkerMid() const noexcept
  {
    return _markerData ? _markerData->MarkerMid.get() : nullptr;
  }

  LegacyRenderSVGResourceMarker *SVGResources::MarkerEnd() const noexcept
  {
    return _markerData ? _markerData->MarkerEnd.get() : nullptr;
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
      krys_debug_assert(!_clipperFilterMaskerData);
      krys_debug_assert(!_markerData);
      krys_debug_assert(!_fillStrokeData);
      _linkedResource->RemoveClientFromCacheAndMarkForInvalidation(renderer, markForInvalidation);
      return;
    }

    if (_clipperFilterMaskerData)
    {
      if (auto *clipper = _clipperFilterMaskerData->Clipper.get())
      {
        clipper->RemoveClientFromCacheAndMarkForInvalidation(renderer, markForInvalidation);
      }
      if (auto *filter = _clipperFilterMaskerData->Filter.get())
      {
        filter->RemoveClientFromCacheAndMarkForInvalidation(renderer, markForInvalidation);
      }
      if (auto *masker = _clipperFilterMaskerData->Masker.get())
      {
        masker->RemoveClientFromCacheAndMarkForInvalidation(renderer, markForInvalidation);
      }
    }

    if (_markerData)
    {
      if (auto *markerStart = _markerData->MarkerStart.get())
      {
        markerStart->RemoveClientFromCacheAndMarkForInvalidation(renderer, markForInvalidation);
      }
      if (auto *markerMid = _markerData->MarkerMid.get())
      {
        markerMid->RemoveClientFromCacheAndMarkForInvalidation(renderer, markForInvalidation);
      }
      if (auto *markerEnd = _markerData->MarkerEnd.get())
      {
        markerEnd->RemoveClientFromCacheAndMarkForInvalidation(renderer, markForInvalidation);
      }
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
      krys_debug_assert(!_clipperFilterMaskerData);
      krys_debug_assert(!_markerData);
      krys_debug_assert(!_fillStrokeData);
      _linkedResource->RemoveAllClientsFromCacheAndMarkForInvalidation();
      _linkedResource = nullptr;
      return true;
    }

    bool foundResources = false;
    switch (resource.ResourceType())
    {
      case RenderSVGResourceType::MaskerResourceType:
      {
        if (!_clipperFilterMaskerData)
        {
          break;
        }
        if (_clipperFilterMaskerData->Masker.get() == &resource)
        {
          resource.RemoveAllClientsFromCacheAndMarkForInvalidation();
          _clipperFilterMaskerData->Masker = nullptr;
          foundResources = true;
        }
        break;
      }
      case RenderSVGResourceType::MarkerResourceType:
      {
        if (!_markerData)
        {
          break;
        }
        if (_markerData->MarkerStart.get() == &resource)
        {
          resource.RemoveAllClientsFromCacheAndMarkForInvalidation();
          _markerData->MarkerStart = nullptr;
          foundResources = true;
        }
        if (_markerData->MarkerMid.get() == &resource)
        {
          resource.RemoveAllClientsFromCacheAndMarkForInvalidation();
          _markerData->MarkerMid = nullptr;
          foundResources = true;
        }
        if (_markerData->MarkerEnd.get() == &resource)
        {
          resource.RemoveAllClientsFromCacheAndMarkForInvalidation();
          _markerData->MarkerEnd = nullptr;
          foundResources = true;
        }
        break;
      }
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
      case RenderSVGResourceType::FilterResourceType:
      {
        if (!_clipperFilterMaskerData)
        {
          break;
        }
        if (_clipperFilterMaskerData->Filter.get() == &resource)
        {
          resource.RemoveAllClientsFromCacheAndMarkForInvalidation();
          _clipperFilterMaskerData->Filter = nullptr;
          foundResources = true;
        }
        break;
      }
      case RenderSVGResourceType::ClipperResourceType:
      {
        if (!_clipperFilterMaskerData)
        {
          break;
        }
        if (_clipperFilterMaskerData->Clipper.get() == &resource)
        {
          resource.RemoveAllClientsFromCacheAndMarkForInvalidation();
          _clipperFilterMaskerData->Clipper = nullptr;
          foundResources = true;
        }
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
      krys_debug_assert(!_clipperFilterMaskerData);
      krys_debug_assert(!_markerData);
      krys_debug_assert(!_fillStrokeData);
      set.insert(linkedResource);
      return;
    }

    if (_clipperFilterMaskerData)
    {
      if (auto *clipper = _clipperFilterMaskerData->Clipper.get())
      {
        set.insert(clipper);
      }
      if (auto *filter = _clipperFilterMaskerData->Filter.get())
      {
        set.insert(filter);
      }
      if (auto *masker = _clipperFilterMaskerData->Masker.get())
      {
        set.insert(masker);
      }
    }

    if (_markerData)
    {
      if (auto *markerStart = _markerData->MarkerStart.get())
      {
        set.insert(markerStart);
      }
      if (auto *markerMid = _markerData->MarkerMid.get())
      {
        set.insert(markerMid);
      }
      if (auto *markerEnd = _markerData->MarkerEnd.get())
      {
        set.insert(markerEnd);
      }
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

  bool SVGResources::SetClipper(LegacyRenderSVGResourceClipper *clipper) noexcept
  {
    krys_debug_assert(clipper);
    krys_debug_assert(clipper->ResourceType() == RenderSVGResourceType::ClipperResourceType);

    if (!_clipperFilterMaskerData)
    {
      _clipperFilterMaskerData = CreateUnique<ClipperFilterMaskerData>();
    }

    _clipperFilterMaskerData->Clipper = CreateWeakPtr(clipper);
    return true;
  }

  void SVGResources::ResetClipper() noexcept
  {
    krys_debug_assert(_clipperFilterMaskerData);
    krys_debug_assert(_clipperFilterMaskerData->Clipper);
    _clipperFilterMaskerData->Clipper = nullptr;
  }

  bool SVGResources::SetFilter(LegacyRenderSVGResourceFilter *filter) noexcept
  {
    krys_debug_assert(filter);
    krys_debug_assert(filter->ResourceType() == RenderSVGResourceType::FilterResourceType);

    if (!_clipperFilterMaskerData)
    {
      _clipperFilterMaskerData = CreateUnique<ClipperFilterMaskerData>();
    }

    _clipperFilterMaskerData->Filter = CreateWeakPtr(filter);
    return true;
  }

  void SVGResources::ResetFilter() noexcept
  {
    krys_debug_assert(_clipperFilterMaskerData);
    krys_debug_assert(_clipperFilterMaskerData->Filter);
    _clipperFilterMaskerData->Filter = nullptr;
  }

  bool SVGResources::SetMarkerStart(LegacyRenderSVGResourceMarker *markerStart) noexcept
  {
    krys_debug_assert(markerStart);
    krys_debug_assert(markerStart->ResourceType() == RenderSVGResourceType::MarkerResourceType);

    if (!_markerData)
    {
      _markerData = CreateUnique<MarkerData>();
    }

    _markerData->MarkerStart = CreateWeakPtr(markerStart);
    return true;
  }

  void SVGResources::ResetMarkerStart() noexcept
  {
    krys_debug_assert(_markerData);
    krys_debug_assert(_markerData->MarkerStart);
    _markerData->MarkerStart = nullptr;
  }

  bool SVGResources::SetMarkerMid(LegacyRenderSVGResourceMarker *markerMid) noexcept
  {
    krys_debug_assert(markerMid);
    krys_debug_assert(markerMid->ResourceType() == RenderSVGResourceType::MarkerResourceType);

    if (!_markerData)
    {
      _markerData = CreateUnique<MarkerData>();
    }

    _markerData->MarkerMid = CreateWeakPtr(markerMid);
    return true;
  }

  void SVGResources::ResetMarkerMid() noexcept
  {
    krys_debug_assert(_markerData);
    krys_debug_assert(_markerData->MarkerMid);
    _markerData->MarkerMid = nullptr;
  }

  bool SVGResources::SetMarkerEnd(LegacyRenderSVGResourceMarker *markerEnd) noexcept
  {
    krys_debug_assert(markerEnd);
    krys_debug_assert(markerEnd->ResourceType() == RenderSVGResourceType::MarkerResourceType);

    if (!_markerData)
    {
      _markerData = CreateUnique<MarkerData>();
    }

    _markerData->MarkerEnd = CreateWeakPtr(markerEnd);
    return true;
  }

  void SVGResources::ResetMarkerEnd() noexcept
  {
    krys_debug_assert(_markerData);
    krys_debug_assert(_markerData->MarkerEnd);
    _markerData->MarkerEnd = nullptr;
  }

  bool SVGResources::SetMasker(LegacyRenderSVGResourceMasker *masker) noexcept
  {
    krys_debug_assert(masker);
    krys_debug_assert(masker->ResourceType() == RenderSVGResourceType::MaskerResourceType);

    if (!_clipperFilterMaskerData)
    {
      _clipperFilterMaskerData = CreateUnique<ClipperFilterMaskerData>();
    }

    _clipperFilterMaskerData->Masker = CreateWeakPtr(masker);
    return true;
  }

  void SVGResources::ResetMasker() noexcept
  {
    krys_debug_assert(_clipperFilterMaskerData);
    krys_debug_assert(_clipperFilterMaskerData->Masker);
    _clipperFilterMaskerData->Masker = nullptr;
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
