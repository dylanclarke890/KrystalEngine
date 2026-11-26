#include "Krystal.Web/Loader/Cache/MemoryCache.hpp"
#include "Krystal.Core/Memory/NeverDestroyed.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include "Krystal.Web/Graphics/Images/Image.hpp"
#include "Krystal.Web/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Web/Loader/Cache/CachedImage.hpp"
#include <algorithm>

namespace krys::web
{
  namespace
  {
    constexpr MonotonicTime::duration MinDelayBeforeLiveDecodedPrune = std::chrono::seconds(1);
    // Pruning goes a little under the capacity, so the cache isn't straight back over it.
    constexpr float TargetPrunePercentage = .95f;

    /// @brief The bytes the cache counts for @p resource: its data, encoded and decoded (not its overhead,
    /// which changes as its response arrives, so the totals stay balanced).
    KRYS_NODISCARD uint32 CountedSize(const CachedResource &resource) noexcept
    {
      return resource.EncodedSize() + resource.DecodedSize();
    }
  }

  MemoryCache &MemoryCache::Singleton() noexcept
  {
    static NeverDestroyed<MemoryCache> memoryCache;
    return memoryCache;
  }

  MemoryCache::MemoryCache() noexcept : _pruneTimer(*this, &MemoryCache::Prune)
  {
  }

  bool MemoryCache::ShouldRemoveFragmentIdentifier(const net::Url &originalUrl) noexcept
  {
    if (!originalUrl.HasFragmentIdentifier())
    {
      return false;
    }
    // Only HTTP(S) URLs share a resource whatever their fragment: a data URL is taken as it is, and other
    // schemes' fragments may name different resources.
    return originalUrl.ProtocolIsInHttpFamily();
  }

  net::Url MemoryCache::RemoveFragmentIdentifierIfNeeded(const net::Url &originalUrl) noexcept
  {
    if (!ShouldRemoveFragmentIdentifier(originalUrl))
    {
      return originalUrl;
    }
    net::Url url = originalUrl;
    url.RemoveFragmentIdentifier();
    return url;
  }

  bool MemoryCache::Add(CachedResource &resource) noexcept
  {
    if (Disabled())
    {
      return false;
    }

    const auto &key = resource.Url().String();
    krys_assert(!_resources.contains(key));
    _resources.emplace(key, ShareRef(resource));
    resource.SetInCache(true);
    AdjustSize(resource.HasClients(), static_cast<int64>(CountedSize(resource)));

    ResourceAccessed(resource);
    return true;
  }

  CachedResource *MemoryCache::ResourceForRequest(const net::ResourceRequest &request) noexcept
  {
    net::Url url = RemoveFragmentIdentifierIfNeeded(request.Url());
    auto found = _resources.find(url.String());
    return found != _resources.end() ? found->second.get() : nullptr;
  }

  uint32 MemoryCache::DeadCapacity() const noexcept
  {
    uint32 capacity = _capacity - std::min(_liveSize, _capacity);
    capacity = std::max(capacity, _minDeadCapacity);
    capacity = std::min(capacity, _maxDeadCapacity);
    return capacity;
  }

  uint32 MemoryCache::LiveCapacity() const noexcept
  {
    return _capacity - DeadCapacity();
  }

  void MemoryCache::PruneLiveResources(bool shouldDestroyDecodedDataForAllLiveResources) noexcept
  {
    uint32 capacity = shouldDestroyDecodedDataForAllLiveResources ? 0 : LiveCapacity();
    if (capacity && _liveSize <= capacity)
    {
      return;
    }

    auto targetSize = static_cast<uint32>(static_cast<float>(capacity) * TargetPrunePercentage);

    PruneLiveResourcesToSize(targetSize, shouldDestroyDecodedDataForAllLiveResources);
  }

  void MemoryCache::PruneLiveResourcesToSize(uint32 targetSize,
                                             bool shouldDestroyDecodedDataForAllLiveResources) noexcept
  {
    if (_inPruneResources)
    {
      return;
    }

    SetForScope reentrancyProtector(_inPruneResources, true);

    auto currentTime = html::EventLoop::Current().Now();

    // Least recently used first.
    for (auto *resource : List<CachedResource *>(_lruList))
    {
      CachedResourceHandle<CachedResource> current(resource);
      if (!current->HasClients() || current->IsLoading() || !current->DecodedSize())
      {
        continue;
      }

      // The rest were used more recently still.
      auto elapsedTime = currentTime - current->_lastDecodedAccessTime;
      if (!shouldDestroyDecodedDataForAllLiveResources && elapsedTime < MinDelayBeforeLiveDecodedPrune)
      {
        return;
      }

      current->DestroyDecodedData();

      if (targetSize && _liveSize <= targetSize)
      {
        return;
      }
    }
  }

