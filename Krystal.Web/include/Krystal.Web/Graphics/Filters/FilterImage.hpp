/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Graphics/Color/DestinationColorSpace.hpp"
#include "Krystal.Web/Graphics/ImageBuffers/ImageBuffer.hpp"

namespace krys::web
{
  class Filter;

  /// @brief An input or result of a filter effect: where it is, and what holds its pixels.
  /// @note A filter's source is an image buffer. Effects' results are held by the accelerated backend that
  /// applied them, as a handle of its own; the backend converts an image between colour spaces as it reads
  /// it.
  // FIXME: Pixel buffers (pixelBuffer, getPixelBuffer, copyPixelBuffer, correctPremultipliedPixelBuffer) and
  // results in image buffers, once filters can be applied in software.
  class FilterImage : public RefCounted<FilterImage>
  {
  private:
    RectF _primitiveSubregion;
    RectF _imageRect;
    RectI _absoluteImageRect;
    bool _isAlphaImage {false};
    bool _isValidPremultiplied {true};
    DestinationColorSpace _colorSpace;
    RefPtr<ImageBuffer> _imageBuffer;
    Maybe<uint32> _acceleratedImage;

  public:
    KRYS_NODISCARD static Ref<FilterImage> Create(const RectF &primitiveSubregion, const RectF &imageRect,
                                                  const RectI &absoluteImageRect, bool isAlphaImage,
                                                  bool isValidPremultiplied,
                                                  const DestinationColorSpace &colorSpace) noexcept;

    KRYS_NODISCARD static Ref<FilterImage> Create(const RectF &primitiveSubregion, const RectF &imageRect,
                                                  const RectI &absoluteImageRect,
                                                  Ref<ImageBuffer> &&imageBuffer) noexcept;

    // The return values are in filter coordinates.
    KRYS_NODISCARD RectF PrimitiveSubregion() const noexcept
    {
      return _primitiveSubregion;
    }

    KRYS_NODISCARD RectF MaxEffectRect(const Filter &filter) const noexcept;

    KRYS_NODISCARD RectF ImageRect() const noexcept
    {
      return _imageRect;
    }

    // The return values are in user-space coordinates.
    KRYS_NODISCARD RectI AbsoluteImageRect() const noexcept
    {
      return _absoluteImageRect;
    }

    KRYS_NODISCARD RectI AbsoluteImageRectRelativeTo(const FilterImage &origin) const noexcept;

    KRYS_NODISCARD PointF MappedAbsolutePoint(const PointF &point) const noexcept;

    KRYS_NODISCARD bool IsAlphaImage() const noexcept
    {
      return _isAlphaImage;
    }

    KRYS_NODISCARD bool IsValidPremultiplied() const noexcept
    {
      return _isValidPremultiplied;
    }

    KRYS_NODISCARD const DestinationColorSpace &ColorSpace() const noexcept
    {
      return _colorSpace;
    }

    KRYS_NODISCARD size_t MemoryCost() const noexcept;

    /// @brief The image buffer holding the image's pixels: a filter's source.
    KRYS_NODISCARD ImageBuffer *GetImageBuffer() const noexcept
    {
      return _imageBuffer.get();
    }

    /// @brief The accelerated backend's handle to the image's pixels, once it has applied the effect whose
    /// result this is (or read the source).
    KRYS_NODISCARD Maybe<uint32> AcceleratedImage() const noexcept
    {
      return _acceleratedImage;
    }

    void SetAcceleratedImage(uint32 image) noexcept
    {
      _acceleratedImage = image;
    }

    /// @brief Nothing: the accelerated backend converts the image to the colour space each effect operates
    /// in as it reads it.
    void TransformToColorSpace(const DestinationColorSpace &) noexcept
    {
    }

  private:
    FilterImage(const RectF &primitiveSubregion, const RectF &imageRect, const RectI &absoluteImageRect,
                bool isAlphaImage, bool isValidPremultiplied,
                const DestinationColorSpace &colorSpace) noexcept;

    FilterImage(const RectF &primitiveSubregion, const RectF &imageRect, const RectI &absoluteImageRect,
                Ref<ImageBuffer> &&imageBuffer) noexcept;
  };

  using FilterImageVector = List<Ref<FilterImage>>;
}
