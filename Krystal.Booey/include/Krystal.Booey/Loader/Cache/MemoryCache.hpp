#pragma once

#include "Krystal.Booey/Loader/Cache/CachedResource.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  // This cache holds subresources used by Web pages: images, scripts, stylesheets, etc.
  // The cache keeps a flexible but bounded window of dead resources that grows/shrinks
  // depending on the live resource load. Here's an example of cache growth over time,
  // with a min dead resource capacity of 25% and a max dead resource capacity of 50%:

  // |-----|                              Dead: -
  // |----------|                         Live: +
  // --|----------|                         Cache boundary: | (objects outside this mark have been evicted)
  // --|----------++++++++++|
  // -------|-----+++++++++++++++|
  // -------|-----+++++++++++++++|+++++
  /// @note Resources are kept in one list, least recently accessed first, rather than in lists by size and
  /// access count; live resources (those with clients) aren't kept in a list of their own.
  // FIXME: Sessions, cache partitions, origins and revalidation, once there are network loads.
  class MemoryCache final
  {
    KRYS_NON_COPY_MOVABLE(MemoryCache);

  private:
    bool _disabled {false};
    bool _inPruneResources {false};
    uint32 _capacity;
    uint32 _minDeadCapacity {0};
    uint32 _maxDeadCapacity;
    MonotonicTime::duration _deadDecodedDataDeletionInterval {};

    uint32 _liveSize {0}; // The number of bytes currently consumed by "live" resources in the cache.
    uint32 _deadSize {0}; // The number of bytes currently consumed by "dead" resources in the cache.

    // A URL-based map of all resources that are in the cache (including the freshest version of objects that
    // are currently being referenced by a Web page).
    HashMap<utf8_string, Ref<CachedResource>> _resources;
    // The resources in the cache, least recently accessed first.
    List<CachedResource *> _lruList;

    Timer _pruneTimer;

  public:
    KRYS_NODISCARD static MemoryCache &Singleton() noexcept;

    KRYS_NODISCARD CachedResource *ResourceForRequest(const ResourceRequest &) noexcept;

    bool Add(CachedResource &) noexcept;
    void Remove(CachedResource &) noexcept;

    KRYS_NODISCARD static bool ShouldRemoveFragmentIdentifier(const url::URL &) noexcept;
    KRYS_NODISCARD static url::URL RemoveFragmentIdentifierIfNeeded(const url::URL &) noexcept;

    void ForEachResource(KRYS_NOESCAPE const Func<void(CachedResource &)> &) noexcept;

    void DestroyDecodedDataForAllImages() noexcept;

    // Sets the cache's memory capacities, in bytes. These will hold only approximately,
    // since the decoded cost of resources like scripts and stylesheets is not known.
    //  - minDeadBytes: The maximum number of bytes that dead resources should consume when the cache is under
    //  pressure.
    //  - maxDeadBytes: The maximum number of bytes that dead resources should consume when the cache is not
    //  under pressure.
    //  - totalBytes: The maximum number of bytes that the cache should consume overall.
    void SetCapacities(uint32 minDeadBytes, uint32 maxDeadBytes, uint32 totalBytes) noexcept;

    // Turn the cache on and off.  Disabling the cache will remove all resources from the cache.  They may
    // still live on if they are referenced by some Web page though.
    void SetDisabled(bool) noexcept;
    KRYS_NODISCARD bool Disabled() const noexcept
    {
      return _disabled;
    }

    void EvictResources() noexcept;

    void Prune() noexcept;
    void PruneSoon() noexcept;
    KRYS_NODISCARD uint32 Size() const noexcept
    {
      return _liveSize + _deadSize;
    }
    KRYS_NODISCARD uint32 LiveSize() const noexcept
    {
      return _liveSize;
    }
    KRYS_NODISCARD uint32 DeadSize() const noexcept
    {
      return _deadSize;
    }

    void SetDeadDecodedDataDeletionInterval(MonotonicTime::duration interval) noexcept
    {
      _deadDecodedDataDeletionInterval = interval;
    }
    KRYS_NODISCARD MonotonicTime::duration DeadDecodedDataDeletionInterval() const noexcept
    {
      return _deadDecodedDataDeletionInterval;
    }

    // Called to adjust the cache totals when a resource changes size.
    void AdjustSize(bool live, int64 delta) noexcept;

    void AddToLiveResourcesSize(CachedResource &) noexcept;
    void RemoveFromLiveResourcesSize(CachedResource &) noexcept;

    void ResourceAccessed(CachedResource &) noexcept;

    // pruneDead*() - Flush decoded and encoded data from resources not referenced by Web pages.
    // pruneLive*() - Flush decoded data from resources still referenced by Web pages.
    void PruneDeadResources() noexcept; // Automatically decide how much to prune.
    void PruneLiveResources(bool shouldDestroyDecodedDataForAllLiveResources = false) noexcept;
    void PruneDeadResourcesToSize(uint32 targetSize) noexcept;
    void PruneLiveResourcesToSize(uint32 targetSize,
                                  bool shouldDestroyDecodedDataForAllLiveResources = false) noexcept;

  private:
    MemoryCache() noexcept;

    KRYS_NODISCARD uint32 LiveCapacity() const noexcept;
    KRYS_NODISCARD uint32 DeadCapacity() const noexcept;
    KRYS_NODISCARD bool NeedsPruning() const noexcept;

    void RemoveFromLRUList(CachedResource &) noexcept;
  };
}
