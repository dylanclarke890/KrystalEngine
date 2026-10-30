#include "Krystal.Booey/Page/Navigation.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/AbortController.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/History/HistoryItem.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/ErrorEvent.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/URLDecomposition.hpp"
#include "Krystal.Booey/Loader/FormSubmission.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/FrameLoadRequest.hpp"
#include "Krystal.Booey/Loader/HistoryController.hpp"
#include "Krystal.Booey/Loader/NavigationScheduler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/NavigationCurrentEntryChangeEvent.hpp"
#include "Krystal.Booey/Page/NavigationDestination.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool IsSameOrigin(const url::URL &a, const url::URL &b) noexcept
    {
      // FIXME: Origins, once there are any: opaque origins are never the same.
      return html::URLDecomposition::SerializedOrigin(a) == html::URLDecomposition::SerializedOrigin(b);
    }

    KRYS_NODISCARD bool IsAboutBlank(const url::URL &url) noexcept
    {
      return url.ProtocolIsAbout() && url.Path() == u8"blank";
    }

    // https://html.spec.whatwg.org/multipage/nav-history-apis.html#getting-the-navigation-api-entry-index
    KRYS_NODISCARD Maybe<size_t> GetEntryIndexOfHistoryItem(const List<Ref<NavigationHistoryEntry>> &entries,
                                                            const HistoryItem &item) noexcept
    {
      for (size_t index = 0; index < entries.size(); ++index)
      {
        if (entries[index]->AssociatedHistoryItem().ItemSequenceNumber() == item.ItemSequenceNumber())
        {
          return index;
        }
      }
      return null;
    }

    // https://html.spec.whatwg.org/multipage/nav-history-apis.html#navigation-api-early-error-result
    KRYS_NODISCARD NavigationResult CreateErrorResult(dom::Exception &&exception) noexcept
    {
      NavigationResult result {NavigationPromise::Create(), NavigationPromise::Create()};
      result.Committed->Reject(dom::Exception(exception));
      result.Finished->Reject(krys::move(exception));
      return result;
    }

    KRYS_NODISCARD NavigationResult CreateErrorResult(dom::ExceptionCode code,
                                                      utf8_stringview message) noexcept
    {
      return CreateErrorResult(dom::Exception {code, utf8_string(message)});
    }

    KRYS_NODISCARD dom::Exception AbortError() noexcept
    {
      return dom::Exception {dom::ExceptionCode::AbortError, u8"Navigation aborted"};
    }

    // https://html.spec.whatwg.org/multipage/nav-history-apis.html#can-have-its-url-rewritten
    KRYS_NODISCARD bool DocumentCanHaveURLRewritten(const dom::Document &document,
                                                    const url::URL &targetURL) noexcept
    {
      const auto &documentURL = document.GetURL();
      if (!IsSameOrigin(documentURL, targetURL))
      {
        return false;
      }

      if (targetURL.ProtocolIsInHTTPFamily())
      {
        return true;
      }

      // Differences in query and fragment are allowed for file: URLs.
      if (targetURL.ProtocolIsFile())
      {
        return documentURL.ViewWithoutQueryOrFragmentIdentifier()
               == targetURL.ViewWithoutQueryOrFragmentIdentifier();
      }

      return url::EqualIgnoringFragmentIdentifier(documentURL, targetURL);
    }
  }

  bool Navigation::RateLimiter::NavigationAllowed() noexcept
  {
    auto currentTime = html::EventLoop::Current().Now();

    // Check if we've exceeded the time window and need to reset.
    if (currentTime - WindowStartTime > WindowDuration)
    {
      WindowStartTime = currentTime;
      NavigationCount = 0;
    }

    // Allow navigation if we're still under the limit.
    if (NavigationCount < MaxNavigationsPerWindow)
    {
      ++NavigationCount;
      return true;
    }
    return false;
  }

  Navigation::Navigation(LocalDOMWindow &window) noexcept : _window(CreateWeakPtr(&window))
  {
  }

  Navigation::~Navigation() noexcept = default;

  LocalFrame *Navigation::Frame() const noexcept
  {
    auto *window = _window.get();
    return window ? window->Frame() : nullptr;
  }

  bool Navigation::HasEntriesAndEventsDisabled() const noexcept
  {
    auto *window = _window.get();
    auto *document = window ? window->Document() : nullptr;
    if (!document || !document->IsFullyActive())
    {
      return true;
    }
    if (document->Frame()->Loader().IsDisplayingInitialEmptyDocument())
    {
      return true;
    }
    // FIXME: Windows of opaque origins, once there are origins.
    return false;
  }

  bool Navigation::CanGoBack() const noexcept
  {
    if (HasEntriesAndEventsDisabled())
    {
      return false;
    }
    krys_debug_assert(_currentEntryIndex);
    return _currentEntryIndex && *_currentEntryIndex;
  }

  bool Navigation::CanGoForward() const noexcept
  {
    if (HasEntriesAndEventsDisabled())
    {
      return false;
    }
    krys_debug_assert(_currentEntryIndex);
    return _currentEntryIndex && *_currentEntryIndex != _entries.size() - 1;
  }

  void Navigation::InitializeForNewWindow(Maybe<NavigationNavigationType> navigationType,
                                          LocalDOMWindow *previousWindow) noexcept
  {
    krys_debug_assert(_entries.empty());
    krys_debug_assert(!_currentEntryIndex);

    if (HasEntriesAndEventsDisabled())
    {
      return;
    }

    auto *frame = Frame();
    if (!frame || !frame->GetPage())
    {
      return;
    }

    RefPtr<HistoryItem> currentItem = ShareRefPtr(frame->Loader().History().CurrentItem());
    if (!currentItem)
    {
      return;
    }

    if (previousWindow)
    {
      auto &previousNavigation = previousWindow->GetNavigation();

      bool shouldProcessPreviousNavigationEntries =
        previousNavigation._currentEntryIndex && !previousNavigation._entries.empty()
        && previousWindow->Document()
        && IsSameOrigin(frame->Document()->GetURL(), previousWindow->Document()->GetURL());

      if (shouldProcessPreviousNavigationEntries)
      {
        for (auto &entry : previousNavigation._entries)
        {
          _entries.push_back(NavigationHistoryEntry::Create(*this, *entry));
        }

        krys_assert(_entries.size() > *previousNavigation._currentEntryIndex);

        if (navigationType == NavigationNavigationType::Traverse)
        {
          _currentEntryIndex = GetEntryIndexOfHistoryItem(_entries, *currentItem);
          if (_currentEntryIndex)
          {
            SetActivation(frame->Loader().History().PreviousItem(), navigationType);
            return;
          }
          // We are doing a cross document traversal, we can't rely on previous window, so clear m_entries and
          // fall back to the normal algorithm for new windows.
          _entries.clear();
        }
        else if (navigationType == NavigationNavigationType::Push)
        {
          // Prune forward entries.
          _entries.erase(_entries.begin()
                           + static_cast<ptrdiff_t>(*previousNavigation._currentEntryIndex + 1),
                         _entries.end());
        }
        else
        {
          Ref<NavigationHistoryEntry> previousEntry = _entries[*previousNavigation._currentEntryIndex];

          if (navigationType == NavigationNavigationType::Replace)
          {
            _entries[*previousNavigation._currentEntryIndex] =
              NavigationHistoryEntry::Create(*this, ShareRef(*currentItem));
          }

          _currentEntryIndex = GetEntryIndexOfHistoryItem(_entries, *currentItem);
          if (_currentEntryIndex && navigationType)
          {
            _activation = NavigationActivation::Create(*navigationType, ShareRef(*CurrentEntry()),
                                                       RefPtr<NavigationHistoryEntry>(previousEntry));
            return;
          }
        }
      }
    }

    _entries.push_back(NavigationHistoryEntry::Create(*this, ShareRef(*currentItem)));
    _currentEntryIndex = _entries.size() - 1;

    SetActivation(frame->Loader().History().PreviousItem(), navigationType);
  }

  void Navigation::SetActivation(HistoryItem *previousItem, Maybe<NavigationNavigationType> type) noexcept
  {
    krys_debug_assert(!_activation);
    if (HasEntriesAndEventsDisabled() || !type)
    {
      return;
    }

    krys_debug_assert(_currentEntryIndex);
    if (IsAboutBlank(CurrentEntry()->AssociatedHistoryItem().URL()))
    {
      return;
    }

    // FIXME: *Initial* about:blank. For navigations on the initial about blank this should already be the
    // type.
    bool wasAboutBlank = previousItem && IsAboutBlank(previousItem->URL());
    if (wasAboutBlank)
    {
      type = NavigationNavigationType::Replace;
    }

    auto *document = Frame()->Document();
    bool isSameOrigin = document && previousItem && IsSameOrigin(previousItem->URL(), document->GetURL());
    auto previousEntryIndex = previousItem ? GetEntryIndexOfHistoryItem(_entries, *previousItem) : null;

    RefPtr<NavigationHistoryEntry> previousEntry;
    if (previousEntryIndex && isSameOrigin)
    {
      previousEntry = _entries[*previousEntryIndex];
    }
    if (type == NavigationNavigationType::Reload)
    {
      previousEntry = ShareRefPtr(CurrentEntry());
    }
    else if (type == NavigationNavigationType::Replace && previousItem && (isSameOrigin || wasAboutBlank))
    {
      previousEntry = NavigationHistoryEntry::Create(*this, ShareRef(*previousItem));
    }

    _activation = NavigationActivation::Create(*type, ShareRef(*CurrentEntry()), krys::move(previousEntry));
  }

  const List<Ref<NavigationHistoryEntry>> &Navigation::Entries() const noexcept
  {
    static const List<Ref<NavigationHistoryEntry>> emptyEntries;
    if (HasEntriesAndEventsDisabled())
    {
      return emptyEntries;
    }
    return _entries;
  }

  NavigationHistoryEntry *Navigation::CurrentEntry() const noexcept
  {
    if (!HasEntriesAndEventsDisabled() && _currentEntryIndex)
    {
      return _entries[*_currentEntryIndex].get();
    }
    return nullptr;
  }

  RefPtr<NavigationAPIMethodTracker> Navigation::MaybeSetUpcomingNonTraversalTracker(
    RefPtr<html::SerializedScriptValue> &&info,
    RefPtr<html::SerializedScriptValue> &&serializedState) noexcept
  {
    auto apiMethodTracker =
      CreateRef<NavigationAPIMethodTracker>(krys::move(info), krys::move(serializedState));

    // FIXME: We should be able to assert m_upcomingNonTraverseMethodTracker is empty.
    if (!HasEntriesAndEventsDisabled())
    {
      _upcomingNonTraverseMethodTracker = apiMethodTracker;
    }

    return apiMethodTracker;
  }

  RefPtr<NavigationAPIMethodTracker>
    Navigation::AddUpcomingTraverseAPIMethodTracker(dom::DOMStringView key,
                                                    RefPtr<html::SerializedScriptValue> &&info) noexcept
  {
    auto apiMethodTracker = CreateRef<NavigationAPIMethodTracker>(krys::move(info), nullptr);
    apiMethodTracker->Key = dom::DOMString(key);
    _upcomingTraverseMethodTrackers.insert_or_assign(dom::DOMString(key),
                                                     Ref<NavigationAPIMethodTracker>(apiMethodTracker));
    return apiMethodTracker;
  }

  NavigationResult
    Navigation::ApiMethodTrackerDerivedResult(const NavigationAPIMethodTracker &apiMethodTracker) noexcept
  {
    return {apiMethodTracker.CommittedPromise, apiMethodTracker.FinishedPromise};
  }

  NavigationResult Navigation::Reload(ReloadOptions &&options) noexcept
  {
    auto state = krys::move(options.State);
    if (!state && CurrentEntry())
    {
      state = ShareRefPtr(CurrentEntry()->AssociatedHistoryItem().NavigationAPIStateObject());
    }

    auto *window = _window.get();
    auto *document = window ? window->Document() : nullptr;
    if (!document || !document->IsFullyActive() || document->UnloadCounter())
    {
      return CreateErrorResult(dom::ExceptionCode::InvalidStateError, u8"Invalid state");
    }

    auto apiMethodTracker = MaybeSetUpcomingNonTraversalTracker(krys::move(options.Info), krys::move(state));

    RefPtr<LocalFrame> frame = ShareRefPtr(Frame());
    FrameLoadRequest frameLoadRequest(ShareRef(*document), ResourceRequest(url::URL(document->GetURL())),
                                      dom::DOMStringAtom(u8"_self"));
    frameLoadRequest.SetNavigationHistoryBehavior(NavigationHistoryBehavior::Reload);
    frameLoadRequest.SetIsFromNavigationAPI(true);
    frame->Loader().ChangeLocation(krys::move(frameLoadRequest));

    return ApiMethodTrackerDerivedResult(*apiMethodTracker);
  }

  NavigationResult Navigation::Navigate(dom::DOMStringView url, NavigateOptions &&options) noexcept
  {
    auto *window = _window.get();
    auto *document = window ? window->Document() : nullptr;
    if (!document)
    {
      return CreateErrorResult(dom::ExceptionCode::InvalidStateError, u8"Invalid state");
    }

    auto newURL = document->CompleteURL(url);
    const auto &currentURL = document->GetURL();

    if (!newURL.IsValid())
    {
      return CreateErrorResult(dom::ExceptionCode::SyntaxError, u8"Invalid URL");
    }

    // Reject all JavaScript URLs in Navigation API.
    if (newURL.ProtocolIsJavaScript())
    {
      return CreateErrorResult(dom::ExceptionCode::NotSupportedError,
                               u8"Navigation API does not support javascript: URLs.");
    }

    if (options.History == HistoryBehavior::Push && IsAboutBlank(currentURL))
    {
      return CreateErrorResult(
        dom::ExceptionCode::NotSupportedError,
        u8"A \"push\" navigation was explicitly requested, but only a \"replace\" navigation is "
        u8"possible while on an about:blank document.");
    }

    if (!document->IsFullyActive() || document->UnloadCounter())
    {
      return CreateErrorResult(dom::ExceptionCode::InvalidStateError, u8"Invalid state");
    }

    auto apiMethodTracker =
      MaybeSetUpcomingNonTraversalTracker(krys::move(options.Info), krys::move(options.State));

    RefPtr<LocalFrame> frame = ShareRefPtr(Frame());
    FrameLoadRequest request(ShareRef(*document), ResourceRequest(krys::move(newURL)),
                             dom::DOMStringAtom(u8"_self"));
    request.SetNavigationHistoryBehavior(options.History);
    request.SetIsFromNavigationAPI(true);
    frame->Loader().LoadFrameRequest(krys::move(request), nullptr, nullptr);

    // If the load() call never made it to the point that NavigateEvent was emitted, thus
    // promoteUpcomingAPIMethodTracker() called, this will be true.
    if (_upcomingNonTraverseMethodTracker == apiMethodTracker)
    {
      _upcomingNonTraverseMethodTracker = nullptr;
      apiMethodTracker->CommittedPromise->Reject(AbortError());
      apiMethodTracker->FinishedPromise->Reject(AbortError());
    }

    return ApiMethodTrackerDerivedResult(*apiMethodTracker);
  }

  NavigationResult Navigation::PerformTraversal(dom::DOMStringView key, Options &&options) noexcept
  {
    auto *window = _window.get();
    auto *document = window ? window->Document() : nullptr;
    if (!document || !document->IsFullyActive() || document->UnloadCounter())
    {
      return CreateErrorResult(dom::ExceptionCode::InvalidStateError, u8"Invalid state");
    }

    if (!EntryIndexOfKey(key))
    {
      return CreateErrorResult(AbortError());
    }

    RefPtr<NavigationHistoryEntry> current = ShareRefPtr(CurrentEntry());
    if (current->Key() == key)
    {
      NavigationResult result {NavigationPromise::Create(), NavigationPromise::Create()};
      result.Committed->Resolve(current.get());
      result.Finished->Resolve(current.get());
      return result;
    }

    if (auto existing = _upcomingTraverseMethodTrackers.find(dom::DOMString(key));
        existing != _upcomingTraverseMethodTrackers.end())
    {
      return ApiMethodTrackerDerivedResult(*existing->second);
    }

    auto apiMethodTracker = AddUpcomingTraverseAPIMethodTracker(key, krys::move(options.Info));

    // FIXME: 11. Let sourceSnapshotParams be the result of snapshotting source snapshot params given
    // document.
    Frame()->GetNavigationScheduler().ScheduleHistoryNavigationByKey(
      dom::DOMString(key),
      [apiMethodTracker](ScheduleHistoryNavigationResult result)
      {
        if (result == ScheduleHistoryNavigationResult::Aborted)
        {
          apiMethodTracker->CommittedPromise->Reject(AbortError());
          apiMethodTracker->FinishedPromise->Reject(AbortError());
        }
      });

    return ApiMethodTrackerDerivedResult(*apiMethodTracker);
  }

  Maybe<size_t> Navigation::EntryIndexOfKey(dom::DOMStringView key) const noexcept
  {
    if (key.empty())
    {
      return null;
    }
    for (size_t index = 0; index < _entries.size(); ++index)
    {
      if (_entries[index]->Key() == key)
      {
        return index;
      }
    }
    return null;
  }

  NavigationHistoryEntry *Navigation::FindEntryByKey(dom::DOMStringView key) const noexcept
  {
    auto index = EntryIndexOfKey(key);
    return index ? _entries[*index].get() : nullptr;
  }

  NavigationResult Navigation::TraverseTo(dom::DOMStringView key, Options &&options) noexcept
  {
    if (!EntryIndexOfKey(key))
    {
      return CreateErrorResult(dom::ExceptionCode::InvalidStateError, u8"Invalid key");
    }
    return PerformTraversal(key, krys::move(options));
  }

  NavigationResult Navigation::Back(Options &&options) noexcept
  {
    if (!CanGoBack())
    {
      return CreateErrorResult(dom::ExceptionCode::InvalidStateError, u8"Cannot go back");
    }
    Ref<NavigationHistoryEntry> previousEntry = _entries[*_currentEntryIndex - 1];
    return PerformTraversal(previousEntry->Key(), krys::move(options));
  }

  NavigationResult Navigation::Forward(Options &&options) noexcept
  {
    if (!CanGoForward())
    {
      return CreateErrorResult(dom::ExceptionCode::InvalidStateError, u8"Cannot go forward");
    }
    Ref<NavigationHistoryEntry> nextEntry = _entries[*_currentEntryIndex + 1];
    return PerformTraversal(nextEntry->Key(), krys::move(options));
  }

  dom::ExceptionOr<void> Navigation::UpdateCurrentEntry(UpdateCurrentEntryOptions &&options) noexcept
  {
    RefPtr<NavigationHistoryEntry> current = ShareRefPtr(CurrentEntry());
    if (!current)
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    current->SetState(krys::move(options.State));

    auto currentEntryChangeEvent = NavigationCurrentEntryChangeEvent::Create(null, *current);
    (void)dom::EventDispatcher::DispatchToTarget(*currentEntryChangeEvent, this);
    return {};
  }

  void Navigation::ResolveFinishedPromise(NavigationAPIMethodTracker *apiMethodTracker) noexcept
  {
    RefPtr<NavigationHistoryEntry> committedToEntry = apiMethodTracker->CommittedToEntry;
    if (!committedToEntry)
    {
      apiMethodTracker->FinishedBeforeCommit = true;
      return;
    }

    apiMethodTracker->CommittedPromise->Resolve(committedToEntry.get());
    apiMethodTracker->FinishedPromise->Resolve(committedToEntry.get());
    CleanupAPIMethodTracker(apiMethodTracker);
  }

  void Navigation::RejectFinishedPromise(NavigationAPIMethodTracker *apiMethodTracker,
                                         const dom::Exception &exception) noexcept
  {
    // Only reject committed promise if it hasn't been fulfilled yet (committedToEntry is null). If the
    // navigation was "committed" (state updated) before being aborted, the committed promise should remain
    // fulfilled while only the finished promise gets rejected.
    if (!apiMethodTracker->CommittedToEntry)
    {
      apiMethodTracker->CommittedPromise->Reject(dom::Exception(exception));
    }
    apiMethodTracker->FinishedPromise->Reject(dom::Exception(exception));
    CleanupAPIMethodTracker(apiMethodTracker);
  }

  void Navigation::RejectFinishedPromise(NavigationAPIMethodTracker *apiMethodTracker) noexcept
  {
    if (!apiMethodTracker)
    {
      return;
    }
    RejectFinishedPromise(apiMethodTracker, AbortError());
  }

  void Navigation::NotifyCommittedToEntry(NavigationAPIMethodTracker *apiMethodTracker,
                                          NavigationHistoryEntry *entry,
                                          NavigationNavigationType navigationType) noexcept
  {
    krys_debug_assert(entry);
    apiMethodTracker->CommittedToEntry = ShareRefPtr(entry);
    if (navigationType != NavigationNavigationType::Traverse && apiMethodTracker->SerializedState)
    {
      apiMethodTracker->CommittedToEntry->SetState(krys::move(apiMethodTracker->SerializedState));
    }

    if (apiMethodTracker->FinishedBeforeCommit)
    {
      ResolveFinishedPromise(apiMethodTracker);
    }
    else
    {
      apiMethodTracker->CommittedPromise->Resolve(entry);
    }
  }

  void Navigation::UpdateNavigationEntry(Ref<HistoryItem> &&item,
                                         ShouldCopyStateObjectFromCurrentEntry) noexcept
  {
    if (!_currentEntryIndex)
    {
      return;
    }
    _entries[*_currentEntryIndex] = NavigationHistoryEntry::Create(*this, krys::move(item));
    // FIXME: Child frames' entries, once there are frames other than the main frame (not planned).
  }

  void Navigation::UpdateForNavigation(
    Ref<HistoryItem> &&item, NavigationNavigationType navigationType,
    ShouldCopyStateObjectFromCurrentEntry shouldCopyStateObjectFromCurrentEntry,
    ShouldNotifyCommitted shouldNotifyCommitted) noexcept
  {
    if (HasEntriesAndEventsDisabled())
    {
      return;
    }

    RefPtr<NavigationHistoryEntry> oldCurrentEntry = ShareRefPtr(CurrentEntry());
    if (!oldCurrentEntry)
    {
      return;
    }

    List<Ref<NavigationHistoryEntry>> disposedEntries;

    switch (navigationType)
    {
      case NavigationNavigationType::Traverse:
        _currentEntryIndex = GetEntryIndexOfHistoryItem(_entries, *item);
        if (!_currentEntryIndex)
        {
          return;
        }
        break;
      case NavigationNavigationType::Push:
        _currentEntryIndex = *_currentEntryIndex + 1;
        for (auto i = *_currentEntryIndex; i < _entries.size(); ++i)
        {
          disposedEntries.push_back(_entries[i]);
        }
        // A placeholder for the new entry, which UpdateNavigationEntry makes below.
        _entries.erase(_entries.begin() + static_cast<ptrdiff_t>(*_currentEntryIndex), _entries.end());
        _entries.push_back(ShareRef(*oldCurrentEntry));
        break;
      case NavigationNavigationType::Replace: disposedEntries.push_back(ShareRef(*oldCurrentEntry)); break;
      case NavigationNavigationType::Reload:  break;
    }

    if (navigationType == NavigationNavigationType::Push
        || navigationType == NavigationNavigationType::Replace)
    {
      UpdateNavigationEntry(krys::move(item), shouldCopyStateObjectFromCurrentEntry);
      if (shouldCopyStateObjectFromCurrentEntry == ShouldCopyStateObjectFromCurrentEntry::Yes)
      {
        _entries[*_currentEntryIndex]->SetState(ShareRefPtr(oldCurrentEntry->State()));
      }
    }

    RefPtr<NavigationAPIMethodTracker> ongoingAPIMethodTracker = _ongoingAPIMethodTracker;
    if (ongoingAPIMethodTracker && shouldNotifyCommitted == ShouldNotifyCommitted::Yes)
    {
      NotifyCommittedToEntry(ongoingAPIMethodTracker.get(), CurrentEntry(), navigationType);
    }

    auto currentEntryChangeEvent =
      NavigationCurrentEntryChangeEvent::Create(navigationType, *oldCurrentEntry);
    (void)dom::EventDispatcher::DispatchToTarget(*currentEntryChangeEvent, this);

    for (auto &disposedEntry : disposedEntries)
    {
      disposedEntry->DispatchDisposeEvent();
    }
  }

  void Navigation::UpdateForReactivation(List<Ref<HistoryItem>> &&newHistoryItems,
                                         HistoryItem &reactivatedItem, HistoryItem *previousItem) noexcept
  {
    if (HasEntriesAndEventsDisabled())
    {
      return;
    }

    List<Ref<NavigationHistoryEntry>> newEntries;
    auto oldEntries = std::exchange(_entries, {});

    for (auto &item : newHistoryItems)
    {
      RefPtr<NavigationHistoryEntry> newEntry;
      for (size_t entryIndex = 0; entryIndex < oldEntries.size(); ++entryIndex)
      {
        if (oldEntries[entryIndex]->AssociatedHistoryItem().ItemSequenceNumber()
            == item->ItemSequenceNumber())
        {
          newEntry = oldEntries[entryIndex];
          oldEntries.erase(oldEntries.begin() + static_cast<ptrdiff_t>(entryIndex));
          break;
        }
      }

      if (!newEntry)
      {
        newEntry = NavigationHistoryEntry::Create(*this, Ref<HistoryItem>(item));
      }
      newEntries.push_back(ShareRef(*newEntry));
    }

    _entries = krys::move(newEntries);
    _currentEntryIndex = GetEntryIndexOfHistoryItem(_entries, reactivatedItem);

    for (auto &disposedEntry : oldEntries)
    {
      disposedEntry->DispatchDisposeEvent();
    }

    // Clear any existing activation before setting a new one for the back/forward cache's restore.
    _activation = nullptr;
    SetActivation(previousItem, NavigationNavigationType::Traverse);
  }

  void Navigation::PromoteUpcomingAPIMethodTracker(const Maybe<dom::DOMString> &destinationKey) noexcept
  {
    // FIXME: We should be able to assert m_ongoingAPIMethodTracker is unset.
    if (destinationKey && !destinationKey->empty())
    {
      if (auto found = _upcomingTraverseMethodTrackers.find(*destinationKey);
          found != _upcomingTraverseMethodTrackers.end())
      {
        _ongoingAPIMethodTracker = found->second;
        _upcomingTraverseMethodTrackers.erase(found);
      }
      else
      {
        _ongoingAPIMethodTracker = nullptr;
      }
    }
    else if (!destinationKey)
    {
      _ongoingAPIMethodTracker = std::exchange(_upcomingNonTraverseMethodTracker, nullptr);
    }
    else if (!_upcomingTraverseMethodTrackers.empty())
    {
      // For traverse navigation where destination key is empty, try to use any available traverse method
      // tracker.
      auto first = _upcomingTraverseMethodTrackers.begin();
      _ongoingAPIMethodTracker = first->second;
      _upcomingTraverseMethodTrackers.erase(first);
    }
  }

  void Navigation::CleanupAPIMethodTracker(NavigationAPIMethodTracker *apiMethodTracker) noexcept
  {
    if (_ongoingAPIMethodTracker == apiMethodTracker)
    {
      _ongoingAPIMethodTracker = nullptr;
    }
    else if (!apiMethodTracker->Key.empty())
    {
      _upcomingTraverseMethodTrackers.erase(apiMethodTracker->Key);
    }
  }

  NavigationAPIMethodTracker *Navigation::UpcomingTraverseMethodTracker(dom::DOMStringView key) const noexcept
  {
    auto found = _upcomingTraverseMethodTrackers.find(dom::DOMString(key));
    return found == _upcomingTraverseMethodTrackers.end() ? nullptr : found->second.get();
  }

  Ref<Navigation::AbortHandler> Navigation::RegisterAbortHandler() noexcept
  {
    std::erase_if(_abortHandlers, [](auto &handler) { return !handler; });
    auto abortHandler = CreateRef<AbortHandler>();
    _abortHandlers.push_back(CreateWeakPtr(abortHandler.get()));
    return abortHandler;
  }

  void Navigation::DispatchNavigateErrorEvent(const dom::Exception &exception) noexcept
  {
    auto *window = _window.get();
    auto *document = window ? window->Document() : nullptr;
    dom::USVString sourceURL(document ? document->GetURL().String() : utf8_string());
    auto event =
      html::ErrorEvent::Create(dom::DOMStringAtom(EventNames::NavigateError),
                               dom::DOMString(exception.Message()), krys::move(sourceURL), 0, 0, nullptr);
    (void)dom::EventDispatcher::DispatchToTarget(*event, this);
  }

  void Navigation::AbortOngoingNavigation(NavigateEvent &event) noexcept
  {
    Ref<NavigateEvent> protectedEvent = ShareRef(event);
    for (auto &abortHandler : _abortHandlers)
    {
      if (abortHandler)
      {
        abortHandler->_wasAborted = true;
      }
    }

    _focusChangedDuringOngoingNavigation = FocusDidChange::No;
    _suppressNormalScrollRestorationDuringOngoingNavigation = false;

    if (event.IsBeingDispatched())
    {
      event.PreventDefault();
    }

    auto exception = AbortError();
    event.SignalAbort(exception);

    _ongoingNavigateEvent = nullptr;

    DispatchNavigateErrorEvent(exception);

    if (RefPtr<NavigationAPIMethodTracker> ongoingAPIMethodTracker = _ongoingAPIMethodTracker)
    {
      RejectFinishedPromise(ongoingAPIMethodTracker.get(), exception);
    }

    if (RefPtr<NavigationTransition> transition = std::exchange(_transition, nullptr))
    {
      transition->RejectPromise(krys::move(exception));
    }
  }

  void Navigation::AbortOngoingNavigationIfNeeded() noexcept
  {
    if (RefPtr<NavigateEvent> ongoingNavigateEvent = _ongoingNavigateEvent)
    {
      AbortOngoingNavigation(*ongoingNavigateEvent);
    }
  }

  namespace
  {
    /// @brief Waits for an intercepted navigation's handlers to complete, as waiting for all their promises
    /// does: fulfilled once they all succeed, rejected once one fails; never before the task that waits.
    /// @see https://webidl.spec.whatwg.org/#wait-for-all
    struct HandlerCompletions final : public RefCounted<HandlerCompletions>
    {
      size_t Remaining {0};
      bool Waiting {false};
      bool Settled {false};
      Maybe<dom::Exception> Error;
      Func<void()> OnFulfilled;
      Func<void(const dom::Exception &)> OnRejected;

      void TrySettle() noexcept
      {
        if (Settled || !Waiting)
        {
          return;
        }
        if (Error)
        {
          Settled = true;
          OnRejected(*Error);
        }
        else if (!Remaining)
        {
          Settled = true;
          OnFulfilled();
        }
      }
    };
  }

  Navigation::DispatchResult Navigation::InnerDispatchNavigateEvent(
    NavigationNavigationType navigationType, Ref<NavigationDestination> &&destination,
    Maybe<dom::DOMString> &&downloadRequestFilename, const FormSubmission *formSubmission,
    html::SerializedScriptValue *classicHistoryAPIState, dom::Element *sourceElement) noexcept
  {
    if (HasEntriesAndEventsDisabled())
    {
      krys_debug_assert(!_ongoingAPIMethodTracker);
      krys_debug_assert(!_upcomingNonTraverseMethodTracker);
      krys_debug_assert(_upcomingTraverseMethodTrackers.empty());
      return DispatchResult::Completed;
    }

    bool wasBeingDispatched = _ongoingNavigateEvent && _ongoingNavigateEvent->IsBeingDispatched();

    AbortOngoingNavigationIfNeeded();

    // Prevent recursion on synchronous history navigation steps issued from the navigate event handler.
    if (wasBeingDispatched && classicHistoryAPIState)
    {
      return DispatchResult::Completed;
    }

    PromoteUpcomingAPIMethodTracker(destination->HasEntry() ? Maybe<dom::DOMString>(destination->Key())
                                                            : null);

    RefPtr<NavigationAPIMethodTracker> apiMethodTracker = _ongoingAPIMethodTracker;

    // Enforce rate limiting to prevent excessive navigation requests. Only check for script-initiated
    // navigations (those with an API method tracker).
    if (apiMethodTracker && !_rateLimiter.NavigationAllowed())
    {
      // FIXME: A warning in the console ("Excessive navigation attempts blocked."), once there's a console.
      dom::Exception exception {dom::ExceptionCode::QuotaExceededError, u8"Navigation rate limit exceeded"};
      apiMethodTracker->CommittedPromise->Reject(dom::Exception(exception));
      apiMethodTracker->FinishedPromise->Reject(krys::move(exception));
      CleanupAPIMethodTracker(apiMethodTracker.get());
      return DispatchResult::Aborted;
    }

    RefPtr<LocalDOMWindow> window = _window.get() ? ShareRefPtr(_window.get()) : nullptr;
    RefPtr<dom::Document> document = ShareRefPtr(window->Document());

    // FIXME: this should not be needed, we should pass it into FrameLoader.
    if (apiMethodTracker && apiMethodTracker->SerializedState)
    {
      destination->SetStateObject(apiMethodTracker->SerializedState.get());
    }
    bool isSameDocument = destination->SameDocument();
    bool isTraversal = navigationType == NavigationNavigationType::Traverse;
    bool canIntercept =
      DocumentCanHaveURLRewritten(*document, destination->URL()) && (!isTraversal || isSameDocument);
    // FIXME: And user involvement is not browser-ui or navigation's relevant global object has transient
    // activation.
    bool canBeCanceled = !isTraversal || isSameDocument;
    const auto &documentURL = document->GetURL();
    const auto &destinationURL = destination->URL();
    bool hashChange = !classicHistoryAPIState
                      && url::EqualIgnoringFragmentIdentifier(documentURL, destinationURL)
                      && (documentURL.HasFragmentIdentifier() != destinationURL.HasFragmentIdentifier()
                          || documentURL.FragmentIdentifier() != destinationURL.FragmentIdentifier());

    RefPtr<html::DOMFormData> formData;
    RefPtr<dom::Element> updatedSourceElement = ShareRefPtr(sourceElement);
    if (formSubmission)
    {
      RefPtr<html::HTMLFormControlElement> submitter = ShareRefPtr(formSubmission->Submitter());
      RefPtr<html::HTMLFormElement> form = ShareRefPtr(formSubmission->Form());

      if (form && formSubmission->GetMethod() == FormSubmission::Method::Post
          && (navigationType == NavigationNavigationType::Push
              || navigationType == NavigationNavigationType::Replace))
      {
        if (auto domFormData = html::DOMFormData::Create(form.get(), submitter.get());
            !domFormData.HasException())
        {
          formData = domFormData.ReleaseValue();
        }
      }

      updatedSourceElement = submitter;
      if (!updatedSourceElement)
      {
        updatedSourceElement = form;
      }
    }

    auto abortController = CreateUnique<dom::AbortController>();

    NavigateEventInit init;
    init.Cancelable = canBeCanceled;
    init.NavigationType = navigationType;
    init.Destination = destination;
    init.Signal = ShareRefPtr(abortController->Signal());
    init.FormData = formData;
    init.DownloadRequest = krys::move(downloadRequestFilename);
    init.Info = apiMethodTracker ? apiMethodTracker->Info : nullptr;
    init.SourceElement = updatedSourceElement;
    init.CanIntercept = canIntercept;
    init.UserInitiated = window->HasTransientActivation();
    init.HashChange = hashChange;

    // Free up no longer needed info.
    if (apiMethodTracker)
    {
      apiMethodTracker->Info = nullptr;
    }

    auto event = NavigateEvent::Create(init, krys::move(abortController));
    _ongoingNavigateEvent = event;
    _focusChangedDuringOngoingNavigation = FocusDidChange::No;
    _suppressNormalScrollRestorationDuringOngoingNavigation = false;

    Ref<Navigation> protectedThis = ShareRef(*this);
    (void)dom::EventDispatcher::DispatchToTarget(*event, this);

    // If the frame was detached in our event.
    if (!Frame())
    {
      AbortOngoingNavigation(*event);
      return DispatchResult::Aborted;
    }

    if (event->DefaultPrevented())
    {
      // FIXME: If navigationType is "traverse", then consume history-action user activation.
      if (!event->Signal()->Aborted())
      {
        AbortOngoingNavigation(*event);
      }
      return DispatchResult::Aborted;
    }

    bool endResultIsSameDocument = event->WasIntercepted() || destination->SameDocument();

    if (event->WasIntercepted())
    {
      event->SetInterceptionState(InterceptionState::Committed);

      RefPtr<NavigationHistoryEntry> fromNavigationHistoryEntry = ShareRefPtr(CurrentEntry());
      krys_debug_assert(fromNavigationHistoryEntry);
      _transition = NavigationTransition::Create(navigationType, ShareRef(*fromNavigationHistoryEntry));

      if (navigationType == NavigationNavigationType::Traverse)
      {
        _suppressNormalScrollRestorationDuringOngoingNavigation = true;
        // For intercepted traverse navigations, update the Navigation API state and fire currententrychange.
        // This must happen AFTER the navigate event but BEFORE intercept handlers run. The committed promise
        // is fulfilled now if there are no handlers, and after they're invoked if there are.
        if (destination->SameDocument())
        {
          if (RefPtr<NavigationHistoryEntry> entry = ShareRefPtr(FindEntryByKey(destination->Key())))
          {
            document->UpdateURLForPushOrReplaceState(destination->URL());
            auto shouldNotifyCommitted =
              event->Handlers().empty() ? ShouldNotifyCommitted::Yes : ShouldNotifyCommitted::No;
            UpdateForNavigation(ShareRef(entry->AssociatedHistoryItem()), navigationType,
                                ShouldCopyStateObjectFromCurrentEntry::No, shouldNotifyCommitted);
          }
        }
      }
      else if (navigationType == NavigationNavigationType::Reload)
      {
        // Not in specification but matches chromium implementation and tests.
        UpdateForNavigation(ShareRef(CurrentEntry()->AssociatedHistoryItem()), navigationType);
      }
      else
      {
        auto historyHandling = navigationType == NavigationNavigationType::Replace
                                 ? NavigationHistoryBehavior::Replace
                                 : NavigationHistoryBehavior::Push;
        Frame()->Loader().UpdateURLAndHistory(destination->URL(), ShareRefPtr(classicHistoryAPIState),
                                              historyHandling);
      }
    }

    if (endResultIsSameDocument)
    {
      auto completions = CreateRef<HandlerCompletions>();
      RefPtr<dom::AbortSignal> signal = ShareRefPtr(event->Signal());
      WeakPtr<Navigation> weakThis = CreateWeakPtr(this);

      completions->OnFulfilled = [weakThis, signal, document, apiMethodTracker]
      {
        RefPtr<Navigation> navigation = ShareRefPtr(weakThis.get());
        if (!navigation || signal->Aborted() || !document->IsFullyActive()
            || !navigation->_ongoingNavigateEvent)
        {
          return;
        }

        auto focusChanged =
          std::exchange(navigation->_focusChangedDuringOngoingNavigation, FocusDidChange::No);
        RefPtr<NavigateEvent> ongoingNavigateEvent = navigation->_ongoingNavigateEvent;
        ongoingNavigateEvent->Finish(*document, InterceptionHandlersDidFulfill::Yes, focusChanged);
        navigation->_ongoingNavigateEvent = nullptr;

        auto successEvent = dom::Event::Create(dom::DOMStringAtom(EventNames::NavigateSuccess), false, false);
        (void)dom::EventDispatcher::DispatchToTarget(*successEvent, navigation.get());

        if (apiMethodTracker)
        {
          navigation->ResolveFinishedPromise(apiMethodTracker.get());
        }

        if (RefPtr<NavigationTransition> transition = std::exchange(navigation->_transition, nullptr))
        {
          transition->ResolvePromise();
        }
      };

      completions->OnRejected = [weakThis, signal, document, apiMethodTracker](const dom::Exception &error)
      {
        RefPtr<Navigation> navigation = ShareRefPtr(weakThis.get());
        if (!navigation || signal->Aborted() || !document->IsFullyActive()
            || !navigation->_ongoingNavigateEvent)
        {
          return;
        }

        auto focusChanged =
          std::exchange(navigation->_focusChangedDuringOngoingNavigation, FocusDidChange::No);
        RefPtr<NavigateEvent> ongoingNavigateEvent = navigation->_ongoingNavigateEvent;
        ongoingNavigateEvent->Finish(*document, InterceptionHandlersDidFulfill::No, focusChanged);
        ongoingNavigateEvent->SignalAbort(error);

        navigation->_ongoingNavigateEvent = nullptr;

        navigation->DispatchNavigateErrorEvent(error);

        if (apiMethodTracker)
        {
          apiMethodTracker->FinishedPromise->Reject(dom::Exception(error));
        }

        if (RefPtr<NavigationTransition> transition = std::exchange(navigation->_transition, nullptr))
        {
          transition->RejectPromise(dom::Exception(error));
        }
      };

      auto &handlers = event->Handlers();
      completions->Remaining = handlers.size();
      for (auto &handler : handlers)
      {
        handler(
          [completions](dom::ExceptionOr<void> result)
          {
            if (completions->Settled)
            {
              return;
            }
            if (result.HasException())
            {
              if (!completions->Error)
              {
                completions->Error = result.ReleaseException();
              }
            }
            else if (completions->Remaining)
            {
              --completions->Remaining;
            }
            completions->TrySettle();
          });
      }

      // For intercepted traverse navigations, notify committed after handlers have been invoked but before
      // they complete. This ensures the correct event ordering.
      if (navigationType == NavigationNavigationType::Traverse && event->WasIntercepted() && apiMethodTracker
          && !apiMethodTracker->CommittedToEntry)
      {
        NotifyCommittedToEntry(apiMethodTracker.get(), CurrentEntry(), navigationType);
      }

      // The handlers' outcomes are waited for from a task, as waiting for all of their promises would.
      html::EventLoop::Current().QueueTask(html::TaskSource::DOMManipulation, *document,
                                           [completions]
                                           {
                                             completions->Waiting = true;
                                             completions->TrySettle();
                                           });

      // If a new event has been dispatched in our event handler then we were aborted above.
      if (_ongoingNavigateEvent != event.get())
      {
        return DispatchResult::Aborted;
      }
    }
    else if (!apiMethodTracker)
    {
      // FIXME: This situation isn't clear, we've made it through the event doing nothing so to avoid
      // incorrectly being aborted we clear this. For cross-document navigations with a tracker, the tracker
      // stays ongoing until the navigation completes, fails or is interrupted.
      _ongoingNavigateEvent = nullptr;
    }

    return event->WasIntercepted() ? DispatchResult::Intercepted : DispatchResult::Completed;
  }

  Navigation::DispatchResult Navigation::DispatchTraversalNavigateEvent(HistoryItem &historyItem) noexcept
  {
    auto *frame = Frame();
    auto *currentItem = frame ? frame->Loader().History().CurrentItem() : nullptr;
    bool isSameDocument =
      currentItem && currentItem->DocumentSequenceNumber() == historyItem.DocumentSequenceNumber();

    RefPtr<NavigationHistoryEntry> destinationEntry;
    for (auto &entry : _entries)
    {
      if (entry->AssociatedHistoryItem().ItemSequenceNumber() == historyItem.ItemSequenceNumber())
      {
        destinationEntry = entry;
        break;
      }
    }

    // FIXME: Set destinations state
    auto destination =
      NavigationDestination::Create(historyItem.URL(), krys::move(destinationEntry), isSameDocument);
    return InnerDispatchNavigateEvent(NavigationNavigationType::Traverse, krys::move(destination), null);
  }

  bool Navigation::DispatchPushReplaceReloadNavigateEvent(const url::URL &url,
                                                          NavigationNavigationType navigationType,
                                                          bool isSameDocument,
                                                          const FormSubmission *formSubmission,
                                                          html::SerializedScriptValue *classicHistoryAPIState,
                                                          dom::Element *sourceElement) noexcept
  {
    auto destination = NavigationDestination::Create(url, nullptr, isSameDocument);
    if (classicHistoryAPIState)
    {
      destination->SetStateObject(classicHistoryAPIState);
    }

    if (navigationType == NavigationNavigationType::Reload)
    {
      formSubmission = nullptr;
      sourceElement = nullptr;
    }

    return InnerDispatchNavigateEvent(navigationType, krys::move(destination), null, formSubmission,
                                      classicHistoryAPIState, sourceElement)
           == DispatchResult::Completed;
  }

  List<Ref<HistoryItem>> Navigation::FilterHistoryItemsForNavigationAPI(List<Ref<HistoryItem>> &&allItems,
                                                                        HistoryItem &currentItem) noexcept
  {
    auto starting =
      std::ranges::find_if(allItems, [&](auto &item) { return item->ItemID() == currentItem.ItemID(); });
    if (starting == allItems.end())
    {
      return {ShareRef(currentItem)};
    }

    auto startingIndex = static_cast<size_t>(starting - allItems.begin());
    List<Ref<HistoryItem>> filteredItems;
    for (auto i = static_cast<int64>(startingIndex) - 1; i >= 0; --i)
    {
      auto &item = allItems[static_cast<size_t>(i)];
      if (!IsSameOrigin(item->URL(), currentItem.URL()))
      {
        break;
      }
      filteredItems.push_back(item);
    }
    std::ranges::reverse(filteredItems);
    filteredItems.push_back(ShareRef(currentItem));

    for (auto i = startingIndex + 1; i < allItems.size(); ++i)
    {
      auto &item = allItems[i];
      if (!IsSameOrigin(item->URL(), currentItem.URL()))
      {
        break;
      }
      filteredItems.push_back(item);
    }
    return filteredItems;
  }
}
