/*
 * Copyright (C) 2015-2024 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Images/ImageObserver.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/SVG/Graphics/SVGImage.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::svg
{
  /// @brief The chrome client of an SVG image's page: the page's repaints and rendering updates go to the
  /// image's observer.
  class SVGImageChromeClient final : public ChromeClient
  {
    KRYS_NON_COPY_MOVABLE(SVGImageChromeClient);

  private:
    WeakPtr<SVGImage> _image;

  public:
    explicit SVGImageChromeClient(SVGImage *image) noexcept : _image(CreateWeakPtr(image))
    {
    }

    KRYS_NODISCARD bool IsSVGImageChromeClient() const noexcept final
    {
      return true;
    }
    KRYS_NODISCARD SVGImage *Image() const noexcept
    {
      return _image.get();
    }

  private:
    /// @brief The image, unless it's gone or being destroyed (its internal page is cleared first).
    KRYS_NODISCARD RefPtr<SVGImage> ProtectedImage() const noexcept
    {
      auto *image = _image.get();
      if (!image || !image->InternalPage())
      {
        return nullptr;
      }
      return ShareRefPtr(image);
    }

    void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept final
    {
    }

    void DidFinishLoadingImageForSVGImage(SVGImageElement &) noexcept final
    {
      RefPtr<SVGImage> image = ProtectedImage();
      if (!image)
      {
        return;
      }

      if (RefPtr<ImageObserver> imageObserver = image->GetImageObserver())
      {
        imageObserver->ImageContentChanged(*image);
      }
    }

    void InvalidateContentsAndRootView(const IntRect &rect) noexcept final
    {
      RefPtr<SVGImage> image = ProtectedImage();
      if (!image)
      {
        return;
      }

      RefPtr<ImageObserver> imageObserver = image->GetImageObserver();
      if (!imageObserver)
      {
        return;
      }

      imageObserver->ImageFrameAvailable(
        *image, image->IsAnimating() ? ImageAnimatingState::Yes : ImageAnimatingState::No, &rect);
    }

    bool ScheduleRenderingUpdate() noexcept final
    {
      RefPtr<SVGImage> image = ProtectedImage();
      if (!image)
      {
        return true;
      }
      if (RefPtr<ImageObserver> imageObserver = image->GetImageObserver())
      {
        imageObserver->ScheduleRenderingUpdate(*image);
      }
      return true;
    }
  };
}
