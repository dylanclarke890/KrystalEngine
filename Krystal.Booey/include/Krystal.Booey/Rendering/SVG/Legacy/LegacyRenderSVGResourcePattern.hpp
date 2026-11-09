/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Graphics/Pattern.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceContainer.hpp"
#include "Krystal.Booey/SVG/PatternAttributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::svg
{
  class SVGPatternElement;
}

namespace krys::boo
{
  class ImageBuffer;

  /// @brief A pattern built for one client, and its transform from pattern space.
  struct PatternData
  {
    RefPtr<boo::Pattern> Pattern;
    AffineTransform Transform;
  };

  /// @brief The renderer of a `<pattern>`: draws the tile each client paints with.
  class LegacyRenderSVGResourcePattern final : public LegacyRenderSVGResourceContainer
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGResourcePattern);

  private:
    svg::PatternAttributes _attributes;
    HashMap<const RenderElement *, UniquePtr<PatternData>> _patternMap;
    bool _shouldCollectPatternAttributes {true};

  public:
    LegacyRenderSVGResourcePattern(svg::SVGPatternElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGResourcePattern() noexcept override;

    KRYS_NODISCARD svg::SVGPatternElement &PatternElement() const noexcept;

    void RemoveAllClientsFromCache() noexcept override;
    void RemoveAllClientsFromCacheAndMarkForInvalidationIfNeeded(
      bool markForInvalidation, VisitedRenderers *visitedRenderers) noexcept override;
    void RemoveClientFromCache(RenderElement &) noexcept override;

    EnumSet<ApplyResult> ApplyResource(RenderElement &, const RenderStyle &, GraphicsContext *&,
                                       EnumSet<RenderSVGResourceMode>) noexcept override;
    void PostApplyResource(RenderElement &, GraphicsContext *&, EnumSet<RenderSVGResourceMode>, const Path *,
                           const RenderElement *) noexcept override;
    KRYS_NODISCARD FloatRect ResourceBoundingBox(const RenderObject &,
                                                 RepaintRectCalculation) noexcept override
    {
      return FloatRect();
    }

    KRYS_NODISCARD RenderSVGResourceType ResourceType() const noexcept override
    {
      return RenderSVGResourceType::PatternResourceType;
    }

    void CollectPatternAttributes(svg::PatternAttributes &) const noexcept;

  private:
    void GetElement() const = delete;
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGResourcePattern";
    }

    bool BuildTileImageTransform(RenderElement &, const svg::PatternAttributes &,
                                 const svg::SVGPatternElement &, FloatRect &patternBoundaries,
                                 AffineTransform &tileImageTransform) const noexcept;

    KRYS_NODISCARD RefPtr<ImageBuffer> CreateTileImage(GraphicsContext &, const FloatSize &,
                                                       const FloatSize &scale,
                                                       const AffineTransform &tileImageTransform,
                                                       const svg::PatternAttributes &) const noexcept;

    KRYS_NODISCARD PatternData *BuildPattern(RenderElement &, EnumSet<RenderSVGResourceMode>,
                                             GraphicsContext &) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGResourcePattern)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.GetType() == krys::boo::RenderObject::Type::LegacySVGResourcePattern;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
