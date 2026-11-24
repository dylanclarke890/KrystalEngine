#pragma once

#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/Loader/LoaderStrategy.hpp"
#include "Krystal.Booey/Loader/ResourceLoaderOptions.hpp"
#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Time/Timer.hpp"
#include "Krystal.Core/Types/SharedBuffer.hpp"
#include "Krystal.Net/ResourceError.hpp"
#include "Krystal.Net/ResourceRequest.hpp"
#include "Krystal.Net/ResourceResponse.hpp"

namespace krys::boo
{
  class CachedResourceClient;
  class CachedResourceLoader;
  class CachedResourceRequest;
  class MemoryCache;

  template <typename>
  class CachedResourceClientWalker;

  /// @brief A loaded (or loading) resource, shared through the memory cache by the documents that use it.
  /// It loads itself through the host's loader strategy (or decodes a data URL), and tells its clients
  /// (`CachedResourceClient`) when it's done.
  /// @note Reference counted: handles (`CachedResourceHandle`) and the memory cache keep a resource alive.
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
      MediaResource,
      RawResource,
      Icon,
      SVGDocumentResource,
      LastType = SVGDocumentResource,
    };

    enum Status : uint8
    {
      Unknown, // Not loading yet (an image waits to be asked for).
      Pending, // Loading.
      Cached,  // Loaded.
      LoadError,
      DecodeError,
    };

  protected:
    ResourceLoaderOptions _options;
    net::ResourceRequest _resourceRequest;
    HashMap<CachedResourceClient *, uint32> _clients;
    RefPtr<SharedBuffer> _data;

  private:
    net::ResourceResponse _response;
    net::ResourceError _error;
    uint32 _encodedSize {0};
    uint32 _decodedSize {0};
    Timer _decodedDataDeletionTimer;
    // When the decoded data was last used: pruning leaves recently used data alone.
    MonotonicTime::duration _lastDecodedAccessTime {};
    utf8_string _initiatorType;
    Type _type;
    Status _status {Pending};
    bool _inCache {false};
    bool _loading {false};
    /// @brief The loader whose request count the load is in, until it's done.
    WeakPtr<CachedResourceLoader> _requestCountingLoader;

  public:
    CachedResource(CachedResourceRequest, Type) noexcept;

    virtual ~CachedResource() noexcept;

    /// @brief Starts loading for @p loader's document, which waits for it: a data URL is decoded here, and
    /// anything else goes to the host's loader strategy. The result comes back as a networking task.
    virtual void Load(CachedResourceLoader &) noexcept;

    /// @brief Takes the data received so far.
    virtual void UpdateBuffer(const SharedBuffer &) noexcept;

    /// @brief Takes the loaded data (null if there's none), ends the load and tells the clients.
    virtual void FinishLoading(const SharedBuffer *) noexcept;

    /// @brief Ends the load with an error status, dropping the data, and tells the clients.
    virtual void Error(Status) noexcept;

    /// @brief The network error the load failed with, if it did.
    void SetResourceError(const net::ResourceError &error) noexcept
    {
      _error = error;
    }

    KRYS_NODISCARD const net::ResourceError &GetResourceError() const noexcept
    {
      return _error;
    }

    /// @brief The request, and its URL.
    KRYS_NODISCARD const net::ResourceRequest &GetResourceRequest() const noexcept
    {
      return _resourceRequest;
    }

    KRYS_NODISCARD const net::Url &Url() const noexcept
    {
      return _resourceRequest.Url();
    }

    /// @brief What kind of resource it is.
    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    /// @brief The response's MIME type and expected length (-1 if unknown).
    KRYS_NODISCARD utf8_string MimeType() const noexcept
    {
      return Response().MimeType();
    }

    KRYS_NODISCARD int64 ExpectedContentLength() const noexcept
    {
      return Response().ExpectedContentLength();
    }

    /// @brief Adds or removes a client (a client can be added more than once, and is removed as often).
    /// A resource without clients is dead: its decoded data can go, and the cache may evict it.
    void AddClient(CachedResourceClient &) noexcept;

    void RemoveClient(CachedResourceClient &) noexcept;

    /// @brief Whether the resource has any clients, or @p client.
    KRYS_NODISCARD bool HasClients() const noexcept
    {
      return !_clients.empty();
    }

    KRYS_NODISCARD bool HasClient(const CachedResourceClient &client) const noexcept
    {
      return _clients.contains(const_cast<CachedResourceClient *>(&client));
    }

    /// @brief Called after a client is added: one added once the load is over is told so at once.
    virtual void DidAddClient(CachedResourceClient &) noexcept;

    /// @brief Called after a client is removed, and after the last one is.
    virtual void DidRemoveClient(CachedResourceClient &) noexcept
    {
    }

    virtual void AllClientsRemoved() noexcept
    {
    }

    /// @brief Starts the timer that drops the decoded data of a dead resource, if the memory cache sets one
    /// (`MemoryCache::SetDeadDecodedDataDeletionInterval`).
    void DestroyDecodedDataIfNeeded() noexcept;

    /// @brief How many distinct clients the resource has.
    KRYS_NODISCARD uint32 NumberOfClients() const noexcept
    {
      return static_cast<uint32>(_clients.size());
    }

    /// @brief The load's status.
    KRYS_NODISCARD Status GetStatus() const noexcept
    {
      return _status;
    }

    void SetStatus(Status status) noexcept
    {
      _status = status;
    }

    /// @brief The memory the resource is counted as using in the cache: its encoded data, its decoded data
    /// and an estimate of its own overhead.
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

    /// @brief Whether the resource is loading (false both before its load starts and after it ends).
    KRYS_NODISCARD bool IsLoading() const noexcept
    {
      return _loading;
    }

    void SetLoading(bool) noexcept;

    /// @brief Whether the load hasn't started yet (an image waiting to be asked for).
    KRYS_NODISCARD virtual bool StillNeedsLoad() const noexcept
    {
      return false;
    }

    /// @brief Whether it's an image.
    KRYS_NODISCARD bool IsImage() const noexcept
    {
      return GetType() == Type::ImageResource;
    }

    /// @brief Marks a load that ended without an error as cached.
    void Finish() noexcept;

    /// @brief Records whether the memory cache holds the resource; one it doesn't lives only as long as its
    /// handles.
    void SetInCache(bool inCache) noexcept
    {
      _inCache = inCache;
    }

    /// @brief Whether the memory cache holds the resource.
    KRYS_NODISCARD bool InCache() const noexcept
    {
      return _inCache;
    }

    /// @brief The loaded data, if it's kept.
    KRYS_NODISCARD SharedBuffer *ResourceBuffer() const noexcept
    {
      return _data.get();
    }

    /// @brief Takes the load's response (which a type may inspect), and its setter.
    virtual void ResponseReceived(net::ResourceResponse &&) noexcept;

    void SetResponse(net::ResourceResponse &&) noexcept;

    /// @brief The load's response.
    KRYS_NODISCARD const net::ResourceResponse &Response() const noexcept
    {
      return _response;
    }

    /// @brief What asked for the resource, as Resource Timing reports it: an element's local name, or `css`.
    KRYS_NODISCARD const utf8_string &InitiatorType() const noexcept
    {
      return _initiatorType;
    }

    /// @brief Whether the load or the decoding failed.
    KRYS_NODISCARD bool ErrorOccurred() const noexcept
    {
      return _status == LoadError || _status == DecodeError;
    }

    /// @brief Whether the load failed with a network error.
    KRYS_NODISCARD bool LoadFailedOrCanceled() const noexcept
    {
      return !_error.IsNull();
    }

    /// @brief Whether the data is kept as it arrives.
    KRYS_NODISCARD DataBufferingPolicy GetDataBufferingPolicy() const noexcept
    {
      return _options.DataBuffering;
    }

    /// @brief Whether the resource may go in the memory cache, by its load options.
    KRYS_NODISCARD bool AllowsCaching() const noexcept
    {
      return _options.Caching == CachingPolicy::AllowCaching
             || _options.Caching == CachingPolicy::AllowCachingMainResourcePrefetch;
    }

    /// @brief The load options.
    KRYS_NODISCARD const ResourceLoaderOptions &Options() const noexcept
    {
      return _options;
    }

    /// @brief Drops the decoded data (an image's bitmaps, a style sheet's parsed rules), keeping what it's
    /// decoded from.
    virtual void DestroyDecodedData() noexcept
    {
    }

  protected:
    /// @brief Sets the size of the encoded or decoded data, updating the memory cache's totals.
    void SetEncodedSize(uint32) noexcept;

    void SetDecodedSize(uint32) noexcept;

    /// @brief Records that the decoded data was used, which makes it the cache's most recently used.
    void DidAccessDecodedData(MonotonicTime::duration timeStamp) noexcept;

    /// @brief Stops or restarts the timer that drops a dead resource's decoded data.
    void StopDecodedDataDeletionTimer() noexcept;

    void RestartDecodedDataDeletionTimer() noexcept;

  private:
    /// @brief Tells the clients the load is over, if it is.
    virtual void CheckNotify() noexcept;

    /// @brief Fails a load that couldn't start.
    void FailBeforeStarting() noexcept;

    /// @brief Drops the decoded data, as its timer fired.
    void DecodedDataDeletionTimerFired() noexcept;

    /// @brief What upstream's subresource loader does when a load ends: hands the response and data to the
    /// resource, or its error.
    void DidFinishLoad(LoadResult) noexcept;

    /// @brief Ends the load with its result: the response and data, or the error.
    void ApplyLoadResult(LoadResult) noexcept;

    /// @brief The load is done: it leaves its loader's request count, which may let the document finish
    /// loading.
    void RequestDone() noexcept;
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_CACHED_RESOURCE(ToClassName, CachedResourceType)                         \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ToClassName)                                             \
    KRYS_NODISCARD static bool IsType(const krys::boo::CachedResource &resource) noexcept                    \
    {                                                                                                        \
      return resource.GetType() == krys::boo::CachedResourceType;                                            \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
