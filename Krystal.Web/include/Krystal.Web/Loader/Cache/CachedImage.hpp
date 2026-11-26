#pragma once

#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Containers/HashSet.hpp"
#include "Krystal.Core/Geometry/GeometryForward.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Web/Graphics/Geometry/LayoutSize.hpp"
#include "Krystal.Web/Graphics/Images/Image.hpp"
#include "Krystal.Web/Graphics/Images/ImageObserver.hpp"
#include "Krystal.Web/Loader/Cache/CachedResource.hpp"

namespace krys::web
{
  class CachedImageClient;
  class RenderElement;
  class RenderObject;
  class SVGImageCache;

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
      /// @brief An observer for @p image's image.
      KRYS_NODISCARD static Ref<CachedImageObserver> Create(CachedImage &image) noexcept
      {
        return AdoptRef(*new CachedImageObserver(image));
      }

      /// @brief The cached images sharing the image.
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

      // The image's observer calls: each goes to the cached images sharing the image, and the queries are
      // answered by the first.
      KRYS_NODISCARD net::Url SourceUrl() const noexcept override;

      KRYS_NODISCARD utf8_string MimeType() const noexcept override;

      KRYS_NODISCARD uint32 NumberOfClients() const noexcept override;

      KRYS_NODISCARD int64 ExpectedContentLength() const noexcept override;

      void EncodedDataStatusChanged(const Image &, img::EncodedDataStatus) noexcept final;

      void DecodedSizeChanged(const Image &, int64 delta) noexcept final;

      void DidDraw(const Image &) noexcept final;

      KRYS_NODISCARD bool CanDestroyDecodedData(const Image &) const noexcept final;

      void ImageFrameAvailable(const Image &, ImageAnimatingState, const RectI *changeRect = nullptr,
                               img::DecodingStatus = img::DecodingStatus::Invalid) noexcept final;

      void ChangedInRect(const Image &, const RectI *) noexcept final;

      void ImageContentChanged(const Image &) noexcept final;

      void ScheduleRenderingUpdate(const Image &) noexcept final;

      KRYS_NODISCARD bool AllowsAnimation(const Image &) const noexcept final;

      KRYS_NODISCARD Settings *GetSettings() const noexcept final;

