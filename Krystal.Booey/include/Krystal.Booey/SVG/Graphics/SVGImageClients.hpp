#pragma once

#include "Krystal.Booey/Graphics/ImageObserver.hpp"
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
