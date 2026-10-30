#pragma once

#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/Graphics/ImageObserver.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResource.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::svg
{
  class SVGImageCache;
}

namespace krys::boo
{
  class CachedImageClient;
  class FloatSize;
  class IntRect;
  class RenderElement;
  class RenderObject;

  /// @brief A cached image: the encoded data a load brought, and the image made from it.
  // FIXME: The broken image (G7c), the image data update back-off for streamed data, and asynchronous
  // decoding's waiting clients.
  class CachedImage final : public CachedResource
  {
    friend class MemoryCache;

  private:
    /// @brief Observes the image for the cached images that share it.
    class CachedImageObserver final : public ImageObserver
    {
    private:
      HashSet<CachedImage *> _cachedImages;

    public:
      KRYS_NODISCARD static Ref<CachedImageObserver> Create(CachedImage &image) noexcept
      {
        return AdoptRef(*new CachedImageObserver(image));
      }

      KRYS_NODISCARD HashSet<CachedImage *> &CachedImages() noexcept
      {
        return _cachedImages;
      }
      KRYS_NODISCARD const HashSet<CachedImage *> &CachedImages() const noexcept
      {
        return _cachedImages;
      }

    private:
      explicit CachedImageObserver(CachedImage &) noexcept;

      // ImageObserver API
      KRYS_NODISCARD url::URL SourceUrl() const noexcept override;
      KRYS_NODISCARD utf8_string MimeType() const noexcept override;
      KRYS_NODISCARD uint32 NumberOfClients() const noexcept override;
      KRYS_NODISCARD int64 ExpectedContentLength() const noexcept override;

      void EncodedDataStatusChanged(const Image &, EncodedDataStatus) noexcept final;
      void DecodedSizeChanged(const Image &, int64 delta) noexcept final;
      void DidDraw(const Image &) noexcept final;

      KRYS_NODISCARD bool CanDestroyDecodedData(const Image &) const noexcept final;
      void ImageFrameAvailable(const Image &, ImageAnimatingState, const IntRect *changeRect = nullptr,
                               DecodingStatus = DecodingStatus::Invalid) noexcept final;
      void ChangedInRect(const Image &, const IntRect *) noexcept final;
      void ImageContentChanged(const Image &) noexcept final;
      void ScheduleRenderingUpdate(const Image &) noexcept final;

      KRYS_NODISCARD bool AllowsAnimation(const Image &) const noexcept final;
    };

    struct ContainerContext
    {
      LayoutSize ContainerSize;
      float ContainerZoom;
      url::URL ImageURL;
    };

    RefPtr<CachedImageObserver> _imageObserver;
    RefPtr<Image> _image;
    UniquePtr<svg::SVGImageCache> _svgImageCache;
    HashMap<const CachedImageClient *, ContainerContext> _pendingContainerContextRequests;
    bool _shouldPaintBrokenImage {true};
    bool _allowsOrientationOverride {true};

  public:
    explicit CachedImage(CachedResourceRequest &&) noexcept;
    ~CachedImage() noexcept override;

    KRYS_NODISCARD Image *
      GetImage() const noexcept; // Returns the nullImage() if the image is not available yet.
    KRYS_NODISCARD Image *ImageForRenderer(const RenderObject *) noexcept; // Returns the nullImage() if the
                                                                           // image is not available yet.
    KRYS_NODISCARD bool HasImage() const noexcept
    {
      return static_cast<bool>(_image);
    }
    KRYS_NODISCARD bool CurrentFrameKnownToBeOpaque(const RenderElement *) noexcept;
    KRYS_NODISCARD bool CurrentFrameIsComplete(const RenderElement *) noexcept;

    // Returns an image and the image's resolution scale factor.
    KRYS_NODISCARD Pair<Image *, float> BrokenImage(float deviceScaleFactor) const noexcept;
    KRYS_NODISCARD bool WillPaintBrokenImage() const noexcept;

