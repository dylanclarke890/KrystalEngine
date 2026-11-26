#include "Krystal.Web/Loader/Cache/CachedImage.hpp"
#include "Krystal.Web/Graphics/Images/BitmapImage.hpp"
#include "Krystal.Web/Graphics/Images/BrokenImageIcon.hpp"
#include "Krystal.Web/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Web/Loader/Cache/CachedImageClient.hpp"
#include "Krystal.Web/Loader/Cache/CachedResourceClientWalker.hpp"
#include "Krystal.Web/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Web/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Web/Loader/Cache/MemoryCache.hpp"
#include "Krystal.Web/Page/Settings.hpp"
#include "Krystal.Web/Rendering/RenderElement.hpp"
#include "Krystal.Web/Svg/Graphics/SVGImage.hpp"
#include "Krystal.Web/Svg/Graphics/SVGImageCache.hpp"

namespace krys::web
{
  VisibleInViewportState CachedImageClient::ImageFrameAvailable(CachedImage &image, ImageAnimatingState,
                                                                const RectI *changeRect) noexcept
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
    if (auto *document = loader.GetDocument())
    {
      _settings = ShareRef(document->GetSettings());
    }
    CachedResource::Load(loader);
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

    if (_image)
    {
      _image->StartAnimationAsynchronously();
    }

