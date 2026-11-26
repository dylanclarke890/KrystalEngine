/*
 * Copyright (C) 2008-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Web/Graphics/Gradient.hpp"
#include "Krystal.Web/Graphics/ImageBuffers/ImageBuffer.hpp"
#include "Krystal.Web/Graphics/Images/GeneratedImage.hpp"

namespace krys::web
{
  /// @brief A gradient drawn as an image of its container's size.
  class GradientImage final : public GeneratedImage
  {
  private:
    const Ref<web::Gradient> _gradient;
    RefPtr<ImageBuffer> _cachedImage;
    SizeF _cachedAdjustedSize;
    uint32 _cachedGeneratorHash {0};
    SizeF _cachedScaleFactor;

  public:
    KRYS_NODISCARD static Ref<GradientImage> Create(web::Gradient &generator, const SizeF &size) noexcept
    {
      return AdoptRef(*new GradientImage(generator, size));
    }

    ~GradientImage() noexcept override;

    KRYS_NODISCARD const web::Gradient &GetGradient() const noexcept
    {
      return *_gradient;
    }

  private:
    GradientImage(web::Gradient &, const SizeF &) noexcept;

    ImageDrawResult Draw(GraphicsContext &, const RectF &destRect, const RectF &srcRect,
                         ImagePaintingOptions = {}) noexcept override;

    void DrawPattern(GraphicsContext &, const RectF &destRect, const RectF &srcRect,
                     const AffineTransform &patternTransform, const PointF &phase, const SizeF &spacing,
                     ImagePaintingOptions = {}) noexcept override;

    KRYS_NODISCARD bool IsGradientImage() const noexcept override
    {
      return true;
    }
  };
}
