/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Images/GeneratedImage.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Url.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class LegacyRenderSVGResourceContainer;
}

namespace krys::boo::svg
{
  /// @brief An image drawn by an SVG resource a CSS image refers to (a `mask-image: url()` to a `<mask>`).
  class SVGResourceImage final : public GeneratedImage
  {
  private:
    WeakPtr<LegacyRenderSVGResourceContainer> _legacyRenderResource;
    style::URL _resourceURL;

  public:
    KRYS_NODISCARD static Ref<SVGResourceImage> Create(LegacyRenderSVGResourceContainer &,
                                                       const style::URL &) noexcept;
    ~SVGResourceImage() noexcept override;

  private:
    SVGResourceImage(LegacyRenderSVGResourceContainer &, const style::URL &) noexcept;

    ImageDrawResult Draw(GraphicsContext &, const FloatRect &destinationRect, const FloatRect &sourceRect,
                         ImagePaintingOptions = {}) noexcept final;
    void DrawPattern(GraphicsContext &, const FloatRect &destRect, const FloatRect &srcRect,
                     const AffineTransform &patternTransform, const FloatPoint &phase,
                     const FloatSize &spacing, ImagePaintingOptions = {}) noexcept final;

    KRYS_NODISCARD bool IsSVGResourceImage() const noexcept final
    {
      return true;
    }

    void Dump(TextStream &) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGResourceImage)
  KRYS_NODISCARD static bool IsType(const krys::boo::Image &image) noexcept
  {
    return image.IsSVGResourceImage();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