  void MemoryCache::PruneDeadResources() noexcept
  {
    uint32 capacity = DeadCapacity();
    if (capacity && _deadSize <= capacity)
    {
      return;
    }

    auto targetSize = static_cast<uint32>(static_cast<float>(capacity) * TargetPrunePercentage);
    PruneDeadResourcesToSize(targetSize);
  }

  void MemoryCache::PruneDeadResourcesToSize(uint32 targetSize) noexcept
  {
    if (_inPruneResources)
    {
      return;
    }

    SetForScope reentrancyProtector(_inPruneResources, true);

    if (targetSize && _deadSize <= targetSize)
    {
      return;
    }

    // Dropping decoded data can change the list, so this goes through a copy that holds the resources.
    List<CachedResourceHandle<CachedResource>> lruList;
    lruList.reserve(_lruList.size());
    for (auto *resource : _lruList)
    {
      lruList.emplace_back(resource);
    }

    // Their decoded data goes first, least recently used first.
    for (auto &resource : lruList)
    {
      if (!resource->InCache())
      {
        continue;
      }

      if (!resource->HasClients() && !resource->IsLoading())
      {
        resource->DestroyDecodedData();

        if (targetSize && _deadSize <= targetSize)
        {
          return;
        }
      }
    }

    // Then the resources themselves.
    for (auto &resource : lruList)
    {
      if (!resource->InCache())
      {
        continue;
      }

      if (!resource->HasClients())
      {
        Remove(*resource);
        if (targetSize && _deadSize <= targetSize)
        {
          return;
        }
      }
    }
  }

  void MemoryCache::SetCapacities(uint32 minDeadBytes, uint32 maxDeadBytes, uint32 totalBytes) noexcept
  {
    krys_debug_assert(minDeadBytes <= maxDeadBytes);
    krys_debug_assert(maxDeadBytes <= totalBytes);
    _minDeadCapacity = minDeadBytes;
    _maxDeadCapacity = maxDeadBytes;
    _capacity = totalBytes;
    Prune();
  }

  void MemoryCache::Remove(CachedResource &resource) noexcept
  {
    CachedResourceHandle<CachedResource> protectedResource(resource);

    // A reload may have removed it already.
    if (!resource.InCache())
    {
      return;
    }

    _resources.erase(resource.Url().String());
    resource.SetInCache(false);

    RemoveFromLRUList(resource);
    AdjustSize(resource.HasClients(), -static_cast<int64>(CountedSize(resource)));
  }

  void MemoryCache::RemoveFromLRUList(CachedResource &resource) noexcept
  {
    std::erase(_lruList, &resource);
  }

  void MemoryCache::ResourceAccessed(CachedResource &resource) noexcept
  {
    krys_debug_assert(resource.InCache());

    RemoveFromLRUList(resource);

    _lruList.push_back(&resource);
  }

  void MemoryCache::AddToLiveResourcesSize(CachedResource &resource) noexcept
  {
    _liveSize += CountedSize(resource);
    _deadSize -= std::min(_deadSize, CountedSize(resource));
  }

  void MemoryCache::RemoveFromLiveResourcesSize(CachedResource &resource) noexcept
  {
    _liveSize -= std::min(_liveSize, CountedSize(resource));
    _deadSize += CountedSize(resource);
  }

  void MemoryCache::AdjustSize(bool live, int64 delta) noexcept
  {
    auto &size = live ? _liveSize : _deadSize;
    if (delta >= 0)
    {
      size += static_cast<uint32>(delta);
    }
    else
    {
      krys_debug_assert(static_cast<int64>(size) + delta >= 0);
      size -= std::min(size, static_cast<uint32>(-delta));
    }
  }

  void MemoryCache::SetDisabled(bool disabled) noexcept
  {
    _disabled = disabled;
    if (!_disabled)
    {
      return;
    }

    while (!_resources.empty())
    {
      Ref<CachedResource> resource = _resources.begin()->second;
      Remove(*resource);
    }
  }

  void MemoryCache::EvictResources() noexcept
  {
    if (Disabled())
    {
      return;
    }

    SetDisabled(true);
    SetDisabled(false);
  }

  bool MemoryCache::NeedsPruning() const noexcept
  {
    return _liveSize + _deadSize > _capacity || _deadSize > _maxDeadCapacity;
  }

  void MemoryCache::Prune() noexcept
  {
    if (!NeedsPruning())
    {
      return;
    }

    // Dead resources first, as they may be taking capacity that live ones need.
    PruneDeadResources();
    PruneLiveResources();
  }

  void MemoryCache::PruneSoon() noexcept
  {
    if (!NeedsPruning())
    {
      return;
    }
    if (_pruneTimer.IsActive())
    {
      return;
    }
    _pruneTimer.StartOneShot({});
  }
}
