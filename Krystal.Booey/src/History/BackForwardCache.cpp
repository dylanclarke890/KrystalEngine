#include "Krystal.Booey/History/BackForwardCache.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/History/CachedPage.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/DocumentLoader.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/HistoryController.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool CanCacheFrame(LocalFrame &frame) noexcept
    {
      auto &frameLoader = frame.Loader();

      if (frameLoader.IsDisplayingInitialEmptyDocument())
      {
        return false;
      }

      RefPtr<dom::Document> document = ShareRefPtr(frame.Document());
      if (!document || !document->Frame())
      {
        return false;
      }

      RefPtr<DocumentLoader> documentLoader = ShareRefPtr(frameLoader.GetDocumentLoader());
      if (!documentLoader)
      {
        return false;
      }

      bool isCacheable = true;

      // FIXME: Site specific quirks, error pages (substitute data for a failing URL), `Cache-Control:
      // no-store` over HTTPS and visually empty pages, once there are those.

      if (!frameLoader.History().CurrentItem())
      {
        isCacheable = false;
      }

      if (frameLoader.QuickRedirectComing())
      {
        isCacheable = false;
      }

      // Still loading: its main resource, or its subresources.
      if (documentLoader->IsLoadingMainResource() || document->GetCachedResourceLoader().RequestCount())
      {
        isCacheable = false;
      }

      if (!frameLoader.Client().CanCachePage())
      {
        isCacheable = false;
      }

      return isCacheable;
    }

    KRYS_NODISCARD bool CanCachePage(Page &page) noexcept
    {
      auto &mainFrame = page.MainFrame();
      bool isCacheable = CanCacheFrame(mainFrame);

      // FIXME: Device motion and orientation, once there are those.

      switch (mainFrame.Loader().LoadType())
      {
        case FrameLoadType::Reload:
        case FrameLoadType::ReloadFromOrigin:
        case FrameLoadType::ReloadExpiredOnly:
          // No point writing to the cache on a reload, since we will just write over it again when we leave
          // that page.
        case FrameLoadType::Same:
          // No point writing to the cache on a same load, since we will just write over it again when we
          // leave that page.
        case FrameLoadType::RedirectWithLockedBackForwardList:
          // Don't write to the cache if in the middle of a redirect, since we will want to store the final
          // page we end up on.
        case FrameLoadType::MultipartReplace:
        case FrameLoadType::NavigationAPIReplace:
          // No point writing to the cache on a replace, since we will just write over it again when we leave
          // that page.
          isCacheable = false;
          break;
        case FrameLoadType::Standard:
        case FrameLoadType::Back:
        case FrameLoadType::Forward:
        case FrameLoadType::IndexedBackForward:
          // Cacheable.
          break;
      }

      return isCacheable;
    }

    void SetBackForwardCacheState(Page &page, dom::BackForwardCacheState state) noexcept
    {
      page.ForEachDocument([&](dom::Document &document) { document.SetBackForwardCacheState(state); });
    }

    void FirePageHideEvent(LocalFrame &frame) noexcept
    {
      RefPtr<dom::Document> document = ShareRefPtr(frame.Document());
      if (!document)
      {
        return;
      }
      frame.Loader().StopLoading(UnloadEventPolicy::UnloadAndPageHide);
    }
  }

  BackForwardCache::BackForwardCache() noexcept = default;

  BackForwardCache::~BackForwardCache() noexcept = default;

  BackForwardCache &BackForwardCache::Singleton() noexcept
  {
    static NeverDestroyed<BackForwardCache> globalBackForwardCache;
    return globalBackForwardCache;
  }

  bool BackForwardCache::CanCache(Page &page) const noexcept
  {
    if (!_maxSize)
    {
      return false;
    }

    // FIXME: Memory pressure, once the host reports it.
    return CanCachePage(page);
  }

  void BackForwardCache::PruneToSizeNow(uint32 size, PruningReason pruningReason) noexcept
  {
    SetForScope change(_maxSize, size);
    Prune(pruningReason);
  }

  void BackForwardCache::SetMaxSize(uint32 maxSize) noexcept
  {
    _maxSize = maxSize;
    Prune(PruningReason::None);
  }

  UniquePtr<CachedPage> BackForwardCache::TrySuspendPage(Page &page, ForceSuspension forceSuspension) noexcept
  {
    Ref<LocalFrame> mainFrame = ShareRef(page.MainFrame());
    mainFrame->Loader().StopForBackForwardCache();

    if (forceSuspension == ForceSuspension::No && !CanCache(page))
    {
      return nullptr;
    }

    SetBackForwardCacheState(page, dom::BackForwardCacheState::AboutToEnterBackForwardCache);

    // FIXME: Focusing the main frame, defocusing a focused subframe, once there are frames other than the
    // main frame.

    // Fire the pagehide event in all frames.
    FirePageHideEvent(*mainFrame);

    page.DestroyRenderTrees();

    // Stop all loads again before checking if we can still cache the page after firing the pagehide event,
    // since the page may have started ping loads in its pagehide event handler.
    mainFrame->Loader().StopForBackForwardCache();

    // Check that the page is still page-cacheable after firing the pagehide event. The JS event handlers
    // could have altered the page in a way that could prevent caching.
    if (forceSuspension == ForceSuspension::No && !CanCache(page))
    {
      SetBackForwardCacheState(page, dom::BackForwardCacheState::NotInBackForwardCache);
      return nullptr;
    }

    SetBackForwardCacheState(page, dom::BackForwardCacheState::InBackForwardCache);

    return CreateUnique<CachedPage>(page);
  }

  bool BackForwardCache::AddIfCacheable(HistoryItem &item, Page *page) noexcept
  {
    if (item.IsInBackForwardCache())
    {
      return false;
    }

    if (!page)
    {
      return false;
    }

    auto cachedPage = TrySuspendPage(*page, ForceSuspension::No);
    if (!cachedPage)
    {
      return false;
    }

    _cachedPageMap.insert_or_assign(item.ItemID(), krys::move(cachedPage));
    (void)_items.add(item.ItemID());

    Prune(PruningReason::ReachedMaxSize);
    return true;
  }

  UniquePtr<CachedPage> BackForwardCache::SuspendPage(Page &page) noexcept
  {
    return TrySuspendPage(page, ForceSuspension::Yes);
  }

  UniquePtr<CachedPage> BackForwardCache::Take(HistoryItem &item, Page *) noexcept
  {
    auto iterator = _cachedPageMap.find(item.ItemID());
    if (iterator == _cachedPageMap.end())
    {
      return nullptr;
    }

    auto *cachedPage = std::get_if<UniquePtr<CachedPage>>(&iterator->second);
    if (!cachedPage)
    {
      krys_debug_assert(!_items.contains(iterator->first));
      return nullptr;
    }

    (void)_items.remove(item.ItemID());
    auto taken = krys::move(*cachedPage);
    _cachedPageMap.erase(iterator);

    if (taken->HasExpired())
    {
      return nullptr;
    }

    return taken;
  }

  void BackForwardCache::RemoveAllItemsForPage(Page &page) noexcept
  {
    // The pages are destroyed after they're out of the map, so their destruction doesn't find them in it.
    List<UniquePtr<CachedPage>> removed;
    for (auto iterator = _cachedPageMap.begin(); iterator != _cachedPageMap.end();)
    {
      auto *cachedPage = std::get_if<UniquePtr<CachedPage>>(&iterator->second);
      if (cachedPage && (*cachedPage)->GetPage() == &page)
      {
        (void)_items.remove(iterator->first);
        removed.push_back(krys::move(*cachedPage));
        iterator = _cachedPageMap.erase(iterator);
        continue;
      }
      ++iterator;
    }
  }

  CachedPage *BackForwardCache::Get(HistoryItem &item, Page *) noexcept
  {
    auto iterator = _cachedPageMap.find(item.ItemID());
    if (iterator == _cachedPageMap.end())
    {
      return nullptr;
    }

    auto *cachedPage = std::get_if<UniquePtr<CachedPage>>(&iterator->second);
    if (!cachedPage)
    {
      krys_debug_assert(!_items.contains(iterator->first));
      return nullptr;
    }

    if ((*cachedPage)->HasExpired())
    {
      Remove(item);
      return nullptr;
    }
    return cachedPage->get();
  }

  void BackForwardCache::Remove(BackForwardItemIdentifier itemID) noexcept
  {
    // Safely ignore attempts to remove items not in the cache.
    auto iterator = _cachedPageMap.find(itemID);
    if (iterator == _cachedPageMap.end() || std::holds_alternative<PruningReason>(iterator->second))
    {
      return;
    }

    (void)_items.remove(itemID);
    auto cachedPage = krys::move(std::get<UniquePtr<CachedPage>>(iterator->second));
    _cachedPageMap.erase(iterator);
  }

  void BackForwardCache::Remove(HistoryItem &item) noexcept
  {
    Remove(item.ItemID());
  }

  void BackForwardCache::Prune(PruningReason pruningReason) noexcept
  {
    while (PageCount() > MaxSize())
    {
      auto oldestItem = _items.first();
      (void)_items.remove(oldestItem);
      // Take the CachedPage before calling set() so ~CachedPage doesn't find itself in the map.
      auto iterator = _cachedPageMap.find(oldestItem);
      UniquePtr<CachedPage> cachedPage;
      if (iterator != _cachedPageMap.end())
      {
        if (auto *page = std::get_if<UniquePtr<CachedPage>>(&iterator->second))
        {
          cachedPage = krys::move(*page);
        }
      }
      _cachedPageMap.insert_or_assign(oldestItem, pruningReason);
    }
  }

  bool BackForwardCache::IsInBackForwardCache(BackForwardItemIdentifier identifier) const noexcept
  {
    auto iterator = _cachedPageMap.find(identifier);
    return iterator != _cachedPageMap.end()
           && std::holds_alternative<UniquePtr<CachedPage>>(iterator->second);
  }

  bool BackForwardCache::HasCachedPageExpired(BackForwardItemIdentifier identifier) const noexcept
  {
    auto iterator = _cachedPageMap.find(identifier);
    if (iterator == _cachedPageMap.end())
    {
      return false;
    }
    auto *cachedPage = std::get_if<UniquePtr<CachedPage>>(&iterator->second);
    return cachedPage && (*cachedPage)->HasExpired();
  }
}
