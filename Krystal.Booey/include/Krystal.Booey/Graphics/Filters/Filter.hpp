#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterFunction.hpp"
#include "Krystal.Booey/Graphics/FloatPoint3D.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class FilterEffect;

  using FilterEffectVector = List<Ref<FilterEffect>>;

  /// @brief Where a filter applies: the box it's relative to, the region it draws into, and its scale.
  struct FilterGeometry
  {
    FloatRect ReferenceBox;
    FloatRect FilterRegion;
    FloatSize Scale;
  };

  /// @brief A whole filter: functions applied one after another, over a region.
  // FIXME: Applying the filter to an image buffer (apply, filterResultBuffer) and the rendering mode it's
  // applied in (renderingMode), once there's a rendering mode other than the graphics context.
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

    /// @brief The styles that apply the filter in the graphics context, the last function's first: each is a
    /// transparency layer, the first outermost.
    KRYS_NODISCARD FilterStyleVector CreateFilterStyles(GraphicsContext &context,
                                                        const FloatRect &sourceImageRect) const noexcept;

  protected:
    explicit Filter(FilterFunction::Type type) noexcept;
    Filter(FilterFunction::Type type, const FilterGeometry &geometry) noexcept;

    KRYS_NODISCARD virtual FilterStyleVector
      CreateFilterStyles(GraphicsContext &, const FilterStyle &sourceStyle) const noexcept = 0;

  private:
    using FilterFunction::CreateFilterStyles;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::Filter)
  KRYS_NODISCARD static bool IsType(const krys::boo::FilterFunction &function) noexcept
  {
    return function.IsFilter();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
