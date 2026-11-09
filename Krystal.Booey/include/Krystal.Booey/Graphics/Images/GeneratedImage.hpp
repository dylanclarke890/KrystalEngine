/*
 * Copyright (C) 2008, 2013 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Images/Image.hpp"

namespace krys::boo
{
  /// @brief An image generated at its container's size, such as a gradient.
  class GeneratedImage : public Image
  {
  private:
    FloatSize _size;

  public:
    void SetContainerSize(const FloatSize &size) noexcept override
    {
      _size = size;
    }
    KRYS_NODISCARD bool UsesContainerSize() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD bool HasRelativeWidth() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD bool HasRelativeHeight() const noexcept override
    {
      return true;
    }

    void ComputeIntrinsicDimensions(float &intrinsicWidth, float &intrinsicHeight,
                                    FloatSize &intrinsicRatio) noexcept override;

    KRYS_NODISCARD FloatSize
      Size(ImageOrientation = ImageOrientation::Orientation::FromImage) const noexcept override
    {
      return _size;
    }

  protected:
    GeneratedImage() noexcept = default;

    ImageDrawResult Draw(GraphicsContext &, const FloatRect &dstRect, const FloatRect &srcRect,
                         ImagePaintingOptions = {}) noexcept override = 0;
    void DrawPattern(GraphicsContext &, const FloatRect &destRect, const FloatRect &srcRect,
                     const AffineTransform &patternTransform, const FloatPoint &phase,
                     const FloatSize &spacing, ImagePaintingOptions = {}) noexcept override = 0;

    // FIXME: Implement this to be less conservative.
    KRYS_NODISCARD bool CurrentFrameKnownToBeOpaque() const noexcept override
    {
      return false;
    }
    KRYS_NODISCARD bool CurrentFrameIsComplete() const noexcept override
    {
      return true;
    }

  private:
    KRYS_NODISCARD bool IsGeneratedImage() const noexcept override
    {
      return true;
    }
  };
}
