/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Images/Image.hpp"
#include "Krystal.Booey/SVG/Graphics/SVGImage.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::svg
{
  /// @brief An SVG image as one client draws it: at that client's container size and zoom, scrolled to the
  /// fragment of the client's URL.
  class SVGImageForContainer final : public Image
  {
  private:
    WeakPtr<SVGImage> _image;
    const FloatSize _containerSize;
    const float _containerZoom;
    const url::URL _initialFragmentURL;

  public:
    KRYS_NODISCARD static Ref<SVGImageForContainer> Create(SVGImage *image, const FloatSize &containerSize,
                                                           float containerZoom,
                                                           const url::URL &initialFragmentURL) noexcept
    {
      return AdoptRef(*new SVGImageForContainer(image, containerSize, containerZoom, initialFragmentURL));
    }

    KRYS_NODISCARD bool IsSVGImageForContainer() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD
    FloatSize Size(ImageOrientation = ImageOrientation::Orientation::FromImage) const noexcept final;

    KRYS_NODISCARD bool UsesContainerSize() const noexcept final
    {
      return _image->UsesContainerSize();
    }
    KRYS_NODISCARD bool HasRelativeWidth() const noexcept final
    {
      return _image->HasRelativeWidth();
    }
    KRYS_NODISCARD bool HasRelativeHeight() const noexcept final
    {
      return _image->HasRelativeHeight();
    }
    void ComputeIntrinsicDimensions(float &intrinsicWidth, float &intrinsicHeight,
                                    FloatSize &intrinsicRatio) noexcept final
    {
      RefPtr<SVGImage> image = ShareRefPtr(_image.get());
      image->ComputeIntrinsicDimensions(intrinsicWidth, intrinsicHeight, intrinsicRatio);
    }

    ImageDrawResult Draw(GraphicsContext &, const FloatRect &, const FloatRect &,
                         ImagePaintingOptions = {}) noexcept final;

    void DrawPattern(GraphicsContext &, const FloatRect &, const FloatRect &, const AffineTransform &,
                     const FloatPoint &, const FloatSize &, ImagePaintingOptions = {}) noexcept final;

    // FIXME: Implement this to be less conservative.
    KRYS_NODISCARD bool CurrentFrameKnownToBeOpaque() const noexcept final
    {
      return false;
    }
    KRYS_NODISCARD bool CurrentFrameIsComplete() const noexcept final
    {
      return static_cast<bool>(_image);
    }

    KRYS_NODISCARD RefPtr<NativeImage> CurrentNativeImage() noexcept final;

  private:
    SVGImageForContainer(SVGImage *, const FloatSize &containerSize, float containerZoom,
                         const url::URL &initialFragmentURL) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGImageForContainer)
  KRYS_NODISCARD static bool IsType(const krys::boo::Image &image) noexcept
  {
    return image.IsSVGImageForContainer();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
