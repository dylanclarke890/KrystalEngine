#pragma once

#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/SerializedScriptValue.hpp"
#include "Krystal.Booey/Loader/FrameLoaderTypes.hpp"
#include "Krystal.Booey/Page/NavigateEvent.hpp"
#include "Krystal.Booey/Page/NavigationActivation.hpp"
#include "Krystal.Booey/Page/NavigationHistoryEntry.hpp"
#include "Krystal.Booey/Page/NavigationNavigationType.hpp"
#include "Krystal.Booey/Page/NavigationPromise.hpp"
#include "Krystal.Booey/Page/NavigationTransition.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <chrono>

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo
{
  class FormSubmission;
  class HistoryItem;
  class LocalDOMWindow;
  class LocalFrame;

  /// @brief The promises of a Navigation API method's navigation, until they settle.
  /// @see https://html.spec.whatwg.org/#navigation-api-method-tracker
  struct NavigationAPIMethodTracker final : public RefCounted<NavigationAPIMethodTracker>
  {
    bool FinishedBeforeCommit {false};
    dom::DOMString Key;
    RefPtr<html::SerializedScriptValue> Info;
    RefPtr<html::SerializedScriptValue> SerializedState;
    RefPtr<NavigationHistoryEntry> CommittedToEntry;
    Ref<NavigationPromise> CommittedPromise;
    Ref<NavigationPromise> FinishedPromise;

    NavigationAPIMethodTracker(RefPtr<html::SerializedScriptValue> &&info,
                               RefPtr<html::SerializedScriptValue> &&serializedState) noexcept
        : Info(krys::move(info)), SerializedState(krys::move(serializedState)),
          CommittedPromise(NavigationPromise::Create()), FinishedPromise(NavigationPromise::Create())
    {
    }
  };

  enum class ShouldCopyStateObjectFromCurrentEntry : bool
  {
    No,
    Yes
  };

  /// @brief Whether `UpdateForNavigation` fulfills the committed promise at once.
  enum class ShouldNotifyCommitted : bool
  {
    No,
    Yes
  };

  /// @brief A window's `navigation`: the session history entries of its document's origin, the current one,
  /// and the navigations it starts (`navigate()`, `reload()`, `traverseTo()`, `back()`, `forward()`), each
  /// with promises of when it commits and finishes. Every navigation of the frame fires `navigate` at it
  /// first, which can cancel or intercept it.
  /// @note Script values (states, `info`) are `SerializedScriptValue`s, and promises are
  /// `NavigationPromise`s.
  /// @see https://html.spec.whatwg.org/#navigation-api
  class Navigation final : public dom::EventTarget
  {
  public:
    using HistoryBehavior = NavigationHistoryBehavior;

    /// @see https://html.spec.whatwg.org/#navigationupdatecurrententryoptions
    struct UpdateCurrentEntryOptions
    {
      RefPtr<html::SerializedScriptValue> State;
    };

    /// @see https://html.spec.whatwg.org/#navigationoptions
    struct Options
    {
      RefPtr<html::SerializedScriptValue> Info;
    };

    /// @see https://html.spec.whatwg.org/#navigationnavigateoptions
    struct NavigateOptions : Options
    {
      RefPtr<html::SerializedScriptValue> State;
      HistoryBehavior History {HistoryBehavior::Auto};
    };

    /// @see https://html.spec.whatwg.org/#navigationreloadoptions
    struct ReloadOptions : Options
    {
      RefPtr<html::SerializedScriptValue> State;
    };

    enum class DispatchResult : uint8
    {
      Completed,
      Aborted,
      Intercepted
    };

    /// @brief Marked if the ongoing navigation is aborted while it's registered.
    class AbortHandler : public RefCounted<AbortHandler>, public krys::CanMakeWeakPtr<AbortHandler>
    {
      friend class Navigation;

    private:
      bool _wasAborted {false};

    public:
      KRYS_NODISCARD bool WasAborted() const noexcept
      {
        return _wasAborted;
      }
    };

  private:
    /// @brief Allows 200 navigations by the Navigation API's methods each 10 seconds (of the host's time).
    struct RateLimiter
    {
      uint32 MaxNavigationsPerWindow {200};
      MonotonicTime::duration WindowDuration {std::chrono::seconds(10)};
      MonotonicTime::duration WindowStartTime {};
      uint32 NavigationCount {0};

      KRYS_NODISCARD bool NavigationAllowed() noexcept;
    };

    WeakPtr<LocalDOMWindow> _window;
    Maybe<size_t> _currentEntryIndex;
    RefPtr<NavigationTransition> _transition;
    RefPtr<NavigationActivation> _activation;
    List<Ref<NavigationHistoryEntry>> _entries;
    RefPtr<NavigateEvent> _ongoingNavigateEvent;
    FocusDidChange _focusChangedDuringOngoingNavigation {FocusDidChange::No};
    bool _suppressNormalScrollRestorationDuringOngoingNavigation {false};
    RefPtr<NavigationAPIMethodTracker> _ongoingAPIMethodTracker;
    RefPtr<NavigationAPIMethodTracker> _upcomingNonTraverseMethodTracker;
    HashMap<dom::DOMString, Ref<NavigationAPIMethodTracker>> _upcomingTraverseMethodTrackers;
    List<WeakPtr<AbortHandler>> _abortHandlers;
    RateLimiter _rateLimiter;

  public:
    explicit Navigation(LocalDOMWindow &window) noexcept;
    ~Navigation() noexcept override;

    KRYS_NODISCARD static Ref<Navigation> Create(LocalDOMWindow &window) noexcept
    {
      return CreateRef<Navigation>(window);
    }

    KRYS_NODISCARD LocalDOMWindow *Window() const noexcept
    {
      return _window.get();
    }

    /// @brief The entries: none if they're disabled (the document isn't fully active, or it's the initial
    /// `about:blank`).
    /// @see https://html.spec.whatwg.org/#dom-navigation-entries
    KRYS_NODISCARD const List<Ref<NavigationHistoryEntry>> &Entries() const noexcept;
    /// @see https://html.spec.whatwg.org/#dom-navigation-currententry
    KRYS_NODISCARD NavigationHistoryEntry *CurrentEntry() const noexcept;
    /// @see https://html.spec.whatwg.org/#dom-navigation-transition
    KRYS_NODISCARD NavigationTransition *Transition() const noexcept
    {
      return _transition.get();
    }
    /// @see https://html.spec.whatwg.org/#dom-navigation-activation
    KRYS_NODISCARD NavigationActivation *Activation() const noexcept
    {
      return _activation.get();
    }
    /// @see https://html.spec.whatwg.org/#dom-navigation-cangoback
    KRYS_NODISCARD bool CanGoBack() const noexcept;
    /// @see https://html.spec.whatwg.org/#dom-navigation-cangoforward
    KRYS_NODISCARD bool CanGoForward() const noexcept;

    /// @brief Gives the window's new document its entries: the previous window's (for a document of the same
    /// origin), updated for the navigation of @p navigationType, or one for the current item.
    /// @see https://html.spec.whatwg.org/#initialize-the-navigation-api-entries-for-a-new-document
    void InitializeForNewWindow(Maybe<NavigationNavigationType> navigationType,
                                LocalDOMWindow *previousWindow) noexcept;

    /// @brief Navigates to @p url (relative to the document's base URL).
    /// @see https://html.spec.whatwg.org/#dom-navigation-navigate
    NavigationResult Navigate(dom::DOMStringView url, NavigateOptions &&options = {}) noexcept;
    /// @see https://html.spec.whatwg.org/#dom-navigation-reload
    NavigationResult Reload(ReloadOptions &&options = {}) noexcept;
    /// @brief Goes to the entry whose key is @p key.
    /// @see https://html.spec.whatwg.org/#dom-navigation-traverseto
    NavigationResult TraverseTo(dom::DOMStringView key, Options &&options = {}) noexcept;
    /// @see https://html.spec.whatwg.org/#dom-navigation-back
    NavigationResult Back(Options &&options = {}) noexcept;
    /// @see https://html.spec.whatwg.org/#dom-navigation-forward
    NavigationResult Forward(Options &&options = {}) noexcept;

    /// @brief Gives the current entry @p options' state, and fires `currententrychange`.
    /// @return `InvalidStateError` if there's no current entry.
    /// @see https://html.spec.whatwg.org/#dom-navigation-updatecurrententry
    dom::ExceptionOr<void> UpdateCurrentEntry(UpdateCurrentEntryOptions &&options) noexcept;

    /// @see https://html.spec.whatwg.org/#fire-a-traverse-navigate-event
    DispatchResult DispatchTraversalNavigateEvent(HistoryItem &historyItem) noexcept;
    /// @return Whether the navigation continues as a load: not if it was canceled or intercepted.
    /// @see https://html.spec.whatwg.org/#fire-a-push/replace/reload-navigate-event
    bool DispatchPushReplaceReloadNavigateEvent(const url::URL &url, NavigationNavigationType navigationType,
                                                bool isSameDocument, const FormSubmission *formSubmission,
                                                html::SerializedScriptValue *classicHistoryAPIState = nullptr,
                                                dom::Element *sourceElement = nullptr) noexcept;

    /// @see https://html.spec.whatwg.org/#update-the-navigation-api-entries-for-a-same-document-navigation
    void
      UpdateForNavigation(Ref<HistoryItem> &&item, NavigationNavigationType navigationType,
                          ShouldCopyStateObjectFromCurrentEntry shouldCopyStateObjectFromCurrentEntry =
                            ShouldCopyStateObjectFromCurrentEntry::No,
                          ShouldNotifyCommitted shouldNotifyCommitted = ShouldNotifyCommitted::Yes) noexcept;
    /// @see https://html.spec.whatwg.org/#update-the-navigation-api-entries-for-reactivation
    void UpdateForReactivation(List<Ref<HistoryItem>> &&newHistoryItems, HistoryItem &reactivatedItem,
                               HistoryItem *previousItem) noexcept;

    /// @see https://html.spec.whatwg.org/#inform-the-navigation-api-about-aborting-navigation
    void AbortOngoingNavigationIfNeeded() noexcept;

    KRYS_NODISCARD NavigationHistoryEntry *FindEntryByKey(dom::DOMStringView key) const noexcept;

    KRYS_NODISCARD bool SuppressNormalScrollRestoration() const noexcept
    {
      return _suppressNormalScrollRestorationDuringOngoingNavigation;
    }

    /// @brief Notes whether the focus moved while a navigation is ongoing (its handlers moved it).
    void SetFocusChanged(FocusDidChange changed) noexcept
    {
      _focusChangedDuringOngoingNavigation = changed;
    }

    /// @brief Rejects @p apiMethodTracker's promises: its navigation was aborted.
    void RejectFinishedPromise(NavigationAPIMethodTracker *apiMethodTracker) noexcept;

    KRYS_NODISCARD NavigationAPIMethodTracker *
      UpcomingTraverseMethodTracker(dom::DOMStringView key) const noexcept;

    KRYS_NODISCARD Ref<AbortHandler> RegisterAbortHandler() noexcept;

    KRYS_NODISCARD NavigateEvent *OngoingNavigateEvent() const noexcept
    {
      return _ongoingNavigateEvent.get();
    }

    KRYS_NODISCARD bool HasInterceptedOngoingNavigateEvent() const noexcept
    {
      return _ongoingNavigateEvent && _ongoingNavigateEvent->WasIntercepted();
    }

    void UpdateNavigationEntry(Ref<HistoryItem> &&item, ShouldCopyStateObjectFromCurrentEntry) noexcept;

    /// @brief @p allItems around @p currentItem (as far as they're of its origin).
    /// @see https://html.spec.whatwg.org/#getting-session-history-entries-for-the-navigation-api
    KRYS_NODISCARD static List<Ref<HistoryItem>>
      FilterHistoryItemsForNavigationAPI(List<Ref<HistoryItem>> &&allItems,
                                         HistoryItem &currentItem) noexcept;

  private:
    KRYS_NODISCARD LocalFrame *Frame() const noexcept;

    /// @see https://html.spec.whatwg.org/#has-entries-and-events-disabled
    KRYS_NODISCARD bool HasEntriesAndEventsDisabled() const noexcept;

    NavigationResult PerformTraversal(dom::DOMStringView key, Options &&options) noexcept;
    DispatchResult InnerDispatchNavigateEvent(NavigationNavigationType navigationType,
                                              Ref<NavigationDestination> &&destination,
                                              Maybe<dom::DOMString> &&downloadRequestFilename,
                                              const FormSubmission *formSubmission = nullptr,
                                              html::SerializedScriptValue *classicHistoryAPIState = nullptr,
                                              dom::Element *sourceElement = nullptr) noexcept;
    void SetActivation(HistoryItem *previousItem, Maybe<NavigationNavigationType> navigationType) noexcept;
    RefPtr<NavigationAPIMethodTracker>
      MaybeSetUpcomingNonTraversalTracker(RefPtr<html::SerializedScriptValue> &&info,
                                          RefPtr<html::SerializedScriptValue> &&serializedState) noexcept;
    RefPtr<NavigationAPIMethodTracker>
      AddUpcomingTraverseAPIMethodTracker(dom::DOMStringView key,
                                          RefPtr<html::SerializedScriptValue> &&info) noexcept;
    void CleanupAPIMethodTracker(NavigationAPIMethodTracker *apiMethodTracker) noexcept;
    void ResolveFinishedPromise(NavigationAPIMethodTracker *apiMethodTracker) noexcept;
    void RejectFinishedPromise(NavigationAPIMethodTracker *apiMethodTracker,
                               const dom::Exception &exception) noexcept;
    void AbortOngoingNavigation(NavigateEvent &event) noexcept;
    void PromoteUpcomingAPIMethodTracker(const Maybe<dom::DOMString> &destinationKey) noexcept;
    void NotifyCommittedToEntry(NavigationAPIMethodTracker *apiMethodTracker, NavigationHistoryEntry *entry,
                                NavigationNavigationType navigationType) noexcept;
    KRYS_NODISCARD static NavigationResult
      ApiMethodTrackerDerivedResult(const NavigationAPIMethodTracker &apiMethodTracker) noexcept;
    KRYS_NODISCARD Maybe<size_t> EntryIndexOfKey(dom::DOMStringView key) const noexcept;
    void DispatchNavigateErrorEvent(const dom::Exception &exception) noexcept;
  };
}