      KRYS_NODISCARD bool UseSystemDarkAppearance() const noexcept final;
    };

    struct ContainerContext
    {
      LayoutSize ContainerSize;
      float ContainerZoom;
      net::Url ImageUrl;
    };

    RefPtr<CachedImageObserver> _imageObserver;
    /// @brief The settings of the document that loaded the image.
    RefPtr<Settings> _settings;
    RefPtr<Image> _image;
    UniquePtr<SVGImageCache> _svgImageCache;
    HashMap<const CachedImageClient *, ContainerContext> _pendingContainerContextRequests;
    /// @brief The clients that drew nothing while the image's frame decodes on the work queue, which a static
    /// image's frame, once it's decoded, is shown in.
    HashSet<const CachedImageClient *> _clientsWaitingForAsyncDecoding;
    bool _shouldPaintBrokenImage {true};
    bool _allowsOrientationOverride {true};

  public:
    /// @brief An image for the request, waiting to be asked to load.
    explicit CachedImage(CachedResourceRequest &&) noexcept;

    ~CachedImage() noexcept override;

    /// @brief The image to draw: the broken image icon if the load failed, the null image until there's an
    /// image, else the image (for a renderer, an SVG image's copy sized for it).
    KRYS_NODISCARD Image *GetImage() const noexcept;

    KRYS_NODISCARD Image *ImageForRenderer(const RenderObject *) noexcept;

    /// @brief Whether an image has been made from the data.
    KRYS_NODISCARD bool HasImage() const noexcept
    {
      return static_cast<bool>(_image);
    }

    /// @brief Whether the current frame of the image drawn for the renderer is known to be opaque, and
    /// whether it's fully decoded.
    KRYS_NODISCARD bool CurrentFrameKnownToBeOpaque(const RenderElement *) noexcept;

    KRYS_NODISCARD bool CurrentFrameIsComplete(const RenderElement *) noexcept;

    /// @brief The broken image icon and its scale factor (1: it's drawn as shapes, at any scale).
    KRYS_NODISCARD static Pair<Image *, float> BrokenImage(float deviceScaleFactor) noexcept;

    /// @brief Whether the load failed and the broken image icon is drawn instead.
    KRYS_NODISCARD bool WillPaintBrokenImage() const noexcept;

    /// @brief Whether there's something to draw for the renderer: no error, and a size.
    KRYS_NODISCARD bool CanRender(const RenderElement *renderer, float multiplier) noexcept
    {
      return !ErrorOccurred() && !ImageSizeForRenderer(renderer, multiplier).IsEmpty();
    }

    /// @brief Whether `image-orientation` may override the orientation the image's data gives it: not for a
    /// load that failed (or was canceled), unless it's a data URL.
    /// @note The renderers don't consult it yet (see `RenderElement::GetImageOrientation`).
    KRYS_NODISCARD bool AllowsOrientationOverride() const noexcept
    {
      return _allowsOrientationOverride;
    }

    void SetAllowsOrientationOverride(bool allowsOrientationOverride) noexcept
    {
      _allowsOrientationOverride = allowsOrientationOverride;
    }

    /// @brief Gives an image sized by its container (an SVG image) the client's container size, zoom and URL;
    /// kept until there's an image.
    void SetContainerContextForClient(const CachedImageClient &, const LayoutSize &, float,
                                      const net::Url &) noexcept;

    /// @brief Remembers that @p client drew nothing while the frame decodes, so it's told when it's decoded.
    void AddClientWaitingForAsyncDecoding(CachedImageClient &) noexcept;

    /// @brief Whether the image takes its size from its container (an SVG image), and whether its width or
    /// height is relative to it.
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
      IntrinsicSize,
    };

    /// @brief The image's size for the renderer: as used (an SVG image's for that renderer) or natural,
    /// oriented as the renderer's style says.
    KRYS_NODISCARD SizeF ImageSizeForRenderer(const RenderElement *renderer,
                                              SizeType = UsedSize) const noexcept;

    /// @brief The same, scaled by the zoom @p multiplier (but not a relative width or height), and kept at
    /// least 1 in a dimension that isn't 0; the second form doesn't keep it.
    KRYS_NODISCARD LayoutSize ImageSizeForRenderer(const RenderElement *, float multiplier,
                                                   SizeType = UsedSize) const noexcept;

    KRYS_NODISCARD LayoutSize UnclampedImageSizeForRenderer(const RenderElement *renderer, float multiplier,
                                                            SizeType = UsedSize) const noexcept;

    /// @brief The image's natural width, height and ratio, where it has them.
    void ComputeIntrinsicDimensions(float &intrinsicWidth, float &intrinsicHeight,
                                    SizeF &intrinsicRatio) noexcept;

    void Load(CachedResourceLoader &) noexcept override;

    KRYS_NODISCARD bool StillNeedsLoad() const noexcept override
    {
      return !ErrorOccurred() && GetStatus() == Unknown && !IsLoading();
    }

    /// @brief Whether @p image (if it's this one's) may animate: every client allows it.
    KRYS_NODISCARD bool AllowsAnimation(const Image &) const noexcept;

    /// @brief Whether a client showing the image is drawn with the dark appearance.
    KRYS_NODISCARD bool UseSystemDarkAppearance() const noexcept;

  private:
    /// @brief Drops the image and its data's size, as a load fails or starts again.
    void Clear() noexcept;

    /// @brief Makes the image, if there isn't one, giving it the container sizes asked for so far.
    void CreateImage() noexcept;

    /// @brief Lets the image go, leaving its observer if this was its last user.
    void ClearImage() noexcept;

    /// @brief Tells the clients the image changed; @p changeRect, if given, is the part that did.
    void NotifyObservers(const RectI *changeRect = nullptr) noexcept;

    void DidAddClient(CachedResourceClient &) noexcept final;

    void DidRemoveClient(CachedResourceClient &) noexcept final;

    void AllClientsRemoved() noexcept override;

    void DestroyDecodedData() noexcept override;

    /// @brief Gives the image the data so far, or all of it.
    img::EncodedDataStatus UpdateImageData(bool allDataReceived) noexcept;

    void Error(CachedResource::Status) noexcept override;

    void ResponseReceived(net::ResourceResponse &&) noexcept override;

    /// @brief The image observer's calls, for this cached image: each does nothing unless the image is this
    /// one's, and tells the clients what concerns them.
    void EncodedDataStatusChanged(const Image &, img::EncodedDataStatus) noexcept;

    void DecodedSizeChanged(const Image &, int64 delta) noexcept;

    void DidDraw(const Image &) noexcept;

    KRYS_NODISCARD bool CanDestroyDecodedData(const Image &) const noexcept;

    void ImageFrameAvailable(const Image &, ImageAnimatingState, const RectI *changeRect = nullptr,
                             img::DecodingStatus = img::DecodingStatus::Invalid) noexcept;

    void ChangedInRect(const Image &, const RectI *) noexcept;

    void ImageContentChanged(const Image &) noexcept;

    void ScheduleRenderingUpdate(const Image &) noexcept;

    /// @brief Takes the data so far, failing the load if it can't be an image.
    void UpdateBufferInternal(const SharedBuffer &) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_CACHED_RESOURCE(CachedImage, CachedResource::Type::ImageResource)
