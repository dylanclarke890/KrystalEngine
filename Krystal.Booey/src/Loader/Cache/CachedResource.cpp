#include "Krystal.Booey/Loader/Cache/CachedResource.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceClient.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceClientWalker.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Booey/Loader/Cache/MemoryCache.hpp"
#include "Krystal.Booey/Platform/Network/DataURLDecoder.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"

namespace krys::boo
{
  CachedResource::CachedResource(CachedResourceRequest &&request, Type type) noexcept
      : _options(request.Options()),
        _decodedDataDeletionTimer(*this, &CachedResource::DecodedDataDeletionTimerFired),
        _initiatorType(request.InitiatorType()), _type(type)
  {
    _resourceRequest = request.ReleaseResourceRequest();
  }

  CachedResource::~CachedResource() noexcept
  {
    krys_debug_assert(!HasClients());
    krys_debug_assert(!InCache());

    auto callbacks = std::exchange(_loadedCallbacks, {});
    for (auto &callback : callbacks)
    {
      callback();
    }
  }

  void CachedResource::FailBeforeStarting() noexcept
  {
    Error(CachedResource::LoadError);
  }

  void CachedResource::Load(CachedResourceLoader &cachedResourceLoader) noexcept
  {
    auto *frame = cachedResourceLoader.Frame();
    if (!frame)
    {
      FailBeforeStarting();
      return;
    }

    _loading = true;
    SetStatus(Pending);

    // The load delays its document's load event until it's done.
    cachedResourceLoader.IncrementRequestCount(*this);
    _requestCountingLoader = CreateWeakPtr(&cachedResourceLoader);

    auto completeAsNetworkingTask =
      [protectedThis = CachedResourceHandle<CachedResource>(*this)](LoadResult &&result)
    {
      html::EventLoop::Current().QueueTask(html::TaskSource::Networking,
                                           [protectedThis, result = krys::move(result)]() mutable
                                           { protectedThis->DidFinishLoad(krys::move(result)); });
    };

    // Data URLs are decoded here, rather than by the host.
    if (URL().ProtocolIsData())
    {
      auto decoded = data_url_decoder::Decode(URL());
      if (!decoded)
      {
        completeAsNetworkingTask(
          Unexpected(ResourceError(utf8_string(ErrorDomainBooeyInternal), 0, URL(), u8"Invalid data URL")));
        return;
      }
      auto size = static_cast<int64>(decoded->Data.size());
      completeAsNetworkingTask(
        LoadedResource {.Response = ResourceResponse(URL(), krys::move(decoded->MimeType), size,
                                                     krys::move(decoded->Charset)),
                        .Data = SharedBuffer::Create(krys::move(decoded->Data))});
      return;
    }

    auto *loaderStrategy = PlatformStrategies::Singleton().GetLoaderStrategy();
    if (!loaderStrategy)
    {
      SetResourceError(
        ResourceError(utf8_string(ErrorDomainBooeyInternal), 0, URL(), u8"No loader strategy"));
      FailBeforeStarting();
      RequestDone();
      return;
    }

    loaderStrategy->LoadResource(*frame, _resourceRequest, _options, krys::move(completeAsNetworkingTask));
  }

  void CachedResource::DidFinishLoad(LoadResult &&result) noexcept
  {
    // The resource's clients hear of it before its document's loads are checked.
    CachedResourceHandle<CachedResource> protectedThis(*this);
    ApplyLoadResult(krys::move(result));
    RequestDone();
  }

  void CachedResource::RequestDone() noexcept
  {
    RefPtr<CachedResourceLoader> loader = ShareRefPtr(std::exchange(_requestCountingLoader, nullptr).get());
    if (!loader)
    {
      return;
    }
    loader->DecrementRequestCount(*this);
    loader->LoadDone();
  }

  void CachedResource::ApplyLoadResult(LoadResult &&result) noexcept
  {
    if (!IsLoading())
    {
      return;
    }

    if (!result)
    {
      SetResourceError(result.error());
      Error(LoadError);
      return;
    }

    ResponseReceived(krys::move(result->Response));
    FinishLoading(result->Data.get());
    if (ErrorOccurred())
    {
      return;
    }
    Finish();
  }

  void CachedResource::CheckNotify() noexcept
  {
    if (IsLoading() || StillNeedsLoad())
    {
      return;
    }

    CachedResourceClientWalker<CachedResourceClient> walker(*this);
    while (auto *client = walker.Next())
    {
      client->NotifyFinished(*this);
    }
  }

  void CachedResource::UpdateBuffer(const SharedBuffer &) noexcept
  {
    krys_debug_assert(GetDataBufferingPolicy() == DataBufferingPolicy::BufferData);
  }

  void CachedResource::FinishLoading(const SharedBuffer *) noexcept
  {
    SetLoading(false);
    CheckNotify();
  }