    CachedResource::DidAddClient(client);
  }

  void CachedImage::DidRemoveClient(CachedResourceClient &client) noexcept
  {
    krys_debug_assert(client.ResourceClientType() == CachedImageClient::ExpectedType());

    _pendingContainerContextRequests.erase(&static_cast<CachedImageClient &>(client));
    _clientsWaitingForAsyncDecoding.erase(&static_cast<CachedImageClient &>(client));

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

  Pair<Image *, float> CachedImage::BrokenImage(float) noexcept
  {
    // The icon is drawn as shapes, so one serves every device scale factor.
    return {&BrokenImageIcon::Singleton(), 1.f};
  }

  bool CachedImage::WillPaintBrokenImage() const noexcept
  {
    return ErrorOccurred() && _shouldPaintBrokenImage;
  }

  Image *CachedImage::GetImage() const noexcept
  {
    if (ErrorOccurred() && _shouldPaintBrokenImage)
    {
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
                                                 const net::Url &imageUrl) noexcept
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
        &client, ContainerContext {containerSize, containerZoom, imageUrl});
      return;
    }

    if (!image->DrawsSVGImage())
    {
      image->SetContainerSize(SizeF(containerSize));
      return;
    }

    _svgImageCache->SetContainerContextForClient(client, containerSize, containerZoom, imageUrl);
  }

  SizeF CachedImage::ImageSizeForRenderer(const RenderElement *renderer, SizeType sizeType) const noexcept
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
                                 : img::ImageOrientation(img::ImageOrientation::Orientation::FromImage));
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

    // Zooming out doesn't take a dimension below 1 unless it's 0.
    LayoutSize minimumSize(imageSize.Width() > 0 ? 1 : 0, imageSize.Height() > 0 ? 1 : 0);
    imageSize.ClampToMinimumSize(minimumSize);

    return imageSize;
  }

  void CachedImage::ComputeIntrinsicDimensions(float &intrinsicWidth, float &intrinsicHeight,
                                               SizeF &intrinsicRatio) noexcept
  {
    if (_image)
    {
      _image->ComputeIntrinsicDimensions(intrinsicWidth, intrinsicHeight, intrinsicRatio);
    }
  }

  void CachedImage::NotifyObservers(const RectI *changeRect) noexcept
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
    if (_image)
    {
      return;
    }

    _imageObserver = CachedImageObserver::Create(*this);

    _image = Image::Create(*_imageObserver);

    if (RefPtr<Image> image = _image)
    {
      if (auto *svgImage = DynamicDowncast<SVGImage>(*image))
      {
        _svgImageCache = CreateUnique<SVGImageCache>(svgImage);
      }

      // The container sizes asked for before there was an image.
      if (image->UsesContainerSize())
      {
        for (auto &[client, context] : _pendingContainerContextRequests)
        {
          SetContainerContextForClient(*client, context.ContainerSize, context.ContainerZoom,
                                       context.ImageUrl);
        }
      }
      _pendingContainerContextRequests.clear();
    }
  }

  CachedImage::CachedImageObserver::CachedImageObserver(CachedImage &image) noexcept
  {
    _cachedImages.insert(&image);
  }

  net::Url CachedImage::CachedImageObserver::SourceUrl() const noexcept
  {
    return !_cachedImages.empty() ? (*_cachedImages.begin())->Url() : net::Url();
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
                                                                  img::EncodedDataStatus status) noexcept
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
                                                             const RectI *changeRect,
                                                             img::DecodingStatus decodingStatus) noexcept
  {
    for (auto *cachedImage : List<CachedImage *>(_cachedImages.begin(), _cachedImages.end()))
    {
      CachedResourceHandle<CachedImage> protectedImage(cachedImage);
      cachedImage->ImageFrameAvailable(image, animatingState, changeRect, decodingStatus);
    }
  }

  void CachedImage::CachedImageObserver::ChangedInRect(const Image &image, const RectI *rect) noexcept
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
    return std::ranges::any_of(_cachedImages, [&image](const CachedImage *cachedImage)
                               { return cachedImage->AllowsAnimation(image); });
  }

  Settings *CachedImage::CachedImageObserver::GetSettings() const noexcept
  {
    for (const auto *cachedImage : _cachedImages)
    {
      if (cachedImage->_settings)
      {
        return cachedImage->_settings.get();
      }
    }
    return nullptr;
  }

  bool CachedImage::CachedImageObserver::UseSystemDarkAppearance() const noexcept
  {
    return std::ranges::any_of(_cachedImages, [](const CachedImage *cachedImage)
                               { return cachedImage->UseSystemDarkAppearance(); });
  }

  bool CachedImage::UseSystemDarkAppearance() const noexcept
  {
    // Not only the first client: an image element's loader comes before its renderer.
    CachedResourceClientWalker<CachedImageClient> walker(*this);
    while (auto *client = walker.Next())
    {
      if (client->UseSystemDarkAppearance())
      {
        return true;
      }
    }
    return false;
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

    // The image takes the data; it decodes it only when asked for its size or a frame, usually as it's drawn.
    img::EncodedDataStatus encodedDataStatus = UpdateImageData(false);

    if (encodedDataStatus > img::EncodedDataStatus::Error
        && encodedDataStatus < img::EncodedDataStatus::SizeAvailable)
    {
      return;
    }

    if (encodedDataStatus == img::EncodedDataStatus::Error || _image->IsNull())
    {
      // The data isn't an image.
      Error(ErrorOccurred() ? GetStatus() : DecodeError);
      if (InCache())
      {
        MemoryCache::Singleton().Remove(*this);
      }
      return;
    }

    // The clients can draw what there is.
    NotifyObservers();
  }

  img::EncodedDataStatus CachedImage::UpdateImageData(bool allDataReceived) noexcept
  {
    if (!_image || !_data)
    {
      return img::EncodedDataStatus::Error;
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

    img::EncodedDataStatus encodedDataStatus = UpdateImageData(true);

    if (encodedDataStatus == img::EncodedDataStatus::Error || !_image || _image->IsNull())
    {
      // The data isn't an image.
      Error(ErrorOccurred() ? GetStatus() : DecodeError);
      if (InCache())
      {
        MemoryCache::Singleton().Remove(*this);
      }
      return;
    }

    SetLoading(false);
    SetAllowsOrientationOverride(!LoadFailedOrCanceled() || _image->SourceUrl().ProtocolIsData());

    NotifyObservers();
    CachedResource::FinishLoading(data);
  }

  void CachedImage::Error(CachedResource::Status status) noexcept
  {
    Clear();
    CachedResource::Error(status);
    NotifyObservers();
  }

  void CachedImage::ResponseReceived(net::ResourceResponse &&newResponse) noexcept
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

  void CachedImage::EncodedDataStatusChanged(const Image &image, img::EncodedDataStatus) noexcept
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

  void CachedImage::AddClientWaitingForAsyncDecoding(CachedImageClient &client) noexcept
  {
    krys_debug_assert(client.ResourceClientType() == CachedImageClient::ExpectedType());
    if (_clientsWaitingForAsyncDecoding.contains(&client))
    {
      return;
    }
    if (!HasClient(client))
    {
      // Not one of the image's clients: the root's renderer, painting the body's background image. As only
      // clients can wait, they all do, so the root is repainted with them.
      CachedResourceClientWalker<CachedImageClient> walker(*this);
      while (auto *other = walker.Next())
      {
        _clientsWaitingForAsyncDecoding.insert(other);
      }
    }
    else
    {
      _clientsWaitingForAsyncDecoding.insert(&client);
    }
  }

  void CachedImage::ImageFrameAvailable(const Image &image, ImageAnimatingState animatingState,
                                        const RectI *changeRect, img::DecodingStatus decodingStatus) noexcept
  {
    if (&image != _image.get())
    {
      return;
    }

    CachedResourceClientWalker<CachedImageClient> walker(*this);
    VisibleInViewportState visibleState = VisibleInViewportState::No;

    while (auto *client = walker.Next())
    {
      // An animation's new frame goes to every client; a decoded frame only to the clients waiting for it.
      if (animatingState == ImageAnimatingState::No && !_clientsWaitingForAsyncDecoding.contains(client))
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

    if (decodingStatus != img::DecodingStatus::Partial)
    {
      _clientsWaitingForAsyncDecoding.clear();
    }
  }

  void CachedImage::ChangedInRect(const Image &image, const RectI *rect) noexcept
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

    // An image animates while every client showing it allows it.
    CachedResourceClientWalker<CachedImageClient> walker(*this);
    while (auto *client = walker.Next())
    {
      if (!client->AllowsAnimation())
      {
        return false;
      }
    }
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
