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

#include "Krystal.Booey/Graphics/Images/GeneratedImage.hpp"

namespace krys::boo
{
  /// @brief A `cross-fade()` image drawn: two images, the first fading out as the second fades in.
  class CrossfadeGeneratedImage final : public GeneratedImage
  {
  private:
    const Ref<Image> _fromImage;
    const Ref<Image> _toImage;
    float _percentage;
    FloatSize _crossfadeSize;

  public:
    /// @brief @p toImage at @p percentage over @p fromImage, both scaled to @p crossfadeSize, drawn into a
    /// container of @p size.
    KRYS_NODISCARD static Ref<CrossfadeGeneratedImage> Create(Image &fromImage, Image &toImage,
                                                              float percentage,
                                                              const FloatSize &crossfadeSize,
                                                              const FloatSize &size) noexcept
    {
      return AdoptRef(*new CrossfadeGeneratedImage(fromImage, toImage, percentage, crossfadeSize, size));
    }

    void SetContainerSize(const FloatSize &) noexcept override
    {
    }
    KRYS_NODISCARD bool UsesContainerSize() const noexcept override
    {
      return false;
    }
    KRYS_NODISCARD bool HasRelativeWidth() const noexcept override
    {
      return false;
    }
    KRYS_NODISCARD bool HasRelativeHeight() const noexcept override
    {
      return false;
    }

    KRYS_NODISCARD FloatSize
      Size(ImageOrientation = ImageOrientation::Orientation::FromImage) const noexcept override
    {
      return _crossfadeSize;
    }

  private:
    CrossfadeGeneratedImage(Image &fromImage, Image &toImage, float percentage,
                            const FloatSize &crossfadeSize, const FloatSize &) noexcept;

    ImageDrawResult Draw(GraphicsContext &, const FloatRect &dstRect, const FloatRect &srcRect,
                         ImagePaintingOptions = {}) noexcept override;
    void DrawPattern(GraphicsContext &, const FloatRect &dstRect, const FloatRect &srcRect,
                     const AffineTransform &patternTransform, const FloatPoint &phase,
                     const FloatSize &spacing, ImagePaintingOptions = {}) noexcept override;

    void DrawCrossfade(GraphicsContext &) noexcept;
  };
}
