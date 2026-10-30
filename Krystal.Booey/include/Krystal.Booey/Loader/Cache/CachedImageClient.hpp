#pragma once

#include "Krystal.Booey/Graphics/ImageTypes.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceClient.hpp"

namespace krys::boo
{
  class CachedImage;
  class IntRect;

  namespace dom
  {
    class Document;
  }

  enum class VisibleInViewportState : uint8
  {
    Unknown,
    Yes,
    No
  };

  /// @brief Something told when a cached image's data or frames change.
  class CachedImageClient : public CachedResourceClient
  {
  public:
    ~CachedImageClient() noexcept override = default;

    KRYS_NODISCARD static CachedResourceClientType ExpectedType() noexcept
    {
      return ImageType;
    }
    KRYS_NODISCARD CachedResourceClientType ResourceClientType() const noexcept override
    {
      return ExpectedType();
    }

    // Called whenever a frame of an image changes because we got more data from the network.
    // If not null, the IntRect is the changed rect of the image.
    virtual void ImageChanged(CachedImage *, const IntRect * = nullptr) noexcept
    {
    }

    KRYS_NODISCARD virtual bool CanDestroyDecodedData() const noexcept
    {
      return true;
    }

    // Called when a new decoded frame for a large image is available or when an animated image is ready to
    // advance to the next frame.
    virtual VisibleInViewportState ImageFrameAvailable(CachedImage &, ImageAnimatingState,
                                                       const IntRect *) noexcept;
    KRYS_NODISCARD virtual VisibleInViewportState ImageVisibleInViewport(const dom::Document &) const noexcept
    {
      return VisibleInViewportState::No;
    }

    virtual void DidRemoveCachedImageClient(CachedImage &) noexcept
    {
    }

    virtual void ImageContentChanged(CachedImage &) noexcept
    {
    }
    virtual void ScheduleRenderingUpdateForImage(CachedImage &) noexcept
    {
    }

    KRYS_NODISCARD virtual bool AllowsAnimation() const noexcept
    {
      return true;
    }

  protected:
    CachedImageClient() noexcept = default;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_CACHED_RESOURCE_CLIENT(CachedImageClient, ImageType)
