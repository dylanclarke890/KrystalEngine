#pragma once

#include "Krystal.Booey/History/CachedFrame.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <chrono>

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  class DocumentLoader;
  class Page;

  /// @brief A page in the back/forward cache: its main frame's cached document, until it expires.
  class CachedPage final
  {
    KRYS_NON_COPY_MOVABLE(CachedPage);

  private:
    WeakPtr<Page> _page;
    MonotonicTime::duration _expirationTime;
    UniquePtr<CachedFrame> _cachedMainFrame;

  public:
    /// @brief How long a page stays in the cache.
    static constexpr MonotonicTime::duration ExpirationInterval = std::chrono::minutes(30);

    /// @brief Caches @p page's main frame's document.
    explicit CachedPage(Page &page) noexcept;
    ~CachedPage() noexcept;

    /// @brief Shows the cached document in @p page's main frame again, and fires `pageshow` (persisted) at
    /// its window.
    void Restore(Page &page) noexcept;

    /// @brief Forgets the cached document.
    void Clear() noexcept;

    KRYS_NODISCARD Page *GetPage() const noexcept
    {
      return _page.get();
    }

    KRYS_NODISCARD dom::Document *Document() const noexcept
    {
      return _cachedMainFrame ? _cachedMainFrame->Document() : nullptr;
    }

    KRYS_NODISCARD DocumentLoader *GetDocumentLoader() const noexcept
    {
      return _cachedMainFrame ? _cachedMainFrame->GetDocumentLoader() : nullptr;
    }

    KRYS_NODISCARD CachedFrame *CachedMainFrame() const noexcept
    {
      return _cachedMainFrame.get();
    }

    /// @brief Whether the page has been in the cache for longer than `ExpirationInterval` (of the host's
    /// time).
    KRYS_NODISCARD bool HasExpired() const noexcept;

  private:
    /// @brief Gives the restored window's navigation the page's entries again.
    static void RestoreNavigationAPIHistoryItems(Page &page) noexcept;
  };
}
