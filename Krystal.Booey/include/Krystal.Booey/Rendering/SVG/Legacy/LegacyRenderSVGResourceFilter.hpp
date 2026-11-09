/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2009 Dirk Schulze <krit@webkit.org>
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

#include "Krystal.Booey/Graphics/GraphicsContextSwitcher.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceContainer.hpp"
#include "Krystal.Booey/SVG/Graphics/Filters/SVGFilterRenderer.hpp"
#include "Krystal.Booey/SVG/SVGUnitTypes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo::svg
{
  class SVGFilterElement;
}

namespace krys::boo
{
  class FilterEffect;
  class GraphicsContext;

  /// @brief A filter as it's applied to one client: drawing its source, then applying the filter.
  struct FilterData
  {
    enum FilterDataState : uint8
    {
      PaintingSource,
      Applying,
      Built,
      CycleDetected,
      MarkedForRemoval
    };

    RefPtr<SVGFilterRenderer> Filter;
    UniquePtr<GraphicsContextSwitcher> TargetSwitcher;
    FloatRect SourceImageRect;
    GraphicsContext *SavedContext {nullptr};
    FilterDataState State {PaintingSource};
  };

  /// @brief The renderer of a `<filter>`: its clients are drawn into an image the filter is applied to.
  class LegacyRenderSVGResourceFilter final : public LegacyRenderSVGResourceContainer
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGResourceFilter);

  private:
    HashMap<const RenderObject *, UniquePtr<FilterData>> _rendererFilterDataMap;

  public:
    LegacyRenderSVGResourceFilter(svg::SVGFilterElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGResourceFilter() noexcept override;

    KRYS_NODISCARD svg::SVGFilterElement &FilterElement() const noexcept;

    KRYS_NODISCARD bool IsIdentity() const noexcept;

    void RemoveAllClientsFromCache() noexcept override;
    void RemoveClientFromCache(RenderElement &) noexcept override;

    EnumSet<ApplyResult> ApplyResource(RenderElement &, const RenderStyle &, GraphicsContext *&,
                                       EnumSet<RenderSVGResourceMode>) noexcept override;
    void PostApplyResource(RenderElement &, GraphicsContext *&, EnumSet<RenderSVGResourceMode>, const Path *,
                           const RenderElement *) noexcept override;

    KRYS_NODISCARD FloatRect ResourceBoundingBox(const RenderObject &,
                                                 RepaintRectCalculation) noexcept override;

    KRYS_NODISCARD svg::SVGUnitTypes::SVGUnitType FilterUnits() const noexcept;
    KRYS_NODISCARD svg::SVGUnitTypes::SVGUnitType PrimitiveUnits() const noexcept;

    void MarkFilterForRepaint(FilterEffect &) noexcept;
    void MarkFilterForRebuild() noexcept;

    KRYS_NODISCARD RenderSVGResourceType ResourceType() const noexcept override
    {
      return RenderSVGResourceType::FilterResourceType;
    }

    KRYS_NODISCARD FloatRect DrawingRegion(RenderObject &) const noexcept;

  private:
    void GetElement() const = delete;
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGResourceFilter";
    }
  };

  TextStream &operator<<(TextStream &, FilterData::FilterDataState) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGResourceFilter)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGResourceFilter();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
