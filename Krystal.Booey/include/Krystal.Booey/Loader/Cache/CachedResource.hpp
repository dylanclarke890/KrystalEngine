#pragma once

#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/Loader/LoaderStrategy.hpp"
#include "Krystal.Booey/Loader/ResourceLoaderOptions.hpp"
#include "Krystal.Booey/Platform/Network/ResourceError.hpp"
#include "Krystal.Booey/Platform/Network/ResourceRequest.hpp"
#include "Krystal.Booey/Platform/Network/ResourceResponse.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class CachedResourceClient;
  class CachedResourceLoader;
  class CachedResourceRequest;
  class MemoryCache;

  template <typename>
  class CachedResourceClientWalker;

  // A resource that is held in the cache. Classes who want to use this object should derive
  // from CachedResourceClient, to get the function calls in case the requested data has arrived.
  // This class also does the actual communication with the loader to obtain the resource from the network.
  /// @note Reference counted: handles (`CachedResourceHandle`) and the memory cache keep a resource alive.
  // FIXME: HTTP revalidation, preloads, cross-origin checks, priorities, redirects and cryptographic
  // digests, once there are network loads.
  class CachedResource : public RefCounted<CachedResource>, public CanMakeWeakPtr<CachedResource>
  {
    friend class MemoryCache;
    template <typename>
    friend class CachedResourceClientWalker;
    KRYS_NON_COPYABLE(CachedResource);

  public:
    enum class Type : uint8
    {
      MainResource,
      ImageResource,
      CSSStyleSheet,
      Script,
      FontResource,
      SVGFontResource,
      MediaResource,
      RawResource,
      Icon,
      SVGDocumentResource,
      LastType = SVGDocumentResource,
    };

    enum Status : uint8
    {
      Unknown, // let cache decide what to do with it
      Pending, // only partially loaded
      Cached,  // regular case
      LoadError,
      DecodeError
    };

  protected:
    ResourceLoaderOptions _options;
    ResourceRequest _resourceRequest;
    HashMap<CachedResourceClient *, uint32> _clients;
    RefPtr<SharedBuffer> _data;

  private:
    ResourceResponse _response;
    ResourceError _error;
    uint32 _encodedSize {0};
    uint32 _decodedSize {0};
    Timer _decodedDataDeletionTimer;
    MonotonicTime::duration _lastDecodedAccessTime {}; // Used as a "thrash guard" in the cache
    utf8_string _initiatorType;
    uint32 _accessCount {0};
    Type _type;
    Status _status {Pending};
    bool _inCache {false};
    bool _loading {false};
    List<Func<void()>> _loadedCallbacks;

  public:
    CachedResource(CachedResourceRequest &&, Type) noexcept;
    virtual ~CachedResource() noexcept;

    virtual void Load(CachedResourceLoader &) noexcept;

    virtual void UpdateBuffer(const SharedBuffer &) noexcept;
    virtual void FinishLoading(const SharedBuffer *) noexcept;
    virtual void Error(Status) noexcept;

    void SetResourceError(const ResourceError &error) noexcept
    {
      _error = error;
    }
    KRYS_NODISCARD const ResourceError &GetResourceError() const noexcept
    {
      return _error;
    }

    KRYS_NODISCARD const ResourceRequest &GetResourceRequest() const noexcept
    {
      return _resourceRequest;
    }
    KRYS_NODISCARD const url::URL &URL() const noexcept
    {
      return _resourceRequest.URL();
    }
    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD utf8_string MimeType() const noexcept
    {
      return Response().MimeType();
    }
    KRYS_NODISCARD int64 ExpectedContentLength() const noexcept
    {
      return Response().ExpectedContentLength();
    }

    void AddClient(CachedResourceClient &) noexcept;
    void RemoveClient(CachedResourceClient &) noexcept;
    KRYS_NODISCARD bool HasClients() const noexcept
    {
      return !_clients.empty();
    }
    KRYS_NODISCARD bool HasClient(const CachedResourceClient &client) const noexcept
    {
      return _clients.contains(const_cast<CachedResourceClient *>(&client));
    }

    virtual void DidAddClient(CachedResourceClient &) noexcept;
    virtual void DidRemoveClient(CachedResourceClient &) noexcept
    {
    }
    virtual void AllClientsRemoved() noexcept
    {
    }
    void DestroyDecodedDataIfNeeded() noexcept;

    KRYS_NODISCARD uint32 NumberOfClients() const noexcept
    {
      return static_cast<uint32>(_clients.size());
    }

    KRYS_NODISCARD Status GetStatus() const noexcept
    {
      return _status;
    }
    void SetStatus(Status status) noexcept
    {
      _status = status;
    }

    KRYS_NODISCARD uint32 Size() const noexcept
    {
      return EncodedSize() + DecodedSize() + OverheadSize();
    }
    KRYS_NODISCARD uint32 EncodedSize() const noexcept
    {
      return _encodedSize;
    }
    KRYS_NODISCARD uint32 DecodedSize() const noexcept
    {
      return _decodedSize;
    }
    KRYS_NODISCARD uint32 OverheadSize() const noexcept;

    // FIXME. Method name is inaccurate. Loading might not have started yet.
    KRYS_NODISCARD bool IsLoaded() const noexcept
    {
      return !_loading;
    }
    KRYS_NODISCARD bool IsLoading() const noexcept
    {
      return _loading;
    }
    void SetLoading(bool) noexcept;
    KRYS_NODISCARD virtual bool StillNeedsLoad() const noexcept
    {
      return false;
    }
    void WhenLoaded(Func<void()> &&) noexcept;

    KRYS_NODISCARD bool IsImage() const noexcept
    {
      return GetType() == Type::ImageResource;
    }

    KRYS_NODISCARD uint32 AccessCount() const noexcept
    {
      return _accessCount;
    }
    void IncreaseAccessCount() noexcept
    {
      _accessCount++;
    }

    // Computes the status of an object after loading.
    // Updates the expire date on the cache entry file
    void Finish() noexcept;

    // Called by the cache if the object has been removed from the cache
    // while still being referenced. This means the object should delete itself
    // if the number of clients observing it ever drops to 0.
    // The resource can be brought back to cache after successful revalidation.
    void SetInCache(bool inCache) noexcept
    {
      _inCache = inCache;
    }
    KRYS_NODISCARD bool InCache() const noexcept
    {
      return _inCache;
    }

    KRYS_NODISCARD SharedBuffer *ResourceBuffer() const noexcept
    {
      return _data.get();
    }

    virtual void ResponseReceived(ResourceResponse &&) noexcept;
    void SetResponse(ResourceResponse &&) noexcept;
    KRYS_NODISCARD const ResourceResponse &Response() const noexcept
    {
      return _response;
    }

    KRYS_NODISCARD const utf8_string &InitiatorType() const noexcept
    {
      return _initiatorType;
    }

    KRYS_NODISCARD bool ErrorOccurred() const noexcept
    {
      return _status == LoadError || _status == DecodeError;
    }
    KRYS_NODISCARD bool LoadFailedOrCanceled() const noexcept
    {
      return !_error.IsNull();
    }

    KRYS_NODISCARD DataBufferingPolicy GetDataBufferingPolicy() const noexcept
    {
      return _options.DataBuffering;
    }
    KRYS_NODISCARD bool AllowsCaching() const noexcept
    {
      return _options.Caching == CachingPolicy::AllowCaching
             || _options.Caching == CachingPolicy::AllowCachingMainResourcePrefetch;
    }
    KRYS_NODISCARD const ResourceLoaderOptions &Options() const noexcept
    {
      return _options;
    }

    virtual void DestroyDecodedData() noexcept
    {
    }

  protected:
    void SetEncodedSize(uint32) noexcept;
    void SetDecodedSize(uint32) noexcept;
    void DidAccessDecodedData(MonotonicTime::duration timeStamp) noexcept;

    ResourceResponse &MutableResponse() noexcept
    {
      return _response;
    }

    void StopDecodedDataDeletionTimer() noexcept;
    void RestartDecodedDataDeletionTimer() noexcept;

  private:
    virtual void CheckNotify() noexcept;
    void FailBeforeStarting() noexcept;
    void DecodedDataDeletionTimerFired() noexcept;

    /// @brief What upstream's subresource loader does when a load ends: hands the response and data to the
    /// resource, or its error.
    void DidFinishLoad(LoadResult &&) noexcept;
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_CACHED_RESOURCE(ToClassName, CachedResourceType)                         \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ToClassName)                                             \
    KRYS_NODISCARD static bool IsType(const krys::boo::CachedResource &resource) noexcept                    \
    {                                                                                                        \
      return resource.GetType() == krys::boo::CachedResourceType;                                            \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
