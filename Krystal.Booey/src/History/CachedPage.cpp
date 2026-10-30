#include "Krystal.Booey/History/CachedPage.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/History/BackForwardController.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/HistoryController.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Navigation.hpp"
#include "Krystal.Booey/Page/Page.hpp"

namespace krys::boo
{
  CachedPage::CachedPage(Page &page) noexcept
      : _page(CreateWeakPtr(&page)), _expirationTime(html::EventLoop::Current().Now() + ExpirationInterval),
        _cachedMainFrame(CreateUnique<CachedFrame>(page.MainFrame()))
  {
  }

  CachedPage::~CachedPage() noexcept
  {
    if (_cachedMainFrame)
    {
      _cachedMainFrame->Destroy();
    }
  }

  namespace
  {
    void FirePageShowEvent(Page &page) noexcept
    {
      // Dispatching JavaScript events can cause frame destruction.
      Ref<LocalFrame> mainFrame = ShareRef(page.MainFrame());
      if (RefPtr<dom::Document> document = ShareRefPtr(mainFrame->Document()))
      {
        // FIXME: The document's visibility (visibilitychange) and its reveal, once there are those.
        document->DispatchPageshowEvent(true);
      }
    }
  }

  void CachedPage::Restore(Page &page) noexcept
  {
    krys_debug_assert(_cachedMainFrame);
    krys_debug_assert(_cachedMainFrame->View());

    _cachedMainFrame->Open();

    // FIXME: Restoring the focused element's focus appearance, the device and page scale factors, the
    // contents size and the caption preferences, once there are those.

    FirePageShowEvent(page);

    // Update Navigation API after pageshow events to ensure correct event ordering.
    RestoreNavigationAPIHistoryItems(page);

    Clear();
  }

  void CachedPage::RestoreNavigationAPIHistoryItems(Page &page) noexcept
  {
    auto &frame = page.MainFrame();
    auto *window = frame.Document() ? frame.Document()->Window() : nullptr;
    auto *currentItem = frame.Loader().History().CurrentItem();
    if (!window || !currentItem)
    {
      return;
    }

    auto &backForward = page.BackForward();
    RefPtr<HistoryItem> previousItem = backForward.ForwardItem();
    auto filteredItems = Navigation::FilterHistoryItemsForNavigationAPI(backForward.AllItems(), *currentItem);
    window->GetNavigation().UpdateForReactivation(krys::move(filteredItems), *currentItem,
                                                  previousItem.get());
  }

  void CachedPage::Clear() noexcept
  {
    krys_debug_assert(_cachedMainFrame);
    _cachedMainFrame->Clear();
    _cachedMainFrame = nullptr;
  }

  bool CachedPage::HasExpired() const noexcept
  {
    return html::EventLoop::Current().Now() > _expirationTime;
  }
}
