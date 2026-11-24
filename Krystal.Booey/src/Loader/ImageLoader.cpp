#include "Krystal.Booey/Loader/ImageLoader.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Graphics/Images/BitmapImage.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/HTML/Images/LazyLoadImageObserver.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Booey/Loader/Cache/MemoryCache.hpp"
#include "Krystal.Booey/Rendering/Replaced/RenderImage.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGImage.hpp"
#include "Krystal.Core/Memory/NeverDestroyed.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD ImageEventSender &LoadEventSender() noexcept
    {
      static MainThreadNeverDestroyed<ImageEventSender> sender;
      return sender;
    }

    KRYS_NODISCARD const StringAtom &LoadEventName() noexcept
    {
      static const StringAtom name {event_names::Load};
      return name;
    }

    KRYS_NODISCARD const StringAtom &ErrorEventName() noexcept
    {
      static const StringAtom name {event_names::Error};
      return name;
    }

    KRYS_NODISCARD bool ContainsOnlyASCIIWhitespace(DOMStringView string) noexcept
    {
      return std::ranges::all_of(string, [](char8 c) { return text::IsASCIIWhitespace(c); });
    }

    KRYS_NODISCARD bool CanReuseFromListOfAvailableImages(const CachedResourceRequest &request) noexcept
    {
      CachedResourceHandle resource =
        MemoryCache::Singleton().ResourceForRequest(request.GetResourceRequest());
      // FIXME: Preloaded resources, once there are preloads.
      if (!resource || resource->StillNeedsLoad())
      {
        return false;
      }

      return true;
    }

    /// @brief Settles each of @p promises as a microtask: fulfilled, or rejected with an `EncodingError` of
    /// @p rejection.
    void SettleDecodePromises(List<ImageLoader::DecodePromise> promises,
                              Maybe<utf8_stringview> rejection) noexcept
    {
      for (auto &promise : promises)
      {
        html::EventLoop::Current().QueueMicrotask(
          [promise = krys::move(promise),
           rejection = rejection.transform([](utf8_stringview message) { return utf8_string(message); })]
          {
            if (rejection)
            {
              promise(Exception(ExceptionCode::EncodingError, *rejection));
            }
            else
            {
              promise({});
            }
          });
      }
    }
  }

  ImageLoader::ImageLoader(boo::Element &element) noexcept
      : _element(element), _derefElementTimer(*this, &ImageLoader::TimerFired)
  {
  }

  ImageLoader::~ImageLoader() noexcept
  {
    if (CachedResourceHandle image = _image)
    {
      image->RemoveClient(*this);
    }

    krys_debug_assert(_hasPendingLoadEvent || _hasPendingErrorEvent
                      || !LoadEventSender().HasPendingEvents(*this));
    if (_hasPendingLoadEvent || _hasPendingErrorEvent)
    {
      LoadEventSender().CancelEvent(*this);
    }
  }

  boo::Document &ImageLoader::GetDocument() noexcept
  {
    return _element.NodeDocument();
  }

  void ImageLoader::ClearImage() noexcept
  {
    ClearImageWithoutConsideringPendingLoadEvent();

    // Last, as letting the element go may destroy this loader.
    UpdatedHasPendingEvent();
  }

  void ImageLoader::ClearImageWithoutConsideringPendingLoadEvent() noexcept
  {
    krys_debug_assert(_failedLoadUrl.empty());
    if (CachedResourceHandle oldImage = _image)
    {
      _image = nullptr;
      if (_hasPendingLoadEvent || _hasPendingErrorEvent)
      {
        LoadEventSender().CancelEvent(*this);
        _hasPendingLoadEvent = _hasPendingErrorEvent = false;
      }
      _imageComplete = true;
      oldImage->RemoveClient(*this);
    }

    if (auto *imageResource = GetRenderImageResource())
    {
      imageResource->ResetAnimation();
    }
  }

  void ImageLoader::UpdateFromElement(RelevantMutation relevantMutation) noexcept
  {
    // Only a document that renders loads its images.
    Ref element = ShareRef(_element);
    Ref document = ShareRef(element->NodeDocument());
    if (!document->HasLivingRenderTree())
    {
      return;
    }

    auto attr = element->ImageSourceUrl();

    // The source that failed last isn't tried again until it's set again.
    if (!_failedLoadUrl.empty() && attr == _failedLoadUrl)
    {
      return;
    }

    // A missing source, or one that's only whitespace, loads nothing.
    CachedResourceHandle<CachedImage> newImage;
    if (attr && !ContainsOnlyASCIIWhitespace(*attr))
    {
      ResourceLoaderOptions options = CachedResourceLoader::DefaultCachedResourceOptions();
      auto *imageElement = DynamicDowncast<HTMLImageElement>(element.get());

      // The same source keeps the URL it resolved to, though the base URL may have changed since.
      net::Url imageUrl;
      if (_image && *attr == _pendingUrl)
      {
        imageUrl = _image->Url();
      }
      else
      {
        if (imageElement)
        {
          // The image's current URL is set by source selection, which runs when its src, srcset or sizes
          // attributes change; until it has run, behave as if src were absent.
          imageUrl = imageElement->CurrentUrl();
          if (imageUrl.IsNull())
          {
            DidUpdateCachedImage(relevantMutation, krys::move(newImage));
            return;
          }
        }
        else
        {
          imageUrl = document->CompleteUrl(*attr);
        }
        _pendingUrl = *attr;
      }

      CachedResourceRequest request(net::ResourceRequest(krys::move(imageUrl)), options);
      request.SetInitiatorType(element->LocalName().View());

      if (_loadManually)
      {
        newImage = document->GetCachedResourceLoader().CreateManuallyLoadedImage(krys::move(request));
      }
      else
      {
        if (_lazyImageLoadState == LazyImageLoadState::None && imageElement)
        {
          if (imageElement->IsLazyLoadable() && !CanReuseFromListOfAvailableImages(request))
          {
            _lazyImageLoadState = LazyImageLoadState::Deferred;
            request.SetIgnoreForRequestCount(true);
          }
        }
        auto imageLoading = _lazyImageLoadState == LazyImageLoadState::Deferred
                              ? ImageLoading::DeferredUntilVisible
                              : ImageLoading::Immediate;
        auto requested = document->GetCachedResourceLoader().RequestImage(krys::move(request), imageLoading);
        newImage = requested ? *requested : CachedResourceHandle<CachedImage>();
      }

      // The request failed at once: no frame, an invalid URL, one an SVG image may not load, or a load that
      // failed straight away.
      // FIXME: Don't fire it while the page is being dismissed, once there's page dismissal.
      if (!newImage)
      {
        _failedLoadUrl = *attr;
        _hasPendingErrorEvent = true;
        LoadEventSender().DispatchEventSoon(*this, ErrorEventName());
      }
      else
      {
        ClearFailedLoadUrl();
      }
    }
    // Fire an error event if the url is empty, or if an image that uses srcset or picture has no source (the
    // selected source is null and it uses srcset or picture).
    else if (auto *imageElement = DynamicDowncast<HTMLImageElement>(element.get());
             attr || (imageElement && imageElement->UsesSrcsetOrPicture()))
    {
      _failedLoadUrl = attr.value_or(DOMString());
      _hasPendingErrorEvent = true;
      LoadEventSender().DispatchEventSoon(*this, ErrorEventName());
    }

    DidUpdateCachedImage(relevantMutation, krys::move(newImage));
  }

  void ImageLoader::DidUpdateCachedImage(RelevantMutation relevantMutation,
                                         const CachedResourceHandle<CachedImage> &newImage) noexcept
  {
    CachedResourceHandle oldImage = _image;
    if (newImage != oldImage || relevantMutation == RelevantMutation::Yes)
    {
      if (_hasPendingLoadEvent)
      {
        LoadEventSender().CancelEvent(*this, LoadEventName());
        _hasPendingLoadEvent = false;
      }

      // The previous load's pending error is canceled by a new image. Without one, this load failed too, and
      // the one pending error stands for both: only the last of several updates fires.
      if (_hasPendingErrorEvent && newImage)
      {
        LoadEventSender().CancelEvent(*this, ErrorEventName());
        _hasPendingErrorEvent = false;
      }

      // The current request changed, so the promises waiting for the old image can't be kept.
      if (newImage != oldImage && !_decodingPromises.empty())
      {
        RejectDecodePromises(u8"The image's source changed.");
      }

      _image = newImage;
      _hasPendingLoadEvent = static_cast<bool>(newImage);
      _imageComplete = !newImage;

      if (newImage)
      {
        // The before-load step: the renderer takes the new image once the element is connected.
        if (GetDocument().HasLivingRenderTree() && _element.IsConnected())
        {
          UpdateRenderer();
        }

        if (_lazyImageLoadState == LazyImageLoadState::Deferred)
        {
          html::LazyLoadImageObserver::Observe(_element);
        }

        // A cached image that has loaded finishes at once, which queues the load event.
        newImage->AddClient(*this);
      }
      else
      {
        ResetLazyImageLoading(GetDocument());
      }

      if (oldImage)
      {
        oldImage->RemoveClient(*this);
        UpdateRenderer();
      }
    }

    if (auto *imageResource = GetRenderImageResource())
    {
      imageResource->ResetAnimation();
    }

    // Last, as letting the element go may destroy this loader.
    UpdatedHasPendingEvent();
  }

  void ImageLoader::UpdateFromElementIgnoringPreviousError(RelevantMutation relevantMutation) noexcept
  {
    ClearFailedLoadUrl();
    UpdateFromElement(relevantMutation);
  }

  void ImageLoader::UpdateFromElementIgnoringPreviousErrorToSameValue() noexcept
  {
    if (!_image || !_image->AllowsCaching() || !_failedLoadUrl.empty())
    {
      UpdateFromElementIgnoringPreviousError(RelevantMutation::Yes);
      return;
    }
    if (_hasPendingLoadEvent || !_pendingUrl.empty())
    {
      return;
    }
    krys_debug_assert(_image);
    _hasPendingLoadEvent = true;
    NotifyFinished(*_image);
  }

  void ImageLoader::NotifyFinished(CachedResource &resource) noexcept
  {
    krys_debug_assert(_failedLoadUrl.empty());
    krys_debug_assert(&resource == _image.get());
    (void)resource;

    _pendingUrl.clear();

    if (IsDeferred())
    {
      html::LazyLoadImageObserver::Unobserve(_element, GetDocument());
      _lazyImageLoadState = LazyImageLoadState::FullImage;
    }

    if (!_hasPendingLoadEvent)
    {
      SetImageCompleteAndMaybeUpdateRenderer();
      if (!_decodingPromises.empty())
      {
        DecodeImage();
      }
      return;
    }

    if (_image->GetResourceError().IsAccessControl())
    {
      SetImageCompleteAndMaybeUpdateRenderer();

      ClearImageWithoutConsideringPendingLoadEvent();

      _hasPendingErrorEvent = true;
      LoadEventSender().DispatchEventSoon(*this, ErrorEventName());

      if (!_decodingPromises.empty())
      {
        RejectDecodePromises(u8"Access control error.");
      }

      krys_debug_assert(!_hasPendingLoadEvent);

      // Last, as letting the element go may destroy this loader.
      UpdatedHasPendingEvent();
      return;
    }

    if (_image->GetResourceError().IsCancellation())
    {
      SetImageCompleteAndMaybeUpdateRenderer();

      if (!_decodingPromises.empty())
      {
        RejectDecodePromises(u8"Loading was canceled.");
      }
      _hasPendingLoadEvent = false;
      // Last, as letting the element go may destroy this loader.
      UpdatedHasPendingEvent();
      return;
    }

    // A bitmap image has no subresources, so its load event can be queued now.
    SetImageCompleteAndMaybeUpdateRenderer();
    if (!_decodingPromises.empty())
    {
      DecodeImage();
    }
    LoadEventSender().DispatchEventSoon(*this, LoadEventName());
  }

  void ImageLoader::Decode(DecodePromise promise) noexcept
  {
    if (!promise)
    {
      return;
    }
    _decodingPromises.push_back(krys::move(promise));

    if (!GetDocument().IsFullyActive())
    {
      RejectDecodePromises(u8"Inactive document.");
      return;
    }

    auto source = _element.ImageSourceUrl();
    if (!source || ContainsOnlyASCIIWhitespace(*source))
    {
      RejectDecodePromises(u8"Missing source URL.");
      return;
    }

    if (_imageComplete)
    {
      DecodeImage();
    }
  }

  void ImageLoader::DecodeImage() noexcept
  {
    krys_debug_assert(!_decodingPromises.empty());

    if (!GetDocument().IsFullyActive())
    {
      RejectDecodePromises(u8"Inactive document.");
      return;
    }

    if (!_image || !_image->GetImage() || _image->ErrorOccurred())
    {
      RejectDecodePromises(u8"Loading error.");
      return;
    }

    // Other images (SVG images) draw without decoding.
    auto *bitmapImage = DynamicDowncast<BitmapImage>(_image->GetImage());
    if (!bitmapImage)
    {
      ResolveDecodePromises();
      return;
    }

    bitmapImage->Decode(
      [promises = std::exchange(_decodingPromises, {})](img::DecodingStatus decodingStatus) mutable
      {
        krys_debug_assert(decodingStatus != img::DecodingStatus::Decoding);
        SettleDecodePromises(krys::move(promises), decodingStatus == img::DecodingStatus::Invalid
                                                     ? Maybe<utf8_stringview>(u8"Decoding error.")
                                                     : null);
      });
  }

  void ImageLoader::ResolveDecodePromises() noexcept
  {
    SettleDecodePromises(std::exchange(_decodingPromises, {}), null);
  }

  void ImageLoader::RejectDecodePromises(utf8_stringview message) noexcept
  {
    SettleDecodePromises(std::exchange(_decodingPromises, {}), message);
  }

  RenderImageResource *ImageLoader::GetRenderImageResource() noexcept
  {
    auto *renderer = _element.Renderer();
    if (!renderer)
    {
      return nullptr;
    }

    // Generated content's image isn't the element's.
    if (auto *renderImage = DynamicDowncast<RenderImage>(*renderer);
        renderImage && !renderImage->IsGeneratedContent())
    {
      return &renderImage->ImageResource();
    }

    if (auto *svgImage = DynamicDowncast<LegacyRenderSVGImage>(*renderer))
    {
      return &svgImage->ImageResource();
    }

    // FIXME: Video posters, once they have renderers.
    return nullptr;
  }

  void ImageLoader::UpdateRenderer() noexcept
  {
    auto *imageResource = GetRenderImageResource();
    if (!imageResource)
    {
      return;
    }

    // The renderer keeps the old image until the new one is complete, so switching doesn't flicker.
    CachedImage *cachedImage = imageResource->GetCachedImage();
    if (_image != cachedImage && (_imageComplete || !cachedImage))
    {
      imageResource->SetCachedImage(CachedResourceHandle {_image});

      // With its image gone, the box is sized for the alt text instead.
      if (auto *renderImage = DynamicDowncast<RenderImage>(GetElement().Renderer()); renderImage && !_image)
      {
        renderImage->UpdateForAltText();
      }
    }
  }

  void ImageLoader::SetImageCompleteAndMaybeUpdateRenderer() noexcept
  {
    _imageComplete = true;
    UpdateRenderer();
  }

  void ImageLoader::UpdatedHasPendingEvent() noexcept
  {
    // A removed element's load or error event can still be heard, so it stays alive until it fires.
    bool wasProtected = _elementIsProtected;
    _elementIsProtected = HasPendingActivity();
    if (wasProtected == _elementIsProtected)
    {
      return;
    }

    if (_elementIsProtected)
    {
      if (_derefElementTimer.IsActive())
      {
        _derefElementTimer.Stop();
      }
      else
      {
        _protectedElement = ShareRefPtr(&_element);
      }
    }
    else
    {
      krys_debug_assert(!_derefElementTimer.IsActive());
      _derefElementTimer.StartOneShot({});
    }
  }

  void ImageLoader::TimerFired() noexcept
  {
    _protectedElement = nullptr;
  }

  bool ImageLoader::HasPendingActivity() const noexcept
  {
    // A lazy load may never start, so it keeps the element alive only once it has.
    bool imageWillBeLoadedLater = _image && !_image->IsLoading() && _image->StillNeedsLoad();
    return (_hasPendingLoadEvent && !imageWillBeLoadedLater) || _hasPendingErrorEvent;
  }

  void ImageLoader::DispatchPendingEvent(ImageEventSender *eventSender, const StringAtom &eventType) noexcept
  {
    krys_debug_assert(eventSender == &LoadEventSender());
    (void)eventSender;
    if (eventType == LoadEventName())
    {
      DispatchPendingLoadEvent();
    }
    if (eventType == ErrorEventName())
    {
      DispatchPendingErrorEvent();
    }
  }

  void ImageLoader::DispatchPendingLoadEvent() noexcept
  {
    if (!_hasPendingLoadEvent)
    {
      return;
    }
    if (!_image)
    {
      return;
    }
    _hasPendingLoadEvent = false;
    if (GetDocument().HasLivingRenderTree())
    {
      DispatchLoadEvent();
    }

    // Last, as letting the element go may destroy this loader.
    UpdatedHasPendingEvent();
  }

  void ImageLoader::DispatchPendingErrorEvent() noexcept
  {
    if (!_hasPendingErrorEvent)
    {
      return;
    }
    _hasPendingErrorEvent = false;
    LoadEventSender().CancelEvent(*this, ErrorEventName());
    if (GetDocument().HasLivingRenderTree())
    {
      (void)dom::EventDispatcher::FireEvent(event_names::Error, _element);
    }

    // Last, as letting the element go may destroy this loader.
    UpdatedHasPendingEvent();
  }

  void ImageLoader::DispatchPendingLoadEvents() noexcept
  {
    LoadEventSender().DispatchPendingEvents();
  }

  void ImageLoader::ElementDidMoveToNewDocument(boo::Document &oldDocument) noexcept
  {
    ClearFailedLoadUrl();
    ClearImage();
    ResetLazyImageLoading(oldDocument);
  }

  void ImageLoader::ClearFailedLoadUrl() noexcept
  {
    _failedLoadUrl.clear();
  }

  void ImageLoader::LoadDeferredImage() noexcept
  {
    if (_lazyImageLoadState != LazyImageLoadState::Deferred)
    {
      return;
    }
    _lazyImageLoadState = LazyImageLoadState::LoadImmediately;
    UpdateFromElement(RelevantMutation::No);
  }

  void ImageLoader::ResetLazyImageLoading(boo::Document &document) noexcept
  {
    if (IsDeferred())
    {
      html::LazyLoadImageObserver::Unobserve(_element, document);
    }
    _lazyImageLoadState = LazyImageLoadState::None;
  }
}
