#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Graphics/BitmapImage.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImageClient.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceClientWalker.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Booey/Loader/Cache/MemoryCache.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/SVG/Graphics/SVGImage.hpp"
#include "Krystal.Booey/SVG/Graphics/SVGImageCache.hpp"

namespace krys::boo
{
  VisibleInViewportState CachedImageClient::ImageFrameAvailable(CachedImage &image, ImageAnimatingState,
                                                                const IntRect *changeRect) noexcept
  {
    ImageChanged(&image, changeRect);
    return VisibleInViewportState::No;
  }

  CachedImage::CachedImage(CachedResourceRequest &&request) noexcept
      : CachedResource(krys::move(request), Type::ImageResource)
  {
    SetStatus(Unknown);
  }

  CachedImage::~CachedImage() noexcept
  {
    ClearImage();
  }

  void CachedImage::Load(CachedResourceLoader &loader) noexcept
  {
    if (loader.ShouldPerformImageLoad(URL()))
    {
      CachedResource::Load(loader);
    }
    else
    {
      SetLoading(false);
    }
  }

  void CachedImage::DidAddClient(CachedResourceClient &client) noexcept
  {
    if (_data && !_image && !ErrorOccurred())
    {
      CreateImage();
      (void)_image->SetData(RefPtr<SharedBuffer>(_data), true);
    }

    krys_debug_assert(client.ResourceClientType() == CachedImageClient::ExpectedType());
    if (_image && !_image->IsNull())
    {
      static_cast<CachedImageClient &>(client).ImageChanged(this);
    }

    // FIXME: Start the image's animation asynchronously (G7d).

    CachedResource::DidAddClient(client);
  }

  void CachedImage::DidRemoveClient(CachedResourceClient &client) noexcept
  {
    krys_debug_assert(client.ResourceClientType() == CachedImageClient::ExpectedType());

    _pendingContainerContextRequests.erase(&static_cast<CachedImageClient &>(client));

    if (_svgImageCache)
    {
      _svgImageCache->RemoveClientFromCache(&static_cast<CachedImageClient &>(client));
    }

    CachedResource::DidRemoveClient(client);

    static_cast<CachedImageClient &>(client).DidRemoveCachedImageClient(*this);
  }

  void CachedImage::AllClientsRemoved() noexcept
  {
    if (_image && !ErrorOccurred())
    {
      _image->ResetAnimation();
    }
  }

  Pair<Image *, float> CachedImage::BrokenImage(float) const noexcept
  {
    // FIXME: The broken image icon (`missingImage`), at the device scale factor (G7c).
    return {&Image::NullImage(), 1.f};
  }

  bool CachedImage::WillPaintBrokenImage() const noexcept
  {
    return ErrorOccurred() && _shouldPaintBrokenImage;
  }

  Image *CachedImage::GetImage() const noexcept
  {
    if (ErrorOccurred() && _shouldPaintBrokenImage)
    {
      // Returning the 1x broken image is non-ideal, but we cannot reliably access the appropriate
      // deviceScaleFactor from here. It is critical that callers use CachedImage::brokenImage()
      // when they need the real, deviceScaleFactor-appropriate broken image icon.
      return BrokenImage(1).first;
    }

    if (_image)
    {
      return _image.get();
    }

    return &Image::NullImage();
  }

  Image *CachedImage::ImageForRenderer(const RenderObject *renderer) noexcept
  {
    if (ErrorOccurred() && _shouldPaintBrokenImage)
    {
      // Returning the 1x broken image is non-ideal, but we cannot reliably access the appropriate
      // deviceScaleFactor from here. It is critical that callers use CachedImage::brokenImage()
      // when they need the real, deviceScaleFactor-appropriate broken image icon.
      return BrokenImage(1).first;
    }

    if (!_image)
    {
      return &Image::NullImage();
    }

    if (_image->DrawsSVGImage())
    {
      if (auto *image = _svgImageCache->ImageForRenderer(renderer); image != &Image::NullImage())
      {
        return image;
      }
    }
    return _image.get();
  }

