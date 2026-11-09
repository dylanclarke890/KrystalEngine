/*
 * Copyright (C) 2021-2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"

namespace krys::boo
{
  Ref<FilterImage> FilterImage::Create(const FloatRect &primitiveSubregion, const FloatRect &imageRect,
                                       const IntRect &absoluteImageRect, bool isAlphaImage,
                                       bool isValidPremultiplied,
                                       const DestinationColorSpace &colorSpace) noexcept
  {
    krys_debug_assert(!ImageBuffer::SizeNeedsClamping(FloatSize(absoluteImageRect.Size())));
    return AdoptRef(*new FilterImage(primitiveSubregion, imageRect, absoluteImageRect, isAlphaImage,
                                     isValidPremultiplied, colorSpace));
  }

  Ref<FilterImage> FilterImage::Create(const FloatRect &primitiveSubregion, const FloatRect &imageRect,
                                       const IntRect &absoluteImageRect,
                                       Ref<ImageBuffer> &&imageBuffer) noexcept
  {
    return AdoptRef(
      *new FilterImage(primitiveSubregion, imageRect, absoluteImageRect, krys::move(imageBuffer)));
  }

  FilterImage::FilterImage(const FloatRect &primitiveSubregion, const FloatRect &imageRect,
                           const IntRect &absoluteImageRect, bool isAlphaImage, bool isValidPremultiplied,
                           const DestinationColorSpace &colorSpace) noexcept
      : _primitiveSubregion(primitiveSubregion), _imageRect(imageRect), _absoluteImageRect(absoluteImageRect),
        _isAlphaImage(isAlphaImage), _isValidPremultiplied(isValidPremultiplied), _colorSpace(colorSpace)
  {
  }

  FilterImage::FilterImage(const FloatRect &primitiveSubregion, const FloatRect &imageRect,
                           const IntRect &absoluteImageRect, Ref<ImageBuffer> &&imageBuffer) noexcept
      : _primitiveSubregion(primitiveSubregion), _imageRect(imageRect), _absoluteImageRect(absoluteImageRect),
        _colorSpace(DestinationColorSpace::SRGB()), _imageBuffer(krys::move(imageBuffer))
  {
  }

  FloatRect FilterImage::MaxEffectRect(const Filter &filter) const noexcept
  {
    return filter.MaxEffectRect(_primitiveSubregion);
  }

  IntRect FilterImage::AbsoluteImageRectRelativeTo(const FilterImage &origin) const noexcept
  {
    return _absoluteImageRect - origin.AbsoluteImageRect().Location();
  }

  FloatPoint FilterImage::MappedAbsolutePoint(const FloatPoint &point) const noexcept
  {
    return FloatPoint(point - _absoluteImageRect.Location());
  }

  size_t FilterImage::MemoryCost() const noexcept
  {
    // Whatever holds the pixels holds four bytes for each.
    return static_cast<size_t>(_absoluteImageRect.Width()) * static_cast<size_t>(_absoluteImageRect.Height())
           * 4u;
  }
}
