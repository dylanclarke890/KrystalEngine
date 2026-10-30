#include "Krystal.Booey/Loader/ImageLoader.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/LazyLoadImageObserver.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Booey/Loader/Cache/MemoryCache.hpp"
#include "Krystal.Booey/Rendering/RenderImage.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGImage.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
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
      static const StringAtom name {EventNames::Load};
      return name;
    }

    KRYS_NODISCARD const StringAtom &ErrorEventName() noexcept
    {
      static const StringAtom name {EventNames::Error};
      return name;
    }

    KRYS_NODISCARD bool ContainsOnlyASCIIWhitespace(dom::DOMStringView string) noexcept
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

      // FIXME: A CORS resource of another origin isn't reused, once there are origins.

      if (resource->Options().RequestMode != request.Options().RequestMode
          || resource->Options().CredentialsMode != request.Options().CredentialsMode)
      {
        return false;
      }

      return true;
    }
  }

  ImageLoader::ImageLoader(dom::Element &element) noexcept
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

  dom::Document &ImageLoader::GetDocument() noexcept
  {
    return _element.NodeDocument();
  }

  void ImageLoader::ClearImage() noexcept
  {
    ClearImageWithoutConsideringPendingLoadEvent();

    // Only consider updating the protection ref-count of the Element immediately before returning
    // from this function as doing so might result in the destruction of this ImageLoader.
    UpdatedHasPendingEvent();
  }

  void ImageLoader::ClearImageWithoutConsideringPendingLoadEvent() noexcept
  {
    krys_debug_assert(_failedLoadURL.empty());
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
    // If we're not making renderers for the page, then don't load images. We don't want to slow
    // down the raw HTML parsing case by loading images we don't intend to display.
    Ref element = ShareRef(_element);
    Ref document = ShareRef(element->NodeDocument());
    if (!document->HasLivingRenderTree())
    {
      return;
    }

    auto attr = element->ImageSourceURL();

    // Avoid loading a URL we already failed to load.
    if (!_failedLoadURL.empty() && attr == _failedLoadURL)
    {
      return;
    }

    // Do not load any image if the 'src' attribute is missing or if it is
    // an empty string.
    CachedResourceHandle<CachedImage> newImage;
    if (attr && !ContainsOnlyASCIIWhitespace(*attr))
    {
      ResourceLoaderOptions options = CachedResourceLoader::DefaultCachedResourceOptions();
      // FIXME: The content security policy and content extension checks skipped for user agent shadow trees,
      // the referrer policy, fetch priority, the image set initiator and the crossorigin attribute, once
      // there are network loads.
      auto *imageElement = DynamicDowncast<html::HTMLImageElement>(element.get());

      // Use URL from original request for same URL loads in order to preserve the original base URL.
      url::URL imageURL;
      if (_image && *attr == _pendingURL)
      {
        imageURL = _image->URL();
      }
      else
      {
        if (imageElement)
        {
          // The image's current URL is set by source selection, which runs when its src, srcset or sizes
          // attributes change; until it has run, behave as if src were absent.
          imageURL = imageElement->CurrentURL();
          if (imageURL.IsNull())
          {
            DidUpdateCachedImage(relevantMutation, krys::move(newImage));
            return;
          }
        }
        else
        {
          imageURL = document->CompleteURL(*attr);
        }
        _pendingURL = *attr;
      }

      CachedResourceRequest request(ResourceRequest(krys::move(imageURL)), options);
      request.SetInitiatorType(element->LocalName().View());

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

      // If we do not have an image here, it means that a cross-site
      // violation occurred, or that the image was blocked via Content
      // Security Policy, or the page is being dismissed. Trigger an
      // error event if the page is not being dismissed.
      // FIXME: Don't fire it while the page is being dismissed, once there's page dismissal.
      if (!newImage)
      {
        _failedLoadURL = *attr;
        _hasPendingErrorEvent = true;
        LoadEventSender().DispatchEventSoon(*this, ErrorEventName());
      }
      else
      {
        ClearFailedLoadURL();
      }
    }
    else if (attr)
    {
      // Fire an error event if the url is empty.
      _failedLoadURL = *attr;
      _hasPendingErrorEvent = true;
      LoadEventSender().DispatchEventSoon(*this, ErrorEventName());
    }

    DidUpdateCachedImage(relevantMutation, krys::move(newImage));
  }

  void ImageLoader::DidUpdateCachedImage(RelevantMutation relevantMutation,
                                         CachedResourceHandle<CachedImage> &&newImage) noexcept
  {
    CachedResourceHandle oldImage = _image;
    if (newImage != oldImage || relevantMutation == RelevantMutation::Yes)
    {
      if (_hasPendingLoadEvent)
      {
        LoadEventSender().CancelEvent(*this, LoadEventName());
        _hasPendingLoadEvent = false;
      }

      // Cancel error events that belong to the previous load, which is now cancelled by changing the src
      // attribute. If newImage is null and _hasPendingErrorEvent is true, we know the error event has been
      // just posted by this load and we should not cancel the event.
      // FIXME: If both previous load and this one got blocked with an error, we can receive one error event
      // instead of two.
      if (_hasPendingErrorEvent && newImage)
      {
        LoadEventSender().CancelEvent(*this, ErrorEventName());
        _hasPendingErrorEvent = false;
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

        // If newImage is cached, AddClient() will result in the load event
        // being queued to fire.
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

    // Only consider updating the protection ref-count of the Element immediately before returning
    // from this function as doing so might result in the destruction of this ImageLoader.
    UpdatedHasPendingEvent();
  }

  void ImageLoader::UpdateFromElementIgnoringPreviousError(RelevantMutation relevantMutation) noexcept
  {
    ClearFailedLoadURL();
    UpdateFromElement(relevantMutation);
  }

  void ImageLoader::UpdateFromElementIgnoringPreviousErrorToSameValue() noexcept
  {
    if (!_image || !_image->AllowsCaching() || !_failedLoadURL.empty())
    {
      UpdateFromElementIgnoringPreviousError(RelevantMutation::Yes);
      return;
    }
    if (_hasPendingLoadEvent || !_pendingURL.empty())
    {
      return;
    }
    krys_debug_assert(_image);
    _hasPendingLoadEvent = true;
    NotifyFinished(*_image);
  }

  void ImageLoader::NotifyFinished(CachedResource &resource) noexcept
  {
    krys_debug_assert(_failedLoadURL.empty());
    krys_debug_assert(&resource == _image.get());
    (void)resource;

    _pendingURL.clear();

    if (IsDeferred())
    {
      html::LazyLoadImageObserver::Unobserve(_element, GetDocument());
      _lazyImageLoadState = LazyImageLoadState::FullImage;
    }

    if (!_hasPendingLoadEvent)
    {
      SetImageCompleteAndMaybeUpdateRenderer();
      return;
    }

    if (_image->GetResourceError().IsAccessControl())
    {
      SetImageCompleteAndMaybeUpdateRenderer();

      ClearImageWithoutConsideringPendingLoadEvent();

      _hasPendingErrorEvent = true;
      LoadEventSender().DispatchEventSoon(*this, ErrorEventName());

      // FIXME: Log "Cannot load image <url> due to access control checks." to the console, once there's a
      // console.

      krys_debug_assert(!_hasPendingLoadEvent);

      // Only consider updating the protection ref-count of the Element immediately before returning
      // from this function as doing so might result in the destruction of this ImageLoader.
      UpdatedHasPendingEvent();
      return;
    }

    if (_image->GetResourceError().IsCancellation())
    {
      SetImageCompleteAndMaybeUpdateRenderer();

      _hasPendingLoadEvent = false;
      // Only consider updating the protection ref-count of the Element immediately before returning
      // from this function as doing so might result in the destruction of this ImageLoader.
      UpdatedHasPendingEvent();
      return;
    }

    // A bitmap image has no subresources, so its load event can be queued now.
    SetImageCompleteAndMaybeUpdateRenderer();
    LoadEventSender().DispatchEventSoon(*this, LoadEventName());
  }

  RenderImageResource *ImageLoader::GetRenderImageResource() noexcept
  {
    auto *renderer = _element.Renderer();
    if (!renderer)
    {
      return nullptr;
    }

    // We don't return style generated image because it doesn't belong to the ImageLoader.
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

    // Only update the renderer if it doesn't have an image or if what we have
    // is a complete image. This prevents flickering in the case where a dynamic
    // change is happening between two images.
    CachedImage *cachedImage = imageResource->GetCachedImage();
    if (_image != cachedImage && (_imageComplete || !cachedImage))
    {
      imageResource->SetCachedImage(CachedResourceHandle {_image});
    }
  }

  void ImageLoader::SetImageCompleteAndMaybeUpdateRenderer() noexcept
  {
    _imageComplete = true;
    UpdateRenderer();
  }

  void ImageLoader::UpdatedHasPendingEvent() noexcept
  {
    // If an Element that does image loading is removed from the DOM the load/error event for the image is
    // still observable. As long as the ImageLoader is actively loading, the Element itself needs to be ref'ed
    // to keep it from being destroyed by DOM manipulation or garbage collection. If such an Element wishes
    // for the load to stop when removed from the DOM it needs to stop the ImageLoader explicitly.
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
    // Because of lazy image loading, an image's load may be deferred indefinitely. To avoid leaking the
    // element, we only protect it once the load has actually started.
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

    // Only consider updating the protection ref-count of the Element immediately before returning
    // from this function as doing so might result in the destruction of this ImageLoader.
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
      (void)dom::EventDispatcher::FireEvent(dom::DOMStringAtom(EventNames::Error), _element);
    }

    // Only consider updating the protection ref-count of the Element immediately before returning
    // from this function as doing so might result in the destruction of this ImageLoader.
    UpdatedHasPendingEvent();
  }

  void ImageLoader::DispatchPendingLoadEvents() noexcept
  {
    LoadEventSender().DispatchPendingEvents();
  }

  void ImageLoader::ElementDidMoveToNewDocument(dom::Document &oldDocument) noexcept
  {
    ClearFailedLoadURL();
    ClearImage();
    ResetLazyImageLoading(oldDocument);
  }

  void ImageLoader::ClearFailedLoadURL() noexcept
  {
    _failedLoadURL.clear();
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

  void ImageLoader::ResetLazyImageLoading(dom::Document &document) noexcept
  {
    if (IsDeferred())
    {
      html::LazyLoadImageObserver::Unobserve(_element, document);
    }
    _lazyImageLoadState = LazyImageLoadState::None;
  }
}