  void CachedImage::SetContainerContextForClient(const CachedImageClient &client,
                                                 const LayoutSize &containerSize, float containerZoom,
                                                 const url::URL &imageURL) noexcept
  {
    if (containerSize.IsEmpty())
    {
      return;
    }
    krys_debug_assert(containerZoom);
    RefPtr<Image> image = _image;
    if (!image)
    {
      _pendingContainerContextRequests.insert_or_assign(
        &client, ContainerContext {containerSize, containerZoom, imageURL});
      return;
    }

    if (!image->DrawsSVGImage())
    {
      image->SetContainerSize(FloatSize(containerSize));
      return;
    }

    _svgImageCache->SetContainerContextForClient(client, containerSize, containerZoom, imageURL);
  }

  FloatSize CachedImage::ImageSizeForRenderer(const RenderElement *renderer, SizeType sizeType) const noexcept
  {
    if (!_image)
    {
      return {};
    }

    if (_image->DrawsSVGImage() && sizeType == UsedSize)
    {
      return _svgImageCache->ImageSizeForRenderer(renderer);
    }

    return _image->Size(renderer ? renderer->GetImageOrientation()
                                 : boo::ImageOrientation(boo::ImageOrientation::Orientation::FromImage));
  }

  LayoutSize CachedImage::UnclampedImageSizeForRenderer(const RenderElement *renderer, float multiplier,
                                                        SizeType sizeType) const noexcept
  {
    LayoutSize imageSize = LayoutSize(ImageSizeForRenderer(renderer, sizeType));
    if (imageSize.IsEmpty() || multiplier == 1.0f)
    {
      return imageSize;
    }

    float widthScale = _image->HasRelativeWidth() ? 1.0f : multiplier;
    float heightScale = _image->HasRelativeHeight() ? 1.0f : multiplier;
    imageSize.Scale(widthScale, heightScale);
    return imageSize;
  }

  LayoutSize CachedImage::ImageSizeForRenderer(const RenderElement *renderer, float multiplier,
                                               SizeType sizeType) const noexcept
  {
    auto imageSize = UnclampedImageSizeForRenderer(renderer, multiplier, sizeType);
    if (imageSize.IsEmpty() || multiplier == 1.0f)
    {
      return imageSize;
    }

    // Don't let images that have a width/height >= 1 shrink below 1 when zoomed.
    LayoutSize minimumSize(imageSize.Width() > 0 ? 1 : 0, imageSize.Height() > 0 ? 1 : 0);
    imageSize.ClampToMinimumSize(minimumSize);

    return imageSize;
  }

  void CachedImage::ComputeIntrinsicDimensions(float &intrinsicWidth, float &intrinsicHeight,
                                               FloatSize &intrinsicRatio) noexcept
  {
    if (_image)
    {
      _image->ComputeIntrinsicDimensions(intrinsicWidth, intrinsicHeight, intrinsicRatio);
    }
  }

  void CachedImage::NotifyObservers(const IntRect *changeRect) noexcept
  {
    CachedResourceClientWalker<CachedImageClient> walker(*this);
    while (auto *client = walker.Next())
    {
      client->ImageChanged(this, changeRect);
    }
  }

  void CachedImage::Clear() noexcept
  {
    DestroyDecodedData();
    ClearImage();
    SetEncodedSize(0);
  }

  void CachedImage::CreateImage() noexcept
  {
    // Create the image if it doesn't yet exist.
    if (_image)
    {
      return;
    }

    _imageObserver = CachedImageObserver::Create(*this);

    _image = Image::Create(*_imageObserver);

    if (RefPtr<Image> image = _image)
    {
      if (auto *svgImage = DynamicDowncast<svg::SVGImage>(*image))
      {
        _svgImageCache = CreateUnique<svg::SVGImageCache>(svgImage);
      }

      // Send queued container size requests.
      if (image->UsesContainerSize())
      {
        for (auto &[client, context] : _pendingContainerContextRequests)
        {
          SetContainerContextForClient(*client, context.ContainerSize, context.ContainerZoom,
                                       context.ImageURL);
        }
      }
      _pendingContainerContextRequests.clear();
    }
  }

