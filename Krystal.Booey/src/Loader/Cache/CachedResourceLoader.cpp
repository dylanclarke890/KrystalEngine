#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Loader/Cache/MemoryCache.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD CachedResourceHandle<CachedResource>
      CreateResource(CachedResource::Type type, CachedResourceRequest &&request) noexcept
    {
      switch (type)
      {
        case CachedResource::Type::ImageResource:
        {
          Ref<CachedResource> resource = AdoptRef(*new CachedImage(krys::move(request)));
          return CachedResourceHandle<CachedResource>(resource.get());
        }
        default:
          // FIXME: The other resource types, once they're loaded.
          krys_assert_msg(false, "Not reached");
          return nullptr;
      }
    }
  }

  CachedResourceLoader::CachedResourceLoader(dom::Document *document) noexcept
      : _document(WeakPtr<dom::Document>::WithRef(document)),
        _garbageCollectDocumentResourcesTimer(*this, &CachedResourceLoader::GarbageCollectDocumentResources)
  {
  }

  CachedResourceLoader::~CachedResourceLoader() noexcept = default;

  CachedResource *CachedResourceLoader::GetCachedResource(utf8_stringview resourceURL) const noexcept
  {
    auto *document = GetDocument();
    krys_debug_assert(document);
    return document ? GetCachedResource(
                        MemoryCache::RemoveFragmentIdentifierIfNeeded(document->CompleteURL(resourceURL)))
                    : nullptr;
  }

  CachedResource *CachedResourceLoader::GetCachedResource(const url::URL &url) const noexcept
  {
    krys_debug_assert(!MemoryCache::ShouldRemoveFragmentIdentifier(url));
    auto found = _documentResources.find(url.String());
    return found != _documentResources.end() ? found->second.get() : nullptr;
  }

  LocalFrame *CachedResourceLoader::Frame() const noexcept
  {
    auto *document = GetDocument();
    return document ? document->Frame() : nullptr;
  }

  dom::Document *CachedResourceLoader::GetDocument() const noexcept
  {
    return _document.get();
  }

  void CachedResourceLoader::SetDocument(dom::Document *document) noexcept
  {
    _document = WeakPtr<dom::Document>::WithRef(document);
  }

  ResourceErrorOr<CachedResourceHandle<CachedImage>>
    CachedResourceLoader::RequestImage(CachedResourceRequest &&request, ImageLoading imageLoading) noexcept
  {
    if (imageLoading == ImageLoading::Immediate)
    {
      imageLoading = ClientDefersImage(request.GetResourceRequest().URL());
    }
    auto resource = RequestResource(CachedResource::Type::ImageResource, krys::move(request), imageLoading);
    if (!resource)
    {
      return Unexpected(resource.error());
    }
    return CachedResourceHandle<CachedImage>(Downcast<CachedImage>(resource->get()));
  }

  ResourceErrorOr<CachedResourceHandle<CachedResource>>
    CachedResourceLoader::RequestResource(CachedResource::Type type, CachedResourceRequest &&request,
                                          ImageLoading imageLoading) noexcept
  {
    url::URL url = request.GetResourceRequest().URL();
    if (!Frame())
    {
      return Unexpected(
        ResourceError(utf8_string(ErrorDomainBooeyInternal), 0, url, u8"Invalid loader state"));
    }

    if (!url.IsValid())
    {
      return Unexpected(ResourceError(utf8_string(ErrorDomainBooeyInternal), 0, url, u8"URL is invalid"));
    }

    // FIXME: Content security policy, mixed content and the other checks before a request (canRequest),
    // once there are network loads.

    auto &memoryCache = MemoryCache::Singleton();
    if (request.AllowsCaching() && memoryCache.Disabled())
    {
      _documentResources.erase(url.String());
    }

    // See if we can use an existing resource from the cache.
    CachedResourceHandle<CachedResource> resource;
    if (request.AllowsCaching())
    {
      resource = memoryCache.ResourceForRequest(request.GetResourceRequest());
    }

    RevalidationPolicy policy = DetermineRevalidationPolicy(type, request, resource.get(), imageLoading);
    switch (policy)
    {
      case Reload:
      case Revalidate: memoryCache.Remove(*resource); [[fallthrough]];
      case Load:       resource = LoadResource(type, krys::move(request)); break;
      case Use:
        krys_debug_assert(resource);
        memoryCache.ResourceAccessed(*resource);
        break;
    }
    krys_debug_assert(resource);

    if ((policy != Use || resource->StillNeedsLoad()) && imageLoading == ImageLoading::Immediate)
    {
      resource->Load(*this);

      // We don't support immediate loads, but we do support immediate failure.
      if (resource->ErrorOccurred())
      {
        if (resource->AllowsCaching() && resource->InCache())
        {
          memoryCache.Remove(*resource);
        }

        auto resourceError = resource->GetResourceError();
        // Synchronous cancellations are likely due to access control.
        if (resourceError.IsNull() || resourceError.IsCancellation())
        {
          return Unexpected(ResourceError({}, 0, url, {}, ResourceError::Type::AccessControl));
        }
        return Unexpected(resourceError);
      }
    }

    krys_debug_assert(resource->URL() == url);
    // FIXME: Collect the document's resources when its loads are done (loadDone), once loads are counted.
    if (!_garbageCollectDocumentResourcesTimer.IsActive())
    {
      _garbageCollectDocumentResourcesTimer.StartOneShot({});
    }
    _documentResources[resource->URL().String()] = resource;
    return resource;
  }

  CachedResourceHandle<CachedResource>
    CachedResourceLoader::LoadResource(CachedResource::Type type, CachedResourceRequest &&request) noexcept
  {
    auto &memoryCache = MemoryCache::Singleton();
    krys_debug_assert(!request.AllowsCaching()
                      || !memoryCache.ResourceForRequest(request.GetResourceRequest()));

    auto resource = CreateResource(type, krys::move(request));

    if (resource->AllowsCaching())
    {
      (void)memoryCache.Add(*resource);
    }

    return resource;
  }

  CachedResourceLoader::RevalidationPolicy CachedResourceLoader::DetermineRevalidationPolicy(
    CachedResource::Type type, CachedResourceRequest &cachedResourceRequest, CachedResource *existingResource,
    ImageLoading imageLoading) const noexcept
  {
    if (!existingResource)
    {
      return Load;
    }

    // If the same URL has been loaded as a different type, we need to reload.
    if (existingResource->GetType() != type)
    {
      return Reload;
    }

    // Do not load from cache if images are not enabled. The load for this image will be blocked in
    // CachedImage::load.
    if (imageLoading == ImageLoading::DeferredUntilVisible)
    {
      return Reload;
    }

    // If credentials were sent with the previous request and won't be
    // with this one, or vice versa, re-fetch the resource.
    if (existingResource->Options().CredentialsMode != cachedResourceRequest.Options().CredentialsMode)
    {
      return Reload;
    }

    // We'll try to reload the resource if it failed last time.
    if (existingResource->ErrorOccurred())
    {
      return Reload;
    }

    if (existingResource->IsLoading())
    {
      // Do not use cached main resources that are still loading because sharing
      // loading CachedResources in this case causes issues with regards to cancellation.
      if (type == CachedResource::Type::MainResource)
      {
        return Reload;
      }
      // For cached subresources that are still loading we ignore the cache policy.
      return Use;
    }

    // FIXME: Revalidate expired resources (HTTP freshness, cache validators), once there are network loads.
    return Use;
  }

  void CachedResourceLoader::SetAutoLoadImages(bool enable) noexcept
  {
    if (enable == _autoLoadImages)
    {
      return;
    }

    _autoLoadImages = enable;

    if (!_autoLoadImages)
    {
      return;
    }

    ReloadImagesIfNotDeferred();
  }

  void CachedResourceLoader::SetImagesEnabled(bool enable) noexcept
  {
    if (enable == _imagesEnabled)
    {
      return;
    }

    _imagesEnabled = enable;

    if (!_imagesEnabled)
    {
      return;
    }

    ReloadImagesIfNotDeferred();
  }

  ImageLoading CachedResourceLoader::ClientDefersImage(const url::URL &) const noexcept
  {
    return _imagesEnabled ? ImageLoading::Immediate : ImageLoading::DeferredUntilVisible;
  }

  bool CachedResourceLoader::ShouldPerformImageLoad(const url::URL &url) const noexcept
  {
    return _autoLoadImages || url.ProtocolIsData();
  }

  bool CachedResourceLoader::ShouldDeferImageLoad(const url::URL &url) const noexcept
  {
    return ClientDefersImage(url) == ImageLoading::DeferredUntilVisible || !ShouldPerformImageLoad(url);
  }

  void CachedResourceLoader::ReloadImagesIfNotDeferred() noexcept
  {
    List<CachedResourceHandle<CachedResource>> resources;
    for (auto &[url, resource] : _documentResources)
    {
      resources.push_back(resource);
    }
    for (auto &resource : resources)
    {
      auto *image = DynamicDowncast<CachedImage>(*resource);
      if (image && resource->StillNeedsLoad()
          && ClientDefersImage(resource->URL()) == ImageLoading::Immediate)
      {
        image->Load(*this);
      }
    }
  }

  // Garbage collecting m_documentResources is a workaround for the
  // CachedResourceHandles on the RHS being strong references. Ideally this
  // would be a weak map, however CachedResourceHandles perform additional
  // bookkeeping on CachedResources, so instead pseudo-GC them -- when the
  // reference count reaches 1, m_documentResources is the only reference, so
  // remove it from the map.
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
      result.CredentialsMode = FetchOptions::Credentials::Include;
      result.RequestMode = FetchOptions::Mode::NoCors;
      result.ContentSecurityPolicy = ContentSecurityPolicyImposition::DoPolicyCheck;
      result.Caching = CachingPolicy::AllowCaching;
      return result;
    }();
    return options;
  }
}
