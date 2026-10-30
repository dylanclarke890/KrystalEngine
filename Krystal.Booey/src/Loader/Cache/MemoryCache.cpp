#include "Krystal.Booey/Loader/Cache/MemoryCache.hpp"
#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    constexpr uint32 DefaultCacheCapacity = 8'192 * 1'024;
    constexpr MonotonicTime::duration MinDelayBeforeLiveDecodedPrune = std::chrono::seconds(1);
    // Percentage of capacity toward which we prune, to avoid immediately pruning again.
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

  MemoryCache::MemoryCache() noexcept
      : _capacity(DefaultCacheCapacity), _maxDeadCapacity(DefaultCacheCapacity),
        _pruneTimer(*this, &MemoryCache::Prune)
  {
  }

  bool MemoryCache::ShouldRemoveFragmentIdentifier(const url::URL &originalURL) noexcept
  {
    if (!originalURL.HasFragmentIdentifier())
    {
      return false;
    }
    // Strip away fragment identifier from HTTP URLs.
    // Data URLs must be unmodified. For file and custom URLs clients may expect resources
    // to be unique even when they differ by the fragment identifier only.
    return originalURL.ProtocolIsInHTTPFamily();
  }

  url::URL MemoryCache::RemoveFragmentIdentifierIfNeeded(const url::URL &originalURL) noexcept
  {
    if (!ShouldRemoveFragmentIdentifier(originalURL))
    {
      return originalURL;
    }
    url::URL url = originalURL;
    url.RemoveFragmentIdentifier();
    return url;
  }

  bool MemoryCache::Add(CachedResource &resource) noexcept
  {
    if (Disabled())
    {
      return false;
    }

    const auto &key = resource.URL().String();
    krys_assert(!_resources.contains(key));
    _resources.emplace(key, ShareRef(resource));
    resource.SetInCache(true);
    AdjustSize(resource.HasClients(), static_cast<int64>(CountedSize(resource)));

    ResourceAccessed(resource);
    return true;
  }

  CachedResource *MemoryCache::ResourceForRequest(const ResourceRequest &request) noexcept
  {
    url::URL url = RemoveFragmentIdentifierIfNeeded(request.URL());
    auto found = _resources.find(url.String());
    return found != _resources.end() ? found->second.get() : nullptr;
  }

  uint32 MemoryCache::DeadCapacity() const noexcept
  {
    // Dead resource capacity is whatever space is not occupied by live resources, bounded by an independent
    // minimum and maximum.
    uint32 capacity = _capacity - std::min(_liveSize, _capacity); // Start with available capacity.
    capacity = std::max(capacity, _minDeadCapacity);              // Make sure it's above the minimum.
    capacity = std::min(capacity, _maxDeadCapacity);              // Make sure it's below the maximum.
    return capacity;
  }

  uint32 MemoryCache::LiveCapacity() const noexcept
  {
    // Live resource capacity is whatever is left over after calculating dead resource capacity.
    return _capacity - DeadCapacity();
  }

  void MemoryCache::PruneLiveResources(bool shouldDestroyDecodedDataForAllLiveResources) noexcept
  {
    uint32 capacity = shouldDestroyDecodedDataForAllLiveResources ? 0 : LiveCapacity();
    if (capacity && _liveSize <= capacity)
    {
      return;
    }

    // Cut by a percentage to avoid immediately pruning again.
    auto targetSize = static_cast<uint32>(static_cast<float>(capacity) * TargetPrunePercentage);

    PruneLiveResourcesToSize(targetSize, shouldDestroyDecodedDataForAllLiveResources);
  }

  void MemoryCache::ForEachResource(KRYS_NOESCAPE const Func<void(CachedResource &)> &function) noexcept
  {
    List<Ref<CachedResource>> resources;
    resources.reserve(_resources.size());
    for (const auto &[url, resource] : _resources)
    {
      resources.push_back(resource);
    }
    for (auto &resource : resources)
    {
      function(*resource);
    }
  }

  void MemoryCache::DestroyDecodedDataForAllImages() noexcept
  {
    ForEachResource(
      [](CachedResource &resource)
      {
        if (auto *cachedImage = DynamicDowncast<CachedImage>(resource))
        {
          if (auto *image = cachedImage->GetImage())
          {
            image->DestroyDecodedData();
          }
        }
      });
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

    // Destroy any decoded data in live objects that we can.
    // Start from the head, since this is the least recently accessed of the objects.
    for (auto *resource : List<CachedResource *>(_lruList))
    {
      CachedResourceHandle<CachedResource> current(resource);
      if (!current->HasClients() || !current->IsLoaded() || !current->DecodedSize())
      {
        continue;
      }

      // Check to see if the remaining resources are too new to prune.
      auto elapsedTime = currentTime - current->_lastDecodedAccessTime;
      if (!shouldDestroyDecodedDataForAllLiveResources && elapsedTime < MinDelayBeforeLiveDecodedPrune)
      {
        return;
      }

      // Destroy our decoded data.
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

    // Cut by a percentage to avoid immediately pruning again.
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

    // Make a copy of the list first (and ref the resources) as calling destroyDecodedData() can alter it.
    List<CachedResourceHandle<CachedResource>> lruList;
    lruList.reserve(_lruList.size());
    for (auto *resource : _lruList)
    {
      lruList.emplace_back(resource);
    }

    // First flush all the decoded data in this queue.
    // Remove from the head, since this is the least frequently accessed of the objects.
    for (auto &resource : lruList)
    {
      if (!resource->InCache())
      {
        continue;
      }

      if (!resource->HasClients() && resource->IsLoaded())
      {
        resource->DestroyDecodedData();

        if (targetSize && _deadSize <= targetSize)
        {
          return;
        }
      }
    }

    // Now evict objects from this list.
    // Remove from the head, since this is the least frequently accessed of the objects.
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

    // The resource may have already been removed by someone other than our caller,
    // who needed a fresh copy for a reload. See <http://bugs.webkit.org/show_bug.cgi?id=12479#c6>.
    if (!resource.InCache())
    {
      return;
    }

    // Remove resource from the resource map.
    _resources.erase(resource.URL().String());
    resource.SetInCache(false);

    // Remove from the LRU list.
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

    // Need to make sure to remove before we increase the access count, since
    // the queue will possibly change.
    RemoveFromLRUList(resource);

    // Add to our access count.
    resource.IncreaseAccessCount();

    // Now insert into the new queue.
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

    PruneDeadResources(); // Prune dead first, in case it was "borrowing" capacity from live.
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