  CachedImage::CachedImageObserver::CachedImageObserver(CachedImage &image) noexcept
  {
    _cachedImages.insert(&image);
  }

  url::URL CachedImage::CachedImageObserver::SourceUrl() const noexcept
  {
    return !_cachedImages.empty() ? (*_cachedImages.begin())->URL() : url::URL();
  }

  utf8_string CachedImage::CachedImageObserver::MimeType() const noexcept
  {
    return !_cachedImages.empty() ? (*_cachedImages.begin())->CachedResource::MimeType() : utf8_string();
  }

  uint32 CachedImage::CachedImageObserver::NumberOfClients() const noexcept
  {
    return !_cachedImages.empty() ? (*_cachedImages.begin())->CachedResource::NumberOfClients() : 0;
  }

  int64 CachedImage::CachedImageObserver::ExpectedContentLength() const noexcept
  {
    return !_cachedImages.empty() ? (*_cachedImages.begin())->CachedResource::ExpectedContentLength() : 0;
  }

  void CachedImage::CachedImageObserver::EncodedDataStatusChanged(const Image &image,
                                                                  EncodedDataStatus status) noexcept
  {
    for (auto *cachedImage : List<CachedImage *>(_cachedImages.begin(), _cachedImages.end()))
    {
      CachedResourceHandle<CachedImage> protectedImage(cachedImage);
      cachedImage->EncodedDataStatusChanged(image, status);
    }
  }

  void CachedImage::CachedImageObserver::DecodedSizeChanged(const Image &image, int64 delta) noexcept
  {
    for (auto *cachedImage : List<CachedImage *>(_cachedImages.begin(), _cachedImages.end()))
    {
      CachedResourceHandle<CachedImage> protectedImage(cachedImage);
      cachedImage->DecodedSizeChanged(image, delta);
    }
  }

  void CachedImage::CachedImageObserver::DidDraw(const Image &image) noexcept
  {
    for (auto *cachedImage : List<CachedImage *>(_cachedImages.begin(), _cachedImages.end()))
    {
      CachedResourceHandle<CachedImage> protectedImage(cachedImage);
      cachedImage->DidDraw(image);
    }
  }

  bool CachedImage::CachedImageObserver::CanDestroyDecodedData(const Image &image) const noexcept
  {
    for (auto *cachedImage : _cachedImages)
    {
      if (&image != cachedImage->GetImage())
      {
        continue;
      }
      if (!cachedImage->CanDestroyDecodedData(image))
      {
        return false;
      }
    }
    return true;
  }

  void CachedImage::CachedImageObserver::ImageFrameAvailable(const Image &image,
                                                             ImageAnimatingState animatingState,
                                                             const IntRect *changeRect,
                                                             DecodingStatus decodingStatus) noexcept
  {
    for (auto *cachedImage : List<CachedImage *>(_cachedImages.begin(), _cachedImages.end()))
    {
      CachedResourceHandle<CachedImage> protectedImage(cachedImage);
      cachedImage->ImageFrameAvailable(image, animatingState, changeRect, decodingStatus);
    }
  }

  void CachedImage::CachedImageObserver::ChangedInRect(const Image &image, const IntRect *rect) noexcept
  {
    for (auto *cachedImage : List<CachedImage *>(_cachedImages.begin(), _cachedImages.end()))
    {
      CachedResourceHandle<CachedImage> protectedImage(cachedImage);
      cachedImage->ChangedInRect(image, rect);
    }
  }

  void CachedImage::CachedImageObserver::ImageContentChanged(const Image &image) noexcept
  {
    for (auto *cachedImage : List<CachedImage *>(_cachedImages.begin(), _cachedImages.end()))
    {
      CachedResourceHandle<CachedImage> protectedImage(cachedImage);
      cachedImage->ImageContentChanged(image);
    }
  }

