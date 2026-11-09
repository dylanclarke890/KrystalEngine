/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class LegacyRenderSVGResourceClipper;
  class LegacyRenderSVGResourceFilter;
  class LegacyRenderSVGResourceContainer;
  class LegacyRenderSVGResourceMarker;
  class LegacyRenderSVGResourceMasker;
  class RenderElement;
  class RenderStyle;

  // Holds a set of resources associated with a RenderObject
  class SVGResources
  {
    friend class SVGResourcesCycleSolver;
    KRYS_NON_COPY_MOVABLE(SVGResources);

  public:
    using ResourceSet = HashSet<LegacyRenderSVGResourceContainer *>;

  private:
    // From SVG 1.1 2nd Edition
    // clipper: 'container elements' and 'graphics elements'
    // filter:  'container elements' and 'graphics elements'
    // masker:  'container elements' and 'graphics elements'
    // -> a, circle, defs, ellipse, glyph, g, image, line, marker, mask, missing-glyph, path, pattern,
    // polygon, polyline, rect, svg, switch, symbol, text, use
    struct ClipperFilterMaskerData
    {
      WeakPtr<LegacyRenderSVGResourceClipper> Clipper;
      WeakPtr<LegacyRenderSVGResourceFilter> Filter;
      WeakPtr<LegacyRenderSVGResourceMasker> Masker;
    };

    // From SVG 1.1 2nd Edition
    // marker: line, path, polygon, polyline
    struct MarkerData
    {
      WeakPtr<LegacyRenderSVGResourceMarker> MarkerStart;
      WeakPtr<LegacyRenderSVGResourceMarker> MarkerMid;
      WeakPtr<LegacyRenderSVGResourceMarker> MarkerEnd;
    };

    // From SVG 1.1 2nd Edition
    // fill:       'shapes' and 'text content elements'
    // stroke:     'shapes' and 'text content elements'
    // -> altGlyph, circle, ellipse, line, path, polygon, polyline, rect, text, textPath, tref, tspan
    struct FillStrokeData
    {
      WeakPtr<LegacyRenderSVGResourceContainer> Fill;
      WeakPtr<LegacyRenderSVGResourceContainer> Stroke;
    };

    UniquePtr<ClipperFilterMaskerData> _clipperFilterMaskerData;
    UniquePtr<MarkerData> _markerData;
    UniquePtr<FillStrokeData> _fillStrokeData;
    WeakPtr<LegacyRenderSVGResourceContainer> _linkedResource;
    bool _inLayoutForDifferentRoot {false};

  public:
    SVGResources() noexcept;

    KRYS_NODISCARD static UniquePtr<SVGResources> BuildCachedResources(const RenderElement &,
                                                                       const RenderStyle &) noexcept;
    void LayoutDifferentRootIfNeeded(const RenderElement &resourcesClient) noexcept;

    KRYS_NODISCARD bool MarkerReverseStart() const noexcept;

    // Ordinary resources
    KRYS_NODISCARD LegacyRenderSVGResourceClipper *Clipper() const noexcept;
    KRYS_NODISCARD LegacyRenderSVGResourceMasker *Masker() const noexcept;
    KRYS_NODISCARD LegacyRenderSVGResourceFilter *Filter() const noexcept;

    KRYS_NODISCARD LegacyRenderSVGResourceMarker *MarkerStart() const noexcept;
    KRYS_NODISCARD LegacyRenderSVGResourceMarker *MarkerMid() const noexcept;
    KRYS_NODISCARD LegacyRenderSVGResourceMarker *MarkerEnd() const noexcept;

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
    void ResetClipper() noexcept;
    void ResetFilter() noexcept;
    void ResetMarkerStart() noexcept;
    void ResetMarkerMid() noexcept;
    void ResetMarkerEnd() noexcept;
    void ResetMasker() noexcept;
    void ResetFill() noexcept;
    void ResetStroke() noexcept;
    void ResetLinkedResource() noexcept;

    bool SetClipper(LegacyRenderSVGResourceClipper *) noexcept;
    bool SetFilter(LegacyRenderSVGResourceFilter *) noexcept;
    bool SetMarkerStart(LegacyRenderSVGResourceMarker *) noexcept;
    bool SetMarkerMid(LegacyRenderSVGResourceMarker *) noexcept;
    bool SetMarkerEnd(LegacyRenderSVGResourceMarker *) noexcept;
    bool SetMasker(LegacyRenderSVGResourceMasker *) noexcept;
    bool SetFill(LegacyRenderSVGResourceContainer *) noexcept;
    bool SetStroke(LegacyRenderSVGResourceContainer *) noexcept;
    bool SetLinkedResource(LegacyRenderSVGResourceContainer *) noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept;
  };
}
