#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class LegacyRenderSVGResourceContainer;
  class RenderElement;
  class RenderStyle;

  // Holds a set of resources associated with a RenderObject
  // FIXME: The clipper, filter and masker (ClipperFilterMaskerData), with clipping and masking (B11f) and
  // filters (B11h); the markers (MarkerData, markerReverseStart), with markers (B11f).
  class SVGResources
  {
    friend class SVGResourcesCycleSolver;
    KRYS_NON_COPY_MOVABLE(SVGResources);

  public:
    using ResourceSet = HashSet<LegacyRenderSVGResourceContainer *>;

  private:
    // From SVG 1.1 2nd Edition
    // fill:       'shapes' and 'text content elements'
    // stroke:     'shapes' and 'text content elements'
    // -> altGlyph, circle, ellipse, line, path, polygon, polyline, rect, text, textPath, tref, tspan
    struct FillStrokeData
    {
      WeakPtr<LegacyRenderSVGResourceContainer> Fill;
      WeakPtr<LegacyRenderSVGResourceContainer> Stroke;
    };

    UniquePtr<FillStrokeData> _fillStrokeData;
    WeakPtr<LegacyRenderSVGResourceContainer> _linkedResource;
    bool _inLayoutForDifferentRoot {false};

  public:
    SVGResources() noexcept;

    KRYS_NODISCARD static UniquePtr<SVGResources> BuildCachedResources(const RenderElement &,
                                                                       const RenderStyle &) noexcept;
    void LayoutDifferentRootIfNeeded(const RenderElement &resourcesClient) noexcept;

    // Paint servers
    KRYS_NODISCARD LegacyRenderSVGResourceContainer *Fill() const noexcept;
    KRYS_NODISCARD LegacyRenderSVGResourceContainer *Stroke() const noexcept;

    // Chainable resources - linked through xlink:href
    KRYS_NODISCARD LegacyRenderSVGResourceContainer *LinkedResource() const noexcept;

    void BuildSetOfResources(ResourceSet &) noexcept;

    // Methods operating on all cached resources
    void RemoveClientFromCacheAndMarkForInvalidation(RenderElement &,
                                                     bool markForInvalidation = true) const noexcept;
    // Returns true if the resource-to-be-destroyed is one of our resources.
    bool ResourceDestroyed(LegacyRenderSVGResourceContainer &) noexcept;

  private:
    // Only used by SVGResourcesCache cycle detection logic
    void ResetFill() noexcept;
    void ResetStroke() noexcept;
    void ResetLinkedResource() noexcept;

    bool SetFill(LegacyRenderSVGResourceContainer *) noexcept;
    bool SetStroke(LegacyRenderSVGResourceContainer *) noexcept;
    bool SetLinkedResource(LegacyRenderSVGResourceContainer *) noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept;
  };
}
