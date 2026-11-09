/*
 * Copyright (C) 2009 Dirk Schulze <krit@webkit.org>
 * Copyright (C) 2013 Google Inc. All rights reserved.
 * Copyright (C) 2021-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterResults.hpp"
#include "Krystal.Booey/Graphics/Geometry/BoxExtents.hpp"
#include "Krystal.Booey/SVG/Graphics/Filters/SVGFilterExpression.hpp"
#include "Krystal.Booey/SVG/SVGUnitTypes.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class FilterImage;
  class GraphicsContext;

  namespace svg
  {
    class SVGElement;
    class SVGFilterElement;
  }

  /// @brief Makes the results cache of a filter.
  using FilterResultsCreator = Func<UniquePtr<FilterResults>()>;

  /// @brief The filter a `<filter>` element builds: its primitives' effects, as an expression over its graph.
  class SVGFilterRenderer final : public Filter
  {
  private:
    svg::SVGUnitTypes::SVGUnitType _primitiveUnits;

    SVGFilterExpression _expression;
    FilterEffectVector _effects;

    UniquePtr<FilterResults> _results;

  public:
    KRYS_NODISCARD static RefPtr<SVGFilterRenderer>
      Create(svg::SVGElement *contextElement, svg::SVGFilterElement &filterElement,
             const FilterGeometry &geometry, EnumSet<FilterRenderingMode> preferredRenderingModes,
             const GraphicsContext &destinationContext) noexcept;
    KRYS_NODISCARD static Ref<SVGFilterRenderer>
      Create(svg::SVGUnitTypes::SVGUnitType primitiveUnits, SVGFilterExpression &&expression,
             FilterEffectVector &&effects, const FilterGeometry &geometry,
             EnumSet<FilterRenderingMode> preferredRenderingModes) noexcept;

    KRYS_NODISCARD static bool IsIdentity(svg::SVGFilterElement &filterElement) noexcept;
    KRYS_NODISCARD static IntOutsets CalculateOutsets(svg::SVGFilterElement &filterElement,
                                                      const FloatRect &targetBoundingBox) noexcept;

    KRYS_NODISCARD svg::SVGUnitTypes::SVGUnitType PrimitiveUnits() const noexcept
    {
      return _primitiveUnits;
    }

    KRYS_NODISCARD const SVGFilterExpression &Expression() const noexcept
    {
      return _expression;
    }
    KRYS_NODISCARD const FilterEffectVector &Effects() const noexcept
    {
      return _effects;
    }

    KRYS_NODISCARD FilterEffectVector EffectsOfType(FilterFunction::Type filterType) const noexcept final;

    FilterResults &EnsureResults(const FilterResultsCreator &resultsCreator) noexcept;
    void ClearEffectResult(FilterEffect &effect) noexcept;
    void MergeEffects(const FilterEffectVector &effects) noexcept;

    KRYS_NODISCARD RefPtr<FilterImage> Apply(FilterImage *sourceImage, FilterResults &results) noexcept final;
    KRYS_NODISCARD FilterStyleVector CreateFilterStyles(GraphicsContext &context,
                                                        const FilterStyle &sourceStyle) const noexcept final;

    KRYS_NODISCARD static FloatSize
      CalculateResolvedSize(const FloatSize &size, const FloatRect &targetBoundingBox,
                            svg::SVGUnitTypes::SVGUnitType primitiveUnits) noexcept;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept final;

    KRYS_NODISCARD static bool IsValidSVGFilterExpression(const SVGFilterExpression &expression,
                                                          const FilterEffectVector &effects) noexcept;

  private:
    SVGFilterRenderer(const FilterGeometry &geometry, svg::SVGUnitTypes::SVGUnitType primitiveUnits) noexcept;
    SVGFilterRenderer(const FilterGeometry &geometry, svg::SVGUnitTypes::SVGUnitType primitiveUnits,
                      SVGFilterExpression &&expression, FilterEffectVector &&effects) noexcept;

    struct BuiltExpression
    {
      SVGFilterExpression Expression;
      FilterEffectVector Effects;
    };

    KRYS_NODISCARD static Maybe<BuiltExpression>
      BuildExpression(svg::SVGElement *contextElement, svg::SVGFilterElement &filterElement,
                      const SVGFilterRenderer &filter, const GraphicsContext &destinationContext) noexcept;
    void SetExpression(SVGFilterExpression &&expression) noexcept
    {
      _expression = krys::move(expression);
    }
    void SetEffects(FilterEffectVector &&effects) noexcept
    {
      _effects = krys::move(effects);
    }

    KRYS_NODISCARD FloatSize ResolvedSize(const FloatSize &size) const noexcept final;
    KRYS_NODISCARD FloatPoint3D ResolvedPoint3D(const FloatPoint3D &point) const noexcept final;

    KRYS_NODISCARD EnumSet<FilterRenderingMode> SupportedFilterRenderingModes(
      EnumSet<FilterRenderingMode> preferredFilterRenderingModes) const noexcept final;

    KRYS_NODISCARD RefPtr<FilterImage> Apply(const Filter &, FilterImage &sourceImage,
                                             FilterResults &results) noexcept final;
    KRYS_NODISCARD FilterStyleVector CreateFilterStyles(GraphicsContext &context, const Filter &,
                                                        const FilterStyle &sourceStyle) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(SVGFilterRenderer)