  void CachedImage::CachedImageObserver::ScheduleRenderingUpdate(const Image &image) noexcept
  {
    for (auto *cachedImage : List<CachedImage *>(_cachedImages.begin(), _cachedImages.end()))
    {
      CachedResourceHandle<CachedImage> protectedImage(cachedImage);
      cachedImage->ScheduleRenderingUpdate(image);
    }
  }

  bool CachedImage::CachedImageObserver::AllowsAnimation(const Image &image) const noexcept
  {
    // FIXME: Ask the cached images once there are system animation controls
    // (Image::systemAllowsAnimationControls).
    (void)image;
    return true;
  }

  void CachedImage::ClearImage() noexcept
  {
    if (!_image)
    {
      return;
    }

    if (auto imageObserver = std::exchange(_imageObserver, nullptr))
    {
      imageObserver->CachedImages().erase(this);

      if (imageObserver->CachedImages().empty())
      {
        _image->SetImageObserver(nullptr);
      }
    }

    _image = nullptr;
    _allowsOrientationOverride = true;
  }

  void CachedImage::UpdateBufferInternal(const SharedBuffer &data) noexcept
  {
    CachedResourceHandle<CachedImage> protectedThis(*this);
    _data = ShareRefPtr(const_cast<SharedBuffer *>(&data));
    SetEncodedSize(static_cast<uint32>(_data->Size()));
    CreateImage();

    // Have the image update its data from its internal buffer. Decoding the image data
    // will be delayed until info (like size or specific image frames) are queried which
    // usually happens when the observers are repainted.
    EncodedDataStatus encodedDataStatus = UpdateImageData(false);

    if (encodedDataStatus > EncodedDataStatus::Error && encodedDataStatus < EncodedDataStatus::SizeAvailable)
    {
      return;
    }

    if (encodedDataStatus == EncodedDataStatus::Error || _image->IsNull())
    {
      // Image decoding failed. Either we need more image data or the image data is malformed.
      Error(ErrorOccurred() ? GetStatus() : DecodeError);
      if (InCache())
      {
        MemoryCache::Singleton().Remove(*this);
      }
      return;
    }

    // Tell our observers to try to draw.
    NotifyObservers();
  }

  EncodedDataStatus CachedImage::UpdateImageData(bool allDataReceived) noexcept
  {
    if (!_image || !_data)
    {
      return EncodedDataStatus::Error;
    }
    return _image->SetData(RefPtr<SharedBuffer>(_data), allDataReceived);
  }

  void CachedImage::UpdateBuffer(const SharedBuffer &buffer) noexcept
  {
    krys_debug_assert(GetDataBufferingPolicy() == DataBufferingPolicy::BufferData);
    UpdateBufferInternal(buffer);
  }

  void CachedImage::FinishLoading(const SharedBuffer *data) noexcept
  {
    _data = ShareRefPtr(const_cast<SharedBuffer *>(data));
    if (_data)
    {
      SetEncodedSize(static_cast<uint32>(_data->Size()));
      CreateImage();
    }

    EncodedDataStatus encodedDataStatus = UpdateImageData(true);

    if (encodedDataStatus == EncodedDataStatus::Error || !_image || _image->IsNull())
    {
      // Image decoding failed; the image data is malformed.
      Error(ErrorOccurred() ? GetStatus() : DecodeError);
      if (InCache())
      {
        MemoryCache::Singleton().Remove(*this);
      }
      return;
    }

    SetLoading(false);
    SetAllowsOrientationOverride(!LoadFailedOrCanceled() || _image->SourceURL().ProtocolIsData());

    NotifyObservers();
    CachedResource::FinishLoading(data);
  }

  void CachedImage::Error(CachedResource::Status status) noexcept
  {
    Clear();
    CachedResource::Error(status);
    NotifyObservers();
  }

  void CachedImage::ResponseReceived(ResourceResponse &&newResponse) noexcept
  {
    if (!Response().IsNull())
    {
      Clear();
    }
    CachedResource::ResponseReceived(krys::move(newResponse));
  }