  void CachedResource::Error(CachedResource::Status status) noexcept
  {
    SetStatus(status);
    krys_debug_assert(ErrorOccurred());
    _data = nullptr;

    SetLoading(false);
    CheckNotify();
  }

  void CachedResource::Finish() noexcept
  {
    if (!ErrorOccurred())
    {
      SetStatus(Cached);
    }
  }

  void CachedResource::SetLoading(bool b) noexcept
  {
    _loading = b;
    if (_loading)
    {
      return;
    }
    auto callbacks = std::exchange(_loadedCallbacks, {});
    for (auto &callback : callbacks)
    {
      callback();
    }
  }

  void CachedResource::WhenLoaded(Func<void()> &&callback) noexcept
  {
    krys_debug_assert(_loading);
    _loadedCallbacks.push_back(krys::move(callback));
  }

  void CachedResource::SetResponse(ResourceResponse &&newResponse) noexcept
  {
    _response = krys::move(newResponse);
  }

  void CachedResource::ResponseReceived(ResourceResponse &&response) noexcept
  {
    SetResponse(krys::move(response));
  }

  void CachedResource::AddClient(CachedResourceClient &client) noexcept
  {
    if (AllowsCaching() && !HasClients() && InCache())
    {
      MemoryCache::Singleton().AddToLiveResourcesSize(*this);
    }

    ++_clients[&client];
    DidAddClient(client);
  }

  void CachedResource::DidAddClient(CachedResourceClient &client) noexcept
  {
    StopDecodedDataDeletionTimer();

    // FIXME: Make calls to notifyFinished async
    if (!IsLoading() && !StillNeedsLoad())
    {
      client.NotifyFinished(*this);
    }
  }

  void CachedResource::RemoveClient(CachedResourceClient &client) noexcept
  {
    auto found = _clients.find(&client);
    krys_debug_assert(found != _clients.end());
    if (found == _clients.end())
    {
      return;
    }
    if (!--found->second)
    {
      _clients.erase(found);
    }
    DidRemoveClient(client);

    if (HasClients())
    {
      return;
    }

    // Removing the last client may leave nothing else holding this resource but the caller.
    CachedResourceHandle<CachedResource> protectedThis(*this);

    auto &memoryCache = MemoryCache::Singleton();
    if (AllowsCaching() && InCache())
    {
      memoryCache.RemoveFromLiveResourcesSize(*this);
    }

    AllClientsRemoved();
    DestroyDecodedDataIfNeeded();

    if (!AllowsCaching())
    {
      return;
    }

    memoryCache.PruneSoon();
  }

  void CachedResource::DestroyDecodedDataIfNeeded() noexcept
  {
    if (!DecodedSize())
    {
      return;
    }
    if (MemoryCache::Singleton().DeadDecodedDataDeletionInterval() == MonotonicTime::duration::zero())
    {
      return;
    }
    RestartDecodedDataDeletionTimer();
  }

  void CachedResource::DecodedDataDeletionTimerFired() noexcept
  {
    DestroyDecodedData();
  }

  void CachedResource::SetDecodedSize(uint32 size) noexcept
  {
    if (size == DecodedSize())
    {
      return;
    }

    int64 delta = static_cast<int64>(size) - DecodedSize();
    _decodedSize = size;

    if (AllowsCaching() && InCache())
    {
      // Update the cache's size totals.
      MemoryCache::Singleton().AdjustSize(HasClients(), delta);
    }
  }

  void CachedResource::SetEncodedSize(uint32 size) noexcept
  {
    if (size == EncodedSize())
    {
      return;
    }

    int64 delta = static_cast<int64>(size) - EncodedSize();
    _encodedSize = size;

    if (AllowsCaching() && InCache())
    {
      MemoryCache::Singleton().AdjustSize(HasClients(), delta);
    }
  }

  void CachedResource::DidAccessDecodedData(MonotonicTime::duration timeStamp) noexcept
  {
    _lastDecodedAccessTime = timeStamp;

    if (AllowsCaching() && InCache())
    {
      auto &memoryCache = MemoryCache::Singleton();
      memoryCache.ResourceAccessed(*this);
      memoryCache.PruneSoon();
    }
  }

  uint32 CachedResource::OverheadSize() const noexcept
  {
    static const uint32 kAverageClientsHashMapSize = 384;
    return static_cast<uint32>(sizeof(CachedResource) + _response.MimeType().size()
                               + kAverageClientsHashMapSize + _resourceRequest.URL().String().size() * 2);
  }

  void CachedResource::StopDecodedDataDeletionTimer() noexcept
  {
    _decodedDataDeletionTimer.Stop();
  }

  void CachedResource::RestartDecodedDataDeletionTimer() noexcept
  {
    _decodedDataDeletionTimer.StartOneShot(MemoryCache::Singleton().DeadDecodedDataDeletionInterval());
  }
}
