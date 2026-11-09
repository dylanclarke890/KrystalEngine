/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Rendering/RepaintRectCalculation.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/HashSet.hpp"

namespace krys::boo
{
  class Color;
  class GraphicsContext;
  class LegacyRenderSVGResourceSolidColor;
  class Path;
  class RenderElement;
  class RenderObject;
  class RenderStyle;

  enum class RenderSVGResourceType : uint8
  {
    MaskerResourceType,
    MarkerResourceType,
    PatternResourceType,
    LinearGradientResourceType,
    RadialGradientResourceType,
    SolidColorResourceType,
    FilterResourceType,
    ClipperResourceType
  };

  enum class RenderSVGResourceMode : uint8
  {
    ApplyToFill,
    ApplyToStroke,
    ApplyToText // used in combination with ApplyTo{Fill|Stroke}Mode
  };

  enum class LegacyRenderSVGResourceApplyResult : uint8
  {
    ResourceApplied,
    ClipContainsRendererContent
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::RenderSVGResourceMode, 3uz);
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::LegacyRenderSVGResourceApplyResult, 2uz);

namespace krys::boo
{
  /// @brief A paint server, clipper, masker, marker or filter that SVG renderers apply around their painting.
  class LegacyRenderSVGResource
  {
  public:
    using ApplyResult = LegacyRenderSVGResourceApplyResult;
    using VisitedRenderers = HashSet<const RenderObject *>;

    LegacyRenderSVGResource() noexcept = default;
    virtual ~LegacyRenderSVGResource() noexcept = default;

    void RemoveAllClientsFromCacheAndMarkForInvalidation(bool markForInvalidation = true) noexcept;
    virtual void RemoveAllClientsFromCache() noexcept = 0;
    virtual void RemoveAllClientsFromCacheAndMarkForInvalidationIfNeeded(
      bool markForInvalidation, VisitedRenderers *visitedRenderers) noexcept = 0;
    virtual void RemoveClientFromCache(RenderElement &) noexcept = 0;
    virtual void RemoveClientFromCacheAndMarkForInvalidation(RenderElement &,
                                                             bool markForInvalidation = true) noexcept = 0;

    virtual EnumSet<ApplyResult> ApplyResource(RenderElement &, const RenderStyle &, GraphicsContext *&,
                                               EnumSet<RenderSVGResourceMode>) noexcept = 0;
    virtual void PostApplyResource(RenderElement &, GraphicsContext *&, EnumSet<RenderSVGResourceMode>,
                                   const Path *, const RenderElement * /* shape */) noexcept
    {
    }
    KRYS_NODISCARD virtual FloatRect ResourceBoundingBox(const RenderObject &,
                                                         RepaintRectCalculation) noexcept = 0;

    KRYS_NODISCARD virtual RenderSVGResourceType ResourceType() const noexcept = 0;

    // Helper utilities used in the render tree to access resources used for painting shapes/text (gradients &
    // patterns & solid colors only)
    KRYS_NODISCARD static LegacyRenderSVGResource *FillPaintingResource(RenderElement &, const RenderStyle &,
                                                                        Color &fallbackColor) noexcept;
    KRYS_NODISCARD static LegacyRenderSVGResource *
      StrokePaintingResource(RenderElement &, const RenderStyle &, Color &fallbackColor) noexcept;
    KRYS_NODISCARD static LegacyRenderSVGResourceSolidColor *SharedSolidPaintingResource() noexcept;

    static void MarkForLayoutAndParentResourceInvalidation(RenderObject &, bool needsLayout = true) noexcept;
    static void
      MarkForLayoutAndParentResourceInvalidationIfNeeded(RenderObject &, bool needsLayout,
                                                         VisitedRenderers *visitedRenderers) noexcept;

    static void FillAndStrokePathOrShape(GraphicsContext &, EnumSet<RenderSVGResourceMode>, const Path *,
                                         const RenderElement *shape) noexcept;
  };

  KRYS_NODISCARD constexpr bool
    ResourceWasApplied(EnumSet<LegacyRenderSVGResource::ApplyResult> result) noexcept
  {
    return result.Contains(LegacyRenderSVGResource::ApplyResult::ResourceApplied);
  }
}
