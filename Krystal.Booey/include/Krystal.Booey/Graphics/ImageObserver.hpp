#pragma once

#include "Krystal.Booey/Graphics/ImageTypes.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  class Image;
  class IntRect;

  /// @brief Interface for notification about changes to an image, including decoding, drawing, and animating.
  // FIXME: The settings and dark appearance queries, once there are settings.
  class ImageObserver : public RefCounted<ImageObserver>, public CanMakeWeakPtr<ImageObserver>
  {
  public:
    virtual ~ImageObserver() noexcept = default;

    KRYS_NODISCARD virtual url::URL SourceUrl() const noexcept = 0;
    KRYS_NODISCARD virtual utf8_string MimeType() const noexcept = 0;
    KRYS_NODISCARD virtual uint32 NumberOfClients() const noexcept
    {
      return 0;
    }
    KRYS_NODISCARD virtual int64 ExpectedContentLength() const noexcept = 0;

    virtual void EncodedDataStatusChanged(const Image &, EncodedDataStatus) noexcept
    {
    }
    virtual void DecodedSizeChanged(const Image &, int64 delta) noexcept = 0;

    virtual void DidDraw(const Image &) noexcept = 0;
    KRYS_NODISCARD virtual bool CanDestroyDecodedData(const Image &) const noexcept
    {
      return true;
    }

    virtual void ImageFrameAvailable(const Image &, ImageAnimatingState, const IntRect *changeRect = nullptr,
                                     DecodingStatus = DecodingStatus::Invalid) noexcept = 0;
    virtual void ChangedInRect(const Image &, const IntRect *changeRect = nullptr) noexcept = 0;
    virtual void ImageContentChanged(const Image &) noexcept = 0;
    virtual void ScheduleRenderingUpdate(const Image &) noexcept = 0;

    KRYS_NODISCARD virtual bool AllowsAnimation(const Image &) const noexcept
    {
      return true;
    }

  protected:
    ImageObserver() noexcept = default;
  };
}
