#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Loader/Cache/CachedCSSStyleSheet.hpp"
#include "Krystal.Booey/Loader/Cache/MemoryCache.hpp"
#include "Krystal.Booey/Loader/LoaderStrategy.hpp"
#include "Krystal.Booey/Page/Clients/BooeyClient.hpp"
#include "Krystal.Booey/Page/Frame.hpp"
#include "Krystal.Booey/Page/Settings.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD CachedResourceHandle<CachedResource>
      CreateResource(CachedResource::Type type, CachedResourceRequest &&request, Settings &settings) noexcept
    {
      switch (type)
      {
        case CachedResource::Type::ImageResource:
        {
          Ref<CachedResource> resource = AdoptRef(*new CachedImage(krys::move(request)));
          return CachedResourceHandle<CachedResource>(resource.get());
        }
        case CachedResource::Type::CSSStyleSheet:
        {
          Ref<CachedResource> resource = AdoptRef(*new CachedCSSStyleSheet(krys::move(request)));
          return CachedResourceHandle<CachedResource>(resource.get());
        }
        case CachedResource::Type::FontResource:
        {
          Ref<CachedResource> resource = AdoptRef(*new CachedFont(krys::move(request)));
          return CachedResourceHandle<CachedResource>(resource.get());
        }
        case CachedResource::Type::SVGDocumentResource:
        {
          Ref<CachedResource> resource =
            AdoptRef(*new CachedSVGDocument(krys::move(request), ShareRef(settings)));
          return CachedResourceHandle<CachedResource>(resource.get());
        }
        case CachedResource::Type::Icon:
        {
          Ref<CachedResource> resource = AdoptRef(*new CachedRawResource(krys::move(request), type));
          return CachedResourceHandle<CachedResource>(resource.get());
        }
        default:
          // FIXME: The other resource types, once they're loaded.
          krys_assert_msg(false, "Not reached");
          return nullptr;
      }
    }
  }

  CachedResourceLoader::CachedResourceLoader(boo::Document *document) noexcept
      : _document(CreateWeakPtr(document)),
        _garbageCollectDocumentResourcesTimer(*this, &CachedResourceLoader::GarbageCollectDocumentResources)
  {
  }

  CachedResourceLoader::~CachedResourceLoader() noexcept = default;

  CachedResource *CachedResourceLoader::GetCachedResource(utf8_stringview resourceUrl) const noexcept
  {
    auto *document = GetDocument();
    krys_debug_assert(document);
    return document ? GetCachedResource(
                        MemoryCache::RemoveFragmentIdentifierIfNeeded(document->CompleteUrl(resourceUrl)))
                    : nullptr;
  }

  CachedResource *CachedResourceLoader::GetCachedResource(const net::Url &url) const noexcept
  {
    krys_debug_assert(!MemoryCache::ShouldRemoveFragmentIdentifier(url));
    auto found = _documentResources.find(url.String());
    return found != _documentResources.end() ? found->second.get() : nullptr;
  }

  boo::Frame *CachedResourceLoader::Frame() const noexcept
  {
    auto *document = GetDocument();
    return document ? document->Frame() : nullptr;
  }

  boo::Document *CachedResourceLoader::GetDocument() const noexcept
  {
    return _document.get();
  }

  void CachedResourceLoader::SetDocument(boo::Document *document) noexcept
  {
    _document = CreateWeakPtr(document);
  }

  ResourceErrorOr<CachedResourceHandle<CachedImage>>
    CachedResourceLoader::RequestImage(CachedResourceRequest &&request, ImageLoading imageLoading) noexcept
  {
    auto resource = RequestResource(CachedResource::Type::ImageResource, krys::move(request), imageLoading);
    if (!resource)
    {
      return Unexpected<net::ResourceError>(resource.error());
    }
    return CachedResourceHandle<CachedImage>(Downcast<CachedImage>(resource->get()));
  }

  CachedResourceHandle<CachedImage>
    CachedResourceLoader::CreateManuallyLoadedImage(CachedResourceRequest &&request) noexcept
  {
    Ref<CachedImage> image = AdoptRef(*new CachedImage(krys::move(request)));
    image->SetStatus(CachedResource::Pending);
    image->SetLoading(true);
    _documentResources[image->Url().String()] = CachedResourceHandle<CachedResource>(image.get());
    return CachedResourceHandle<CachedImage>(image.get());
  }

  ResourceErrorOr<CachedResourceHandle<CachedCSSStyleSheet>>
    CachedResourceLoader::RequestCSSStyleSheet(CachedResourceRequest &&request) noexcept
  {
    auto resource = RequestResource(CachedResource::Type::CSSStyleSheet, krys::move(request));
    if (!resource)
    {
      return Unexpected<net::ResourceError>(resource.error());
    }
    return CachedResourceHandle<CachedCSSStyleSheet>(Downcast<CachedCSSStyleSheet>(resource->get()));
  }

  ResourceErrorOr<CachedResourceHandle<CachedFont>>
    CachedResourceLoader::RequestFont(CachedResourceRequest &&request) noexcept
  {
    auto resource = RequestResource(CachedResource::Type::FontResource, krys::move(request));
    if (!resource)
    {
      return Unexpected<net::ResourceError>(resource.error());
    }
    return CachedResourceHandle<CachedFont>(Downcast<CachedFont>(resource->get()));
  }

  ResourceErrorOr<CachedResourceHandle<CachedSVGDocument>>
    CachedResourceLoader::RequestSVGDocument(CachedResourceRequest &&request) noexcept
  {
    auto resource = RequestResource(CachedResource::Type::SVGDocumentResource, krys::move(request));
    if (!resource)
    {
      return Unexpected<net::ResourceError>(resource.error());
    }
    return CachedResourceHandle<CachedSVGDocument>(Downcast<CachedSVGDocument>(resource->get()));
  }

  ResourceErrorOr<CachedResourceHandle<CachedRawResource>>
    CachedResourceLoader::RequestIcon(CachedResourceRequest &&request) noexcept
  {
    auto resource = RequestResource(CachedResource::Type::Icon, krys::move(request));
    if (!resource)
    {
      return Unexpected<net::ResourceError>(resource.error());
    }
    return CachedResourceHandle<CachedRawResource>(Downcast<CachedRawResource>(resource->get()));
  }

  ResourceErrorOr<CachedResourceHandle<CachedResource>>
    CachedResourceLoader::RequestResource(CachedResource::Type type, CachedResourceRequest &&request,
                                          ImageLoading imageLoading) noexcept
  {
    net::Url url = request.GetResourceRequest().Url();
    if (!Frame())
    {
      return Unexpected<net::ResourceError>(
        net::ResourceError(utf8_string(ErrorDomainBooeyInternal), 0, url, u8"Invalid loader state"));
    }

    if (!url.IsValid())
    {
      return Unexpected<net::ResourceError>(
        net::ResourceError(utf8_string(ErrorDomainBooeyInternal), 0, url, u8"URL is invalid"));
    }

    // An SVG image's document may load nothing but data URLs.
    if (type != CachedResource::Type::MainResource)
    {
      if (auto *frame = Frame();
          frame && frame->GetBooeyClient().IsSVGImageBooeyClient() && !url.ProtocolIsData())
      {
        return Unexpected<net::ResourceError>(net::ResourceError(utf8_string(ErrorDomainBooeyInternal), 0,
                                                                 url, u8"Not allowed to request resource",
                                                                 net::ResourceError::Type::AccessControl));
      }
    }

    auto &memoryCache = MemoryCache::Singleton();
    if (request.AllowsCaching() && memoryCache.Disabled())
    {
      _documentResources.erase(url.String());
    }

    CachedResourceHandle<CachedResource> resource;
    if (request.AllowsCaching())
    {
      resource = memoryCache.ResourceForRequest(request.GetResourceRequest());
    }

    RevalidationPolicy policy = DetermineRevalidationPolicy(type, resource.get(), imageLoading);
    switch (policy)
    {
      case Reload: memoryCache.Remove(*resource); [[fallthrough]];
      case Load:   resource = LoadResource(type, krys::move(request)); break;
      case Use:
        krys_debug_assert(resource);
        memoryCache.ResourceAccessed(*resource);
        break;
    }
    krys_debug_assert(resource);

    if ((policy != Use || resource->StillNeedsLoad()) && imageLoading == ImageLoading::Immediate)
    {
      resource->Load(*this);

      // A load finishes later, but it can fail at once.
      if (resource->ErrorOccurred())
      {
        if (resource->AllowsCaching() && resource->InCache())
        {
          memoryCache.Remove(*resource);
        }

        auto resourceError = resource->GetResourceError();
        // A failure without an error, or a cancellation, is taken to be refused access.
        if (resourceError.IsNull() || resourceError.IsCancellation())
        {
          return Unexpected<net::ResourceError>(
            net::ResourceError({}, 0, url, {}, net::ResourceError::Type::AccessControl));
        }
        return Unexpected<net::ResourceError>(resourceError);
      }
    }

    if (!url.ProtocolIsData())
    {
      _validatedUrls.insert(url.String());
    }

    krys_debug_assert(resource->Url() == url);
    if (!_garbageCollectDocumentResourcesTimer.IsActive())
    {
      _garbageCollectDocumentResourcesTimer.StartOneShot({});
    }
    _documentResources[resource->Url().String()] = resource;
    return resource;
  }

  CachedResourceHandle<CachedResource>
    CachedResourceLoader::LoadResource(CachedResource::Type type, CachedResourceRequest &&request) noexcept
  {
    auto &memoryCache = MemoryCache::Singleton();
    krys_debug_assert(!request.AllowsCaching()
                      || !memoryCache.ResourceForRequest(request.GetResourceRequest()));

    auto *document = GetDocument();
    auto resource =
      CreateResource(type, krys::move(request), document ? document->GetSettings() : Settings::Default());

    if (resource->AllowsCaching())
    {
      (void)memoryCache.Add(*resource);
    }

    return resource;
  }

  void CachedResourceLoader::IncrementRequestCount(const CachedResource &resource) noexcept
  {
    if (resource.IgnoreForRequestCount())
    {
      return;
    }
    ++_requestCount;
  }

  void CachedResourceLoader::DecrementRequestCount(const CachedResource &resource) noexcept
  {
    if (resource.IgnoreForRequestCount())
    {
      return;
    }
    --_requestCount;
    krys_debug_assert(_requestCount >= 0);
  }

  void CachedResourceLoader::LoadDone() noexcept
  {
    Ref<CachedResourceLoader> protectedThis = ShareRef(*this);
    if (RefPtr<boo::Frame> frame = ShareRefPtr(Frame()))
    {
      frame->Loader().LoadDone();
    }
    if (!_garbageCollectDocumentResourcesTimer.IsActive())
    {
      _garbageCollectDocumentResourcesTimer.StartOneShot({});
    }
  }

  void CachedResourceLoader::DocumentDidFinishLoadEvent() noexcept
  {
    _validatedUrls.clear();
  }

  CachedResourceLoader::RevalidationPolicy CachedResourceLoader::DetermineRevalidationPolicy(
    CachedResource::Type type, CachedResource *existingResource, ImageLoading imageLoading) const noexcept
  {
    if (!existingResource)
    {
      return Load;
    }

    // A resource of another type for the same URL is replaced.
    if (existingResource->GetType() != type)
    {
      return Reload;
    }

    // A deferred (lazy) image doesn't reuse the cached one: its own replaces it, and loads once the image
    // is near the viewport.
    if (imageLoading == ImageLoading::DeferredUntilVisible)
    {
      return Reload;
    }

    // A document doesn't load a URL twice, whatever the cache policy.
    if (_validatedUrls.contains(existingResource->Url().String()))
    {
      return Use;
    }

    auto cachePolicy = GetCachePolicy(type);

    if (cachePolicy == CachePolicy::Reload)
    {
      return Reload;
    }

    // A failed load is tried again.
    if (existingResource->ErrorOccurred())
    {
      return Reload;
    }

    if (existingResource->IsLoading())
    {
      // A main resource isn't shared while it loads, as canceling one load would cancel both.
      if (type == CachedResource::Type::MainResource)
      {
        return Reload;
      }
      // A subresource is, whatever the cache policy.
      return Use;
    }
    return Use;
  }

  CachePolicy CachedResourceLoader::GetCachePolicy(CachedResource::Type type) const noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return CachePolicy::Use;
    }

    // The host's policy, unless the load is a reload.
    // FIXME: Main resources' policy, once main resources go through the memory cache.
    (void)type;
    auto policy = frame->Loader().SubresourceCachePolicy();
    return policy != CachePolicy::Use ? policy : frame->SubresourceCachePolicy();
  }

  // The document's resources are held by handles, which keep them alive, so the ones nobody else holds are
  // dropped here.
  void CachedResourceLoader::GarbageCollectDocumentResources() noexcept
  {
    List<utf8_string> resourcesToDelete;

    for (auto &[url, resource] : _documentResources)
    {
      // The memory cache holds a reference too while the resource is in it.
      uint32 otherReferences = resource->InCache() ? 1 : 0;
      if (resource->GetRefCount() == 1 + otherReferences && !resource->IsLoading())
      {
        resourcesToDelete.push_back(url);
      }
    }

    for (auto &url : resourcesToDelete)
    {
      _documentResources.erase(url);
    }
  }

  const ResourceLoaderOptions &CachedResourceLoader::DefaultCachedResourceOptions() noexcept
  {
    static const ResourceLoaderOptions options = []
    {
      ResourceLoaderOptions result;
      result.SendLoadCallbacks = SendCallbackPolicy::SendCallbacks;
      result.DataBuffering = DataBufferingPolicy::BufferData;
      result.ContentSecurityPolicy = ContentSecurityPolicyImposition::DoPolicyCheck;
      result.Caching = CachingPolicy::AllowCaching;
      return result;
    }();
    return options;
  }
}
