/*
 * Copyright (C) 2011-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Web/Graphics/Filters/Filter.hpp"
#include "Krystal.Web/Graphics/Images/GeneratedImage.hpp"

namespace krys::web
{
  /// @brief A `filter()` image drawn: its input image drawn through the filter as it's painted, rather than
  /// filtered into a bitmap up front, so it works wherever the destination can apply the filter.
  class FilterGeneratedImage final : public GeneratedImage
  {
  private:
    const Ref<Image> _image;
    const Ref<Filter> _filter;

  public:
    /// @brief @p image filtered by @p filter, whose filter region is the image's rect at @p size.
    KRYS_NODISCARD static Ref<FilterGeneratedImage> Create(Image &image, Filter &filter,
                                                           const SizeF &size) noexcept
    {
      return AdoptRef(*new FilterGeneratedImage(image, filter, size));
    }

  private:
    FilterGeneratedImage(Image &, Filter &, const SizeF &) noexcept;

    ImageDrawResult Draw(GraphicsContext &, const RectF &dstRect, const RectF &srcRect,
                         ImagePaintingOptions = {}) noexcept override;

    void DrawPattern(GraphicsContext &, const RectF &dstRect, const RectF &srcRect,
                     const AffineTransform &patternTransform, const PointF &phase, const SizeF &spacing,
                     ImagePaintingOptions = {}) noexcept override;

    void DrawFiltered(GraphicsContext &) noexcept;
  };
}
