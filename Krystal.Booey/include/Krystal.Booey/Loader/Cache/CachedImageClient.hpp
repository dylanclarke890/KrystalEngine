#pragma once

#include "Krystal.Booey/Graphics/Images/ImageTypes.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceClient.hpp"
#include "Krystal.Core/Geometry/GeometryForward.hpp"

namespace krys::boo
{
  class CachedImage;
  class Document;

  enum class VisibleInViewportState : uint8
  {
    Unknown,
    Yes,
    No,
  };

  /// @brief Something told when a cached image's data or frames change.
  class CachedImageClient : public CachedResourceClient
  {
  public:
    ~CachedImageClient() noexcept override = default;

    /// @brief Its client kind.
    KRYS_NODISCARD static CachedResourceClientType ExpectedType() noexcept
    {
      return ImageType;
    }

    KRYS_NODISCARD CachedResourceClientType ResourceClientType() const noexcept override
    {
      return ExpectedType();
    }

    /// @brief Called when the image changes (more data, a failure); the rectangle, if given, is the part that
    /// did.
    virtual void ImageChanged(CachedImage *, const RectI * = nullptr) noexcept
    {
    }

    /// @brief Whether the image's decoded data may be dropped while the client uses it.
    KRYS_NODISCARD virtual bool CanDestroyDecodedData() const noexcept
    {
      return true;
    }

    /// @brief Called when a frame decoded off the main thread is ready, or an animation is ready for its next
    /// frame.
    /// @return Whether the client shows the image in the viewport: an animation nobody sees stops.
    virtual VisibleInViewportState ImageFrameAvailable(CachedImage &, ImageAnimatingState,
                                                       const RectI *) noexcept;

    /// @brief Called after the client is removed from the image.
    virtual void DidRemoveCachedImageClient(CachedImage &) noexcept
    {
    }

    /// @brief Called when the image's content changes without new data (an SVG image's document).
    virtual void ImageContentChanged(CachedImage &) noexcept
    {
    }

    /// @brief Called when the image needs a rendering update (to advance an animation).
    virtual void ScheduleRenderingUpdateForImage(CachedImage &) noexcept
    {
    }

    /// @brief Whether the image may animate for the client.
    KRYS_NODISCARD virtual bool AllowsAnimation() const noexcept
    {
      return true;
    }

    /// @brief Whether the client is drawn with the dark appearance.
    KRYS_NODISCARD virtual bool UseSystemDarkAppearance() const noexcept
    {
      return false;
    }

  protected:
    CachedImageClient() noexcept = default;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_CACHED_RESOURCE_CLIENT(CachedImageClient, ImageType)