    KRYS_NODISCARD bool CanRender(const RenderElement *renderer, float multiplier) noexcept
    {
      return !ErrorOccurred() && !ImageSizeForRenderer(renderer, multiplier).IsEmpty();
    }

    void SetAllowsOrientationOverride(bool b) noexcept
    {
      _allowsOrientationOverride = b;
    }
    KRYS_NODISCARD bool AllowsOrientationOverride() const noexcept
    {
      return _allowsOrientationOverride;
    }

    void SetContainerContextForClient(const CachedImageClient &, const LayoutSize &, float,
                                      const url::URL &) noexcept;
    KRYS_NODISCARD bool UsesImageContainerSize() const noexcept
    {
      return _image && _image->UsesContainerSize();
    }
    KRYS_NODISCARD bool ImageHasRelativeWidth() const noexcept
    {
      return _image && _image->HasRelativeWidth();
    }
    KRYS_NODISCARD bool ImageHasRelativeHeight() const noexcept
    {
      return _image && _image->HasRelativeHeight();
    }

    void UpdateBuffer(const SharedBuffer &) noexcept override;
    void FinishLoading(const SharedBuffer *) noexcept override;

    enum SizeType : uint8
    {
      UsedSize,
      IntrinsicSize
    };
    KRYS_NODISCARD FloatSize ImageSizeForRenderer(const RenderElement *renderer,
                                                  SizeType = UsedSize) const noexcept;
    // This method takes a zoom multiplier that can be used to increase the natural size of the image by the
    // zoom.
    KRYS_NODISCARD LayoutSize ImageSizeForRenderer(const RenderElement *, float multiplier,
                                                   SizeType = UsedSize) const noexcept; // returns the size of
                                                                                        // the complete image.
    KRYS_NODISCARD LayoutSize UnclampedImageSizeForRenderer(const RenderElement *renderer, float multiplier,
                                                            SizeType = UsedSize) const noexcept;
    void ComputeIntrinsicDimensions(float &intrinsicWidth, float &intrinsicHeight,
                                    FloatSize &intrinsicRatio) noexcept;

    void Load(CachedResourceLoader &) noexcept override;

    KRYS_NODISCARD bool StillNeedsLoad() const noexcept override
    {
      return !ErrorOccurred() && GetStatus() == Unknown && !IsLoading();
    }

    KRYS_NODISCARD bool AllowsAnimation(const Image &) const noexcept;

  private:
    void Clear() noexcept;

    void CreateImage() noexcept;
    void ClearImage() noexcept;
    // If not null, changeRect is the changed part of the image.
    void NotifyObservers(const IntRect *changeRect = nullptr) noexcept;

    void DidAddClient(CachedResourceClient &) noexcept final;
    void DidRemoveClient(CachedResourceClient &) noexcept final;

    void AllClientsRemoved() noexcept override;
    void DestroyDecodedData() noexcept override;

    EncodedDataStatus UpdateImageData(bool allDataReceived) noexcept;
    void Error(CachedResource::Status) noexcept override;
    void ResponseReceived(ResourceResponse &&) noexcept override;

    void EncodedDataStatusChanged(const Image &, EncodedDataStatus) noexcept;
    void DecodedSizeChanged(const Image &, int64 delta) noexcept;
    void DidDraw(const Image &) noexcept;
    KRYS_NODISCARD bool CanDestroyDecodedData(const Image &) const noexcept;
    void ImageFrameAvailable(const Image &, ImageAnimatingState, const IntRect *changeRect = nullptr,
                             DecodingStatus = DecodingStatus::Invalid) noexcept;
    void ChangedInRect(const Image &, const IntRect *) noexcept;
    void ImageContentChanged(const Image &) noexcept;
    void ScheduleRenderingUpdate(const Image &) noexcept;

    void UpdateBufferInternal(const SharedBuffer &) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_CACHED_RESOURCE(CachedImage, CachedResource::Type::ImageResource)
