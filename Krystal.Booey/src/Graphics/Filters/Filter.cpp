/*
 * Copyright (C) 2021-2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterResults.hpp"
#include "Krystal.Booey/Graphics/ImageBuffers/ImageBuffer.hpp"
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
