#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterFunction.hpp"
#include "Krystal.Booey/Graphics/FloatPoint3D.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class FilterEffect;
  class FilterImage;
  class FilterResults;
  class ImageBuffer;

  using FilterEffectVector = List<Ref<FilterEffect>>;

  /// @brief Where a filter applies: the box it's relative to, the region it draws into, and its scale.
  struct FilterGeometry
  {
    FloatRect ReferenceBox;
    FloatRect FilterRegion;
    FloatSize Scale;
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

    KRYS_NODISCARD FloatSize FilterScale() const noexcept
    {
      return _geometry.Scale;
    }
    void SetFilterScale(const FloatSize &filterScale) noexcept
    {
      _geometry.Scale = filterScale;
    }

    KRYS_NODISCARD FloatRect FilterRegion() const noexcept
    {
      return _geometry.FilterRegion;
    }
    void SetFilterRegion(const FloatRect &filterRegion) noexcept
    {
      _geometry.FilterRegion = filterRegion;
    }

    KRYS_NODISCARD FloatRect ReferenceBox() const noexcept
    {
      return _geometry.ReferenceBox;
    }

    KRYS_NODISCARD virtual FloatSize ResolvedSize(const FloatSize &size) const noexcept
    {
      return size;
    }
    KRYS_NODISCARD virtual FloatPoint3D ResolvedPoint3D(const FloatPoint3D &point) const noexcept
    {
      return point;
    }

    KRYS_NODISCARD FloatPoint ScaledByFilterScale(const FloatPoint &point) const noexcept;
    KRYS_NODISCARD FloatSize ScaledByFilterScale(const FloatSize &size) const noexcept;
    KRYS_NODISCARD FloatRect ScaledByFilterScale(const FloatRect &rect) const noexcept;

    KRYS_NODISCARD FloatRect MaxEffectRect(const FloatRect &primitiveSubregion) const noexcept;
    KRYS_NODISCARD FloatRect ClipToMaxEffectRect(const FloatRect &imageRect,
                                                 const FloatRect &primitiveSubregion) const noexcept;

    KRYS_NODISCARD virtual FilterEffectVector EffectsOfType(FilterFunction::Type) const noexcept = 0;

    /// @brief Scales the filter down if its region, at its scale, is too big for an image buffer.
    /// @return Whether it did.
    bool ClampFilterRegionIfNeeded() noexcept;

    /// @brief The filter applied to @p sourceImage, which holds @p sourceImageRect (in filter coordinates);
    /// null when it can't be.
    KRYS_NODISCARD RefPtr<FilterImage> Apply(ImageBuffer *sourceImage, const FloatRect &sourceImageRect,
                                             FilterResults &results) noexcept;

    /// @brief The styles that apply the filter in the graphics context, the last function's first: each is a
    /// transparency layer, the first outermost.
    KRYS_NODISCARD FilterStyleVector CreateFilterStyles(GraphicsContext &context,
                                                        const FloatRect &sourceImageRect) const noexcept;

    /// @brief The image buffer holding @p filterImage's pixels; null when an accelerated backend holds them.
    KRYS_NODISCARD ImageBuffer *FilterResultBuffer(FilterImage &filterImage) const noexcept;

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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::Filter)
  KRYS_NODISCARD static bool IsType(const krys::boo::FilterFunction &function) noexcept
  {
    return function.IsFilter();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
