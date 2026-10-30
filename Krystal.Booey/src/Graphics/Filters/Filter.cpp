#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterResults.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include <algorithm>

namespace krys::boo
{
  Filter::Filter(FilterFunction::Type type) noexcept : FilterFunction(type)
  {
  }

  Filter::Filter(FilterFunction::Type type, const FilterGeometry &geometry) noexcept
      : FilterFunction(type), _geometry(geometry)
  {
  }

  FloatPoint Filter::ScaledByFilterScale(const FloatPoint &point) const noexcept
  {
    return point.Scaled(_geometry.Scale.Width(), _geometry.Scale.Height());
  }

  FloatSize Filter::ScaledByFilterScale(const FloatSize &size) const noexcept
  {
    return size * _geometry.Scale;
  }

  FloatRect Filter::ScaledByFilterScale(const FloatRect &rect) const noexcept
  {
    auto scaledRect = rect;
    scaledRect.Scale(_geometry.Scale);
    return scaledRect;
  }

  FloatRect Filter::MaxEffectRect(const FloatRect &primitiveSubregion) const noexcept
  {
    return Intersection(primitiveSubregion, _geometry.FilterRegion);
  }

  FloatRect Filter::ClipToMaxEffectRect(const FloatRect &imageRect,
                                        const FloatRect &primitiveSubregion) const noexcept
  {
    auto maxEffectRect = MaxEffectRect(primitiveSubregion);
    return Intersection(imageRect, maxEffectRect);
  }

  bool Filter::ClampFilterRegionIfNeeded() noexcept
  {
    auto scaledFilterRegion = ScaledByFilterScale(_geometry.FilterRegion);

    FloatSize clampingScale(1, 1);
    if (!ImageBuffer::SizeNeedsClamping(scaledFilterRegion.Size(), clampingScale))
    {
      return false;
    }

    _geometry.Scale = _geometry.Scale * clampingScale;
    return true;
  }

  void Filter::SetFilterRenderingModes(EnumSet<FilterRenderingMode> preferredFilterRenderingModes) noexcept
  {
    _filterRenderingModes = SupportedFilterRenderingModes(preferredFilterRenderingModes);
    // There's no software rendering, so some mode the page prefers must be supported.
    krys_debug_assert(!_filterRenderingModes.IsEmpty());
  }

  RefPtr<FilterImage> Filter::Apply(ImageBuffer *sourceImage, const FloatRect &sourceImageRect,
                                    FilterResults &results) noexcept
  {
    RefPtr<FilterImage> input;

    if (sourceImage)
    {
      auto absoluteSourceImageRect = EnclosingIntRect(ScaledByFilterScale(sourceImageRect));
      input = FilterImage::Create(_geometry.FilterRegion, sourceImageRect, absoluteSourceImageRect,
                                  ShareRef(*sourceImage));
    }

    auto result = Apply(input.get(), results);
    if (!result)
    {
      return nullptr;
    }

    result->TransformToColorSpace(DestinationColorSpace::SRGB());
    return result;
  }

  ImageBuffer *Filter::FilterResultBuffer(FilterImage &filterImage) const noexcept
  {
    return filterImage.GetImageBuffer();
  }

  FilterStyleVector Filter::CreateFilterStyles(GraphicsContext &context,
                                               const FloatRect &sourceImageRect) const noexcept
  {
    auto input = FilterStyle {null, _geometry.FilterRegion, sourceImageRect};
    auto result = CreateFilterStyles(context, input);
    if (result.empty())
    {
      return {};
    }

    std::ranges::reverse(result);
    result.shrink_to_fit();
    return result;
  }
}
