#pragma once

#include "Krystal.Booey/History/HistoryItem.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  class CachedPage;
  class LocalFrame;
  class Page;

  /// @brief Why a page left the back/forward cache before it was taken back.
  enum class PruningReason : uint8
  {
    None,
    ProcessSuspended,
    MemoryPressure,
    ReachedMaxSize
  };

  /// @brief The pages left (suspended, with their documents, views and loaders) that going back or forward
  /// shows again at once, rather than loading them: up to its maximum size (none until the host sets one),
  /// the oldest leaving first, each for at most `CachedPage::ExpirationInterval`. A page is cached as it's
  /// left, if it can be: it's loaded, it's not a reload or a redirect, and its client allows it.
  /// @see https://html.spec.whatwg.org/#note-bfcache
  class BackForwardCache final
  {
    KRYS_NON_COPY_MOVABLE(BackForwardCache);

  private:
    HashMap<BackForwardItemIdentifier, Variant<PruningReason, UniquePtr<CachedPage>>> _cachedPageMap;
    /// @brief The cached pages' items, the oldest first.
    ListHashSet<BackForwardItemIdentifier> _items;
    uint32 _maxSize {0};

  public:
    BackForwardCache() noexcept;
    ~BackForwardCache() noexcept;

    /// @brief The back/forward cache of the process.
    KRYS_NODISCARD static BackForwardCache &Singleton() noexcept;

    /// @brief Whether @p page's document can be cached as it's left.
    KRYS_NODISCARD bool CanCache(Page &page) const noexcept;

    /// @brief Drops the oldest pages until there are at most @p maxSize, for @p pruningReason.
    void PruneToSizeNow(uint32 maxSize, PruningReason pruningReason) noexcept;
    /// @brief Sets how many pages the cache keeps; the oldest above it go.
    void SetMaxSize(uint32 maxSize) noexcept;
    KRYS_NODISCARD uint32 MaxSize() const noexcept
    {
      return _maxSize;
    }

    /// @brief Suspends @p page, whether or not it can be cached.
    KRYS_NODISCARD UniquePtr<CachedPage> SuspendPage(Page &page) noexcept;
    /// @brief Caches @p page's document for @p item as it's left, if it can be (firing `pagehide`,
    /// persisted); prunes if the maximum size is exceeded.
    /// @return Whether the page was cached.
    bool AddIfCacheable(HistoryItem &item, Page *page) noexcept;
    void Remove(BackForwardItemIdentifier itemID) noexcept;
    void Remove(HistoryItem &item) noexcept;
    /// @brief The page cached for @p item; null if there's none, or it expired (which removes it).
    KRYS_NODISCARD CachedPage *Get(HistoryItem &item, Page *page) noexcept;
    /// @brief Takes the page cached for @p item out of the cache; null if there's none or it expired.
    KRYS_NODISCARD UniquePtr<CachedPage> Take(HistoryItem &item, Page *page) noexcept;

    /// @brief Drops the pages cached for @p page, which is going away.
    void RemoveAllItemsForPage(Page &page) noexcept;

    KRYS_NODISCARD uint32 PageCount() const noexcept
    {
      return static_cast<uint32>(_items.size());
    }

    KRYS_NODISCARD bool IsInBackForwardCache(BackForwardItemIdentifier identifier) const noexcept;
    KRYS_NODISCARD bool HasCachedPageExpired(BackForwardItemIdentifier identifier) const noexcept;

  private:
    enum class ForceSuspension : bool
    {
      No,
      Yes
    };

    KRYS_NODISCARD UniquePtr<CachedPage> TrySuspendPage(Page &page, ForceSuspension forceSuspension) noexcept;
    void Prune(PruningReason pruningReason) noexcept;
  };
}