  void CachedImage::DestroyDecodedData() noexcept
  {
    bool canDeleteImage = !_image || (_image->HasOneRef() && _image->IsBitmapImage());
    if (canDeleteImage && !IsLoading() && !HasClients())
    {
      _image = nullptr;
      SetDecodedSize(0);
    }
    else if (_image && !ErrorOccurred())
    {
      _image->DestroyDecodedData();
    }
  }

  void CachedImage::EncodedDataStatusChanged(const Image &image, EncodedDataStatus) noexcept
  {
    if (&image != _image.get())
    {
      return;
    }

    NotifyObservers();
  }

  void CachedImage::DecodedSizeChanged(const Image &image, int64 delta) noexcept
  {
    if (&image != _image.get())
    {
      return;
    }

    krys_debug_assert(delta >= 0 || static_cast<int64>(DecodedSize()) + delta >= 0);
    SetDecodedSize(static_cast<uint32>(static_cast<int64>(DecodedSize()) + delta));
  }

  void CachedImage::DidDraw(const Image &image) noexcept
  {
    if (&image != _image.get())
    {
      return;
    }

    CachedResource::DidAccessDecodedData(html::EventLoop::Current().Now());
  }

  bool CachedImage::CanDestroyDecodedData(const Image &image) const noexcept
  {
    if (&image != _image.get())
    {
      return false;
    }

    CachedResourceClientWalker<CachedImageClient> walker(*this);
    while (auto *client = walker.Next())
    {
      if (!client->CanDestroyDecodedData())
      {
        return false;
      }
    }

    return true;
  }

  void CachedImage::ImageFrameAvailable(const Image &image, ImageAnimatingState animatingState,
                                        const IntRect *changeRect, DecodingStatus) noexcept
  {
    if (&image != _image.get())
    {
      return;
    }

    CachedResourceClientWalker<CachedImageClient> walker(*this);
    VisibleInViewportState visibleState = VisibleInViewportState::No;

    while (auto *client = walker.Next())
    {
      // All the clients of animated images have to be notified. The new frame has to be drawn in all of them.
      // FIXME: Static images notify only the clients waiting for asynchronous decoding, once there is any.
      if (animatingState == ImageAnimatingState::No)
      {
        continue;
      }
      if (client->ImageFrameAvailable(*this, animatingState, changeRect) == VisibleInViewportState::Yes)
      {
        visibleState = VisibleInViewportState::Yes;
      }
    }

    if (visibleState == VisibleInViewportState::No && animatingState == ImageAnimatingState::Yes)
    {
      _image->StopAnimation();
    }
  }

  void CachedImage::ChangedInRect(const Image &image, const IntRect *rect) noexcept
  {
    if (&image != _image.get())
    {
      return;
    }
    NotifyObservers(rect);
  }

  void CachedImage::ImageContentChanged(const Image &image) noexcept
  {
    if (&image != _image.get())
    {
      return;
    }

    CachedResourceClientWalker<CachedImageClient> walker(*this);
    while (auto *client = walker.Next())
    {
      client->ImageContentChanged(*this);
    }
  }

  void CachedImage::ScheduleRenderingUpdate(const Image &image) noexcept
  {
    if (&image != _image.get())
    {
      return;
    }

    CachedResourceClientWalker<CachedImageClient> walker(*this);
    while (auto *client = walker.Next())
    {
      client->ScheduleRenderingUpdateForImage(*this);
    }
  }

  bool CachedImage::AllowsAnimation(const Image &image) const noexcept
  {
    if (&image != _image.get())
    {
      return false;
    }

    // FIXME: Ask the clients once there are system animation controls.
    return true;
  }

  bool CachedImage::CurrentFrameKnownToBeOpaque(const RenderElement *renderer) noexcept
  {
    RefPtr<Image> image = ShareRefPtr(ImageForRenderer(renderer));
    return image->CurrentFrameKnownToBeOpaque();
  }

  bool CachedImage::CurrentFrameIsComplete(const RenderElement *renderer) noexcept
  {
    RefPtr<Image> image = ShareRefPtr(ImageForRenderer(renderer));
    return image->CurrentFrameIsComplete();
  }
}
