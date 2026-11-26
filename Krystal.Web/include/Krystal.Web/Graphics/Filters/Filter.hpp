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

#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Geometry/Point3.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Graphics/Filters/FilterFunction.hpp"

namespace krys::web
{
  class FilterEffect;
  class FilterImage;
  class FilterResults;
  class ImageBuffer;

  using FilterEffectVector = List<Ref<FilterEffect>>;

  /// @brief Where a filter applies: the box it's relative to, the region it draws into, and its scale.
  struct FilterGeometry
  {
    RectF ReferenceBox;
    RectF FilterRegion;
    SizeF Scale;
  };

  /// @brief A whole filter: functions applied one after another, over a region.
  class Filter : public FilterFunction
  {
  private:
    FilterGeometry _geometry;
    EnumSet<FilterRenderingMode> _filterRenderingModes {FilterRenderingMode::Software};

  public:
    KRYS_NODISCARD EnumSet<FilterRenderingMode> FilterRenderingModes() const noexcept
    {
      return _filterRenderingModes;
    }

    void SetFilterRenderingModes(EnumSet<FilterRenderingMode> preferredFilterRenderingModes) noexcept;

    KRYS_NODISCARD const FilterGeometry &Geometry() const noexcept
    {
      return _geometry;
    }

    KRYS_NODISCARD SizeF FilterScale() const noexcept
    {
      return _geometry.Scale;
    }

    void SetFilterScale(const SizeF &filterScale) noexcept
    {
      _geometry.Scale = filterScale;
    }

    KRYS_NODISCARD RectF FilterRegion() const noexcept
    {
      return _geometry.FilterRegion;
    }

    void SetFilterRegion(const RectF &filterRegion) noexcept
    {
      _geometry.FilterRegion = filterRegion;
    }

    KRYS_NODISCARD RectF ReferenceBox() const noexcept
    {
      return _geometry.ReferenceBox;
    }

    KRYS_NODISCARD virtual SizeF ResolvedSize(const SizeF &size) const noexcept
    {
      return size;
    }

    KRYS_NODISCARD virtual Point3F ResolvedPoint3D(const Point3F &point) const noexcept
    {
      return point;
    }

    KRYS_NODISCARD PointF ScaledByFilterScale(const PointF &point) const noexcept;

    KRYS_NODISCARD SizeF ScaledByFilterScale(const SizeF &size) const noexcept;

    KRYS_NODISCARD RectF ScaledByFilterScale(const RectF &rect) const noexcept;

    KRYS_NODISCARD RectF MaxEffectRect(const RectF &primitiveSubregion) const noexcept;

    KRYS_NODISCARD RectF ClipToMaxEffectRect(const RectF &imageRect,
                                             const RectF &primitiveSubregion) const noexcept;

    KRYS_NODISCARD virtual FilterEffectVector EffectsOfType(FilterFunction::Type) const noexcept = 0;

    /// @brief Scales the filter down if its region, at its scale, is too big for an image buffer.
    /// @return Whether it did.
    bool ClampFilterRegionIfNeeded() noexcept;

    /// @brief The filter applied to @p sourceImage, which holds @p sourceImageRect (in filter coordinates);
    /// null when it can't be.
    KRYS_NODISCARD RefPtr<FilterImage> Apply(ImageBuffer *sourceImage, const RectF &sourceImageRect,
                                             FilterResults &results) noexcept;

    /// @brief The styles that apply the filter in the graphics context, the last function's first: each is a
    /// transparency layer, the first outermost.
    KRYS_NODISCARD FilterStyleVector CreateFilterStyles(GraphicsContext &context,
                                                        const RectF &sourceImageRect) const noexcept;

    /// @brief The image buffer holding @p filterImage's pixels; null when an accelerated backend holds them.
    KRYS_NODISCARD static ImageBuffer *FilterResultBuffer(FilterImage &filterImage) noexcept;

  protected:
    explicit Filter(FilterFunction::Type type) noexcept;

    Filter(FilterFunction::Type type, const FilterGeometry &geometry) noexcept;

    KRYS_NODISCARD virtual RefPtr<FilterImage> Apply(FilterImage *sourceImage, FilterResults &) noexcept = 0;

    KRYS_NODISCARD virtual FilterStyleVector
      CreateFilterStyles(GraphicsContext &, const FilterStyle &sourceStyle) const noexcept = 0;

  private:
    using FilterFunction::Apply;
    using FilterFunction::CreateFilterStyles;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::Filter)
  KRYS_NODISCARD static bool IsType(const krys::web::FilterFunction &function) noexcept
  {
    return function.IsFilter();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
