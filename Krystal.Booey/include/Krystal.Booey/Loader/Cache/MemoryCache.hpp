#pragma once

#include "Krystal.Booey/Loader/Cache/CachedResource.hpp"
#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/NeverDestroyed.hpp"
#include "Krystal.Core/Time/Timer.hpp"

namespace krys::boo
{
  /// @brief The process-wide cache of the resources documents load (images, style sheets, fonts, SVG
  /// documents), by URL, so documents share them and don't load them again.
  ///
  /// A resource with clients is live; one without is dead. The capacity is shared: dead resources get what
  /// live ones leave, kept between a minimum and a maximum. Pruning drops the decoded data of the least
  /// recently used resources, live ones too, and evicts dead ones, until the sizes fit.
  /// @note Resources are kept in one list, least recently accessed first, rather than in lists by size and
  /// access count; live resources (those with clients) aren't kept in a list of their own.
  class MemoryCache final
  {
    friend class NeverDestroyed<MemoryCache>;
    KRYS_NON_COPY_MOVABLE(MemoryCache);

  private:
    constexpr static uint32 DefaultCacheCapacity = 8'192 * 1'024;

    bool _disabled {false};
    bool _inPruneResources {false};
    uint32 _capacity {DefaultCacheCapacity};
    uint32 _minDeadCapacity {0};
    uint32 _maxDeadCapacity {DefaultCacheCapacity};
    MonotonicTime::duration _deadDecodedDataDeletionInterval {};

    // The bytes the cache's live and dead resources take.
    uint32 _liveSize {0};
    uint32 _deadSize {0};

    // The cached resources by URL (without an HTTP URL's fragment).
    HashMap<utf8_string, Ref<CachedResource>> _resources;
    // The resources in the cache, least recently accessed first.
    List<CachedResource *> _lruList;

    Timer _pruneTimer;

  public:
    /// @brief The cache.
    KRYS_NODISCARD static MemoryCache &Singleton() noexcept;

    /// @brief The cached resource for the request's URL, if there is one.
    KRYS_NODISCARD CachedResource *ResourceForRequest(const net::ResourceRequest &) noexcept;

    /// @brief Adds a resource (unless the cache is off) or removes one.
    /// @return Whether it was added.
    bool Add(CachedResource &) noexcept;

    void Remove(CachedResource &) noexcept;

    /// @brief Whether the cache ignores the URL's fragment, as it does an HTTP(S) URL's, and the URL without
    /// it.
    KRYS_NODISCARD static bool ShouldRemoveFragmentIdentifier(const net::Url &) noexcept;

    KRYS_NODISCARD static net::Url RemoveFragmentIdentifierIfNeeded(const net::Url &) noexcept;

    /// @brief Sets the capacities in bytes, and prunes to them. They're approximate: decoded data is sized by
    /// estimate.
    /// @param minDeadBytes What dead resources may keep however full the cache is with live ones.
    /// @param maxDeadBytes What dead resources may take at most.
    /// @param totalBytes What the cache may take altogether.
    void SetCapacities(uint32 minDeadBytes, uint32 maxDeadBytes, uint32 totalBytes) noexcept;

    /// @brief Turns the cache off (evicting everything; resources in use live on with their users) or on.
    void SetDisabled(bool) noexcept;

    /// @brief Whether the cache is off.
    KRYS_NODISCARD bool Disabled() const noexcept
    {
      return _disabled;
    }

    /// @brief Evicts every resource.
    void EvictResources() noexcept;

    /// @brief Prunes if the cache is over capacity: now, or in a task (once for any number of calls).
    void Prune() noexcept;

    void PruneSoon() noexcept;

    /// @brief The bytes the resources take, and the dead ones.
    KRYS_NODISCARD uint32 Size() const noexcept
    {
      return _liveSize + _deadSize;
    }

    KRYS_NODISCARD uint32 DeadSize() const noexcept
    {
      return _deadSize;
    }

    /// @brief How long a resource without clients keeps its decoded data before dropping it; zero (the
    /// default) keeps it until pruning.
    void SetDeadDecodedDataDeletionInterval(MonotonicTime::duration interval) noexcept
    {
      _deadDecodedDataDeletionInterval = interval;
    }

    KRYS_NODISCARD MonotonicTime::duration DeadDecodedDataDeletionInterval() const noexcept
    {
      return _deadDecodedDataDeletionInterval;
    }

    /// @brief Adds @p delta to the live or dead total, as a resource's size changes.
    void AdjustSize(bool live, int64 delta) noexcept;

    /// @brief Moves a resource's size between the live and dead totals, as it gains its first client or loses
    /// its last.
    void AddToLiveResourcesSize(CachedResource &) noexcept;

    void RemoveFromLiveResourcesSize(CachedResource &) noexcept;

    /// @brief Makes the resource the most recently used.
    void ResourceAccessed(CachedResource &) noexcept;

    /// @brief Prunes dead resources (their decoded data, then the resources) or live ones (their decoded data
    /// only, if it hasn't been used for a second) to a little under their capacity, or to @p targetSize; 0
    /// prunes all it can.
    void PruneDeadResources() noexcept;

    void PruneLiveResources(bool shouldDestroyDecodedDataForAllLiveResources = false) noexcept;

    void PruneDeadResourcesToSize(uint32 targetSize) noexcept;

    void PruneLiveResourcesToSize(uint32 targetSize,
                                  bool shouldDestroyDecodedDataForAllLiveResources = false) noexcept;

  private:
    MemoryCache() noexcept;

    /// @brief The capacity for live resources: what the dead resources' capacity leaves.
    KRYS_NODISCARD uint32 LiveCapacity() const noexcept;

    /// @brief The capacity for dead resources: what live ones leave, between the minimum and the maximum.
    KRYS_NODISCARD uint32 DeadCapacity() const noexcept;

    /// @brief Whether the cache is over its capacity, or its dead resources over their maximum.
    KRYS_NODISCARD bool NeedsPruning() const noexcept;

    /// @brief Takes the resource out of the use order.
    void RemoveFromLRUList(CachedResource &) noexcept;
  };
}
