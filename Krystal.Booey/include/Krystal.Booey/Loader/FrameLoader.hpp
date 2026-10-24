#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/History/HistoryItem.hpp"
#include "Krystal.Booey/Loader/Cache/CachePolicy.hpp"
#include "Krystal.Booey/Loader/FetchOptions.hpp"
#include "Krystal.Booey/Loader/FrameLoaderTypes.hpp"
#include "Krystal.Booey/Loader/LocalFrameLoaderClient.hpp"
#include "Krystal.Booey/Page/NavigationNavigationType.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  class Element;
  class Event;
}

namespace krys::boo::html
{
  class SerializedScriptValue;
}

namespace krys::boo::url
{
  class URL;
}

namespace krys::boo
{
  class CachedFrame;
  class LocalDOMWindow;
  class DocumentLoader;
  class FormSubmission;
  class FrameLoadRequest;
  class HistoryController;
  class LocalFrame;
  class NavigationAction;
  class ResourceError;
  class ResourceRequest;
  class SubstituteData;

  /// @brief Where a frame is in loading a document.
  enum class FrameState : uint8
  {
    /// @brief A load started; the frame still shows its old document.
    Provisional,
    /// @brief The new document is the frame's, and loading.
    CommittedPage,
    /// @brief Nothing is loading.
    Complete,
  };

  /// @brief Navigates a frame: links, form submissions and the host ask the frame's loader, which asks its
  /// client whether to navigate, navigates within the document itself, and loads other documents through a
  /// document loader, provisionally until their data arrives and they commit; each load updates the page's
  /// session history, which the loader also goes back and forward through.
  /// @note FIXME: Redirects; new windows and other frames with a frame tree (not planned).
  class FrameLoader
  {
    KRYS_NON_COPY_MOVABLE(FrameLoader);

  private:
    LocalFrame &_frame;
    UniquePtr<LocalFrameLoaderClient> _client;
    UniquePtr<HistoryController> _history;
    /// @brief The history item the frame is going to (back or forward), until its document has loaded.
    RefPtr<HistoryItem> _requestedHistoryItem;
    FrameLoadType _loadType {FrameLoadType::Standard};
    /// @brief The loader of the frame's document.
    RefPtr<DocumentLoader> _documentLoader;
    /// @brief The loader of the document the frame is loading, until it commits.
    RefPtr<DocumentLoader> _provisionalDocumentLoader;
    FrameState _state {FrameState::Complete};
    /// @brief Whether the frame shows the initial empty document (`about:blank`) it was given.
    bool _isDisplayingInitialEmptyDocument {false};
    /// @brief Whether the document finished loading (its load event fired), or there's no load.
    bool _isComplete {true};
    /// @brief Whether the document's load event was fired (implicitClose).
    bool _didCallImplicitClose {true};
    /// @brief Whether the document's unload events were fired.
    bool _wasUnloadEventEmitted {false};
    /// @brief Whether the document's beforeunload or unload events are being dispatched.
    bool _pageDismissalEventBeingDispatched {false};
    /// @brief Whether the next load replaces the current history item: a scheduled navigation that does.
    bool _quickRedirectComing {false};
    /// @brief Whether the frame has committed a load other than its initial empty document.
    bool _committedFirstRealDocumentLoad {false};
    /// @brief Whether the current item's scroll position is to be restored at the next rendering update.
    bool _shouldRestoreScrollPositionAndViewState {false};
    /// @brief Whether the provisional load is a page the back/forward cache restores.
    bool _loadingFromCachedPage {false};
    /// @brief Fires `navigate` for the cross-document load that's starting, once the page that's left allows
    /// it; whether the load goes on.
    Func<bool()> _pendingDispatchNavigateEvent;
    /// @brief The history item a traversal to another document goes to, whose `navigate` fires once the page
    /// that's left allows it.
    RefPtr<HistoryItem> _pendingNavigationAPIItem;

  public:
    /// @param client The host's side of the loader; without one, the loader has an empty client, which
    /// ignores every navigation.
    FrameLoader(LocalFrame &frame, UniquePtr<LocalFrameLoaderClient> &&client) noexcept;
    ~FrameLoader() noexcept;

    KRYS_NODISCARD LocalFrameLoaderClient &Client() const noexcept
    {
      return *_client;
    }

    KRYS_NODISCARD LocalFrame &Frame() const noexcept
    {
      return _frame;
    }

    /// @brief The frame's part in the page's session history.
    KRYS_NODISCARD HistoryController &History() const noexcept
    {
      return *_history;
    }

    /// @brief The history item the frame is going to, until its document has loaded.
    KRYS_NODISCARD HistoryItem *RequestedHistoryItem() const noexcept
    {
      return _requestedHistoryItem.get();
    }

    /// @brief The type of the frame's current load.
    KRYS_NODISCARD FrameLoadType LoadType() const noexcept
    {
      return _loadType;
    }

    KRYS_NODISCARD FrameState State() const noexcept
    {
      return _state;
    }

    /// @brief The loader of the frame's document, if it has one.
    KRYS_NODISCARD DocumentLoader *GetDocumentLoader() const noexcept;

    /// @brief The loader of the document the frame is loading, if any.
    KRYS_NODISCARD DocumentLoader *ProvisionalDocumentLoader() const noexcept;

    /// @brief Whether a document is loading in the frame: provisionally, or its data still to come.
    KRYS_NODISCARD bool IsLoading() const noexcept;

    KRYS_NODISCARD bool IsDisplayingInitialEmptyDocument() const noexcept
    {
      return _isDisplayingInitialEmptyDocument;
    }

    /// @brief Whether the load committing is of a page from the back/forward cache, which is restored after
    /// the commit.
    KRYS_NODISCARD bool IsLoadingFromCachedPage() const noexcept
    {
      return _loadingFromCachedPage;
    }

    /// @brief Whether the frame has committed a load other than its initial empty document.
    KRYS_NODISCARD bool CommittedFirstRealDocumentLoad() const noexcept
    {
      return _committedFirstRealDocumentLoad;
    }

    /// @brief How the document's subresources use the memory cache, from the load type while the document
    /// loads: revalidated for a reload, kept for going back and forward.
    KRYS_NODISCARD CachePolicy SubresourceCachePolicy() const noexcept;

    /// @brief Gives the frame its initial empty document (`about:blank`), at once.
    /// @see https://html.spec.whatwg.org/#initialise-the-document-object
    void Init() noexcept;

    /// @brief Loads @p request into the frame, as the host asks: its URL, or the substitute data it carries.
    /// The client is asked first (the action is from the client), and the document commits on a later task.
    void Load(FrameLoadRequest &&request) noexcept;

    /// @brief Stops the frame's provisional load, if any; the client is told it failed (cancelled).
    /// @param clearProvisionalItem Whether the history item being loaded is forgotten: unless a new load is
    /// starting.
    void StopAllLoaders(ClearProvisionalItem clearProvisionalItem = ClearProvisionalItem::Yes) noexcept;

    void StopAllLoadersAndCheckCompleteness() noexcept;

    /// @brief Loads the document again, as the host asks; its form state isn't restored.
    void Reload(bool isRequestFromClientOrUserInput = false) noexcept;

    /// @brief Loads @p item into the frame (back or forward): a fragment or pushed state of the document
    /// navigates within it; another document loads, with the markup the item kept if the host gave it.
    void LoadItem(HistoryItem &item, HistoryItem *fromItem, FrameLoadType loadType) noexcept;

    /// @brief A scheduled navigation is about to happen: it replaces the current history item if
    /// @p lockBackForwardList, or if that's the only one and `about:blank`.
    void ClientRedirected(bool lockBackForwardList) noexcept;

    /// @brief The frame's document was laid out for the first time: going back or forward restores its scroll
    /// position at the next rendering update.
    void DidFirstLayout() noexcept;

    /// @brief Restores the current item's scroll position, if the first layout asked for it.
    void RestoreScrollPositionAndViewStateNowIfNeeded() noexcept;

    /// @brief Whether @p url is the current history item's.
    KRYS_NODISCARD bool ShouldTreatURLAsSameAsCurrent(const url::URL &url) const noexcept;

    /// @brief Whether the next load replaces the current history item (a scheduled navigation that does).
    KRYS_NODISCARD bool QuickRedirectComing() const noexcept
    {
      return _quickRedirectComing;
    }

    /// @brief Stops the document's load, firing the unload events @p unloadEventPolicy asks for; its
    /// scheduled navigations are cancelled.
    void StopLoading(UnloadEventPolicy unloadEventPolicy) noexcept;

    /// @brief Stops the document's loads and scheduled navigations, as its page enters the back/forward
    /// cache.
    void StopForBackForwardCache() noexcept;

    /// @brief Shows @p cachedFrame's document in the frame again, with its view.
    void Open(CachedFrame &cachedFrame) noexcept;

    /// @brief Gives the document @p newURL and adds (or replaces, for @p historyHandling `Replace`) its
    /// history entry with @p stateObject, without a load.
    /// @see https://html.spec.whatwg.org/#url-and-history-update-steps
    void UpdateURLAndHistory(const url::URL &newURL, RefPtr<html::SerializedScriptValue> &&stateObject,
                             NavigationHistoryBehavior historyHandling) noexcept;

    /// @brief Fires `beforeunload` at the window; if the page asks to stay and the user has interacted with
    /// it, the host is asked to confirm leaving.
    /// @return Whether the document may be left.
    /// @see https://html.spec.whatwg.org/#checking-if-unloading-is-canceled
    KRYS_NODISCARD bool ShouldClose() noexcept;

    /// @brief Navigates to @p url, as following a link does.
    /// @param passedTarget The browsing context to navigate (`_self` and the others, or a name); empty for
    /// the document's base target.
    /// @param triggeringEvent The event that started the navigation (a click), if any.
    /// @param openerPolicy Whether a new browsing context gets an opener; by default, unless the referrer
    /// policy is `no-referrer`.
    /// @param downloadAttribute The link's `download` attribute; null if it has none.
    /// @param sourceElement The link, if any.
    void ChangeLocation(const url::URL &url, dom::DOMStringAtom passedTarget, dom::Event *triggeringEvent,
                        ReferrerPolicy referrerPolicy, Maybe<NewFrameOpenerPolicy> openerPolicy,
                        dom::DOMStringAtom downloadAttribute,
                        NavigationHistoryBehavior historyBehavior = NavigationHistoryBehavior::Auto,
                        dom::Element *sourceElement = nullptr) noexcept;

    /// @brief Navigates as @p frameRequest asks, to a `GET` request.
    void ChangeLocation(FrameLoadRequest &&frameRequest, dom::Event *triggeringEvent = nullptr) noexcept;

    /// @brief Schedules the navigation to a form submission (a `GET` or a `POST`), which happens on a later
    /// task.
    void SubmitForm(Ref<FormSubmission> &&submission) noexcept;

    /// @brief Loads @p request into the browsing context it names, once the client allows it.
    /// @param formSubmission The form submitted, for a form submission.
    void LoadFrameRequest(FrameLoadRequest &&request, dom::Event *event,
                          RefPtr<FormSubmission> &&formSubmission) noexcept;

    /// @brief The frame a navigation targeting @p name navigates: this frame for an empty name, `_self`,
    /// `_parent`, `_top` (a frame is the only one in its tree) and the frame's own name; null for `_blank`
    /// and other names, which would open a new browsing context.
    KRYS_NODISCARD LocalFrame *FindFrameForNavigation(dom::DOMStringView name) const noexcept;

    /// @brief Run when the parser stops parsing the frame's document: scrolls to its URL's fragment.
    void FinishedParsing() noexcept;

    /// @brief The provisional load's data arrived: it becomes the frame's document loader.
    void CommitProvisionalLoad() noexcept;

    /// @brief The committed document was made and is the frame's: the client sets it up (custom elements)
    /// before it's parsed, and hears that the load committed.
    /// @param previousWindow The window of the document the frame showed before, if any.
    void DidBeginDocument(LocalDOMWindow *previousWindow = nullptr) noexcept;

    /// @brief Stops the frame's loads, as the host asks, and aborts the ongoing navigation.
    void StopForUserCancel() noexcept;

    /// @brief The Navigation API's type of a navigation of @p loadType, with @p historyHandling.
    KRYS_NODISCARD static NavigationNavigationType
      DetermineNavigationType(FrameLoadType loadType, NavigationHistoryBehavior historyHandling) noexcept;

    /// @brief @p loader's document has all its data.
    void FinishedLoading(DocumentLoader &loader) noexcept;

    /// @brief A load of the document (a subresource) is done: checks whether the document has loaded.
    void LoadDone() noexcept;

    /// @brief Whether the document has loaded (parsed, its subresources and anything else delaying its load
    /// event done), and if so, fires its load event and tells the client.
    /// @see https://html.spec.whatwg.org/#the-end
    void CheckCompleted() noexcept;

    KRYS_NODISCARD bool IsComplete() const noexcept
    {
      return _isComplete;
    }

    /// @brief @p loader's main resource couldn't be loaded (or the load was stopped).
    void ReceivedMainResourceError(DocumentLoader &loader, const ResourceError &error) noexcept;

    /// @brief Whether navigating from @p currentURL to @p destinationURL loads a document, rather than
    /// navigating to a fragment: unless @p destinationURL has a fragment and is otherwise @p currentURL.
    KRYS_NODISCARD static bool ShouldReload(const url::URL &currentURL,
                                            const url::URL &destinationURL) noexcept;

  private:
    void LoadURL(FrameLoadRequest &&frameLoadRequest, FrameLoadType newLoadType, dom::Event *event,
                 RefPtr<FormSubmission> &&formSubmission) noexcept;

    /// @brief Loads @p request (or @p substituteData) as a new document, which @p action started and the
    /// client allowed. A reload of a document the host gave the markup of loads that markup again.
    /// @return Whether the load started: not if the page asked to stay.
    bool LoadWithNavigationAction(const ResourceRequest &request, NavigationAction &&action,
                                  FrameLoadType loadType, Maybe<SubstituteData> &&substituteData) noexcept;

    /// @brief Loads into the frame as @p loader loads (a new loader, or one the back/forward cache kept),
    /// once the page that's left lets it.
    /// @return Whether the load started.
    bool LoadWithDocumentLoader(DocumentLoader &loader, FrameLoadType loadType) noexcept;

    /// @brief Commits the provisional load at once, from the page the back/forward cache kept for its item.
    void LoadProvisionalItemFromCachedPage() noexcept;

    void LoadSameDocumentItem(HistoryItem &item) noexcept;
    void LoadDifferentDocumentItem(HistoryItem &item, HistoryItem *fromItem, FrameLoadType loadType) noexcept;

    /// @brief The committed document gets a view of its own, as a new page's would: scrolled to its top, not
    /// yet laid out.
    void TransitionToCommittedForNewPage() noexcept;

    void SetState(FrameState newState) noexcept;
    void FrameLoadCompleted() noexcept;
    void RestoreScrollPositionAndViewStateSoon() noexcept;

    /// @brief Whether the load is complete, and if so, tells the client.
    void CheckLoadComplete() noexcept;

    /// @brief Fires the document's load event, once it's parsed and nothing delays it.
    void CheckCallImplicitClose() noexcept;

    /// @brief Fires `navigate` at the window's navigation for a push, replace or reload to @p newURL.
    /// @return Whether the navigation goes on.
    bool DispatchNavigateEvent(FrameLoadType loadType, NavigationHistoryBehavior historyHandling,
                               const url::URL &newURL, dom::DOMStringAtom downloadAttribute,
                               bool isSameDocument, const FormSubmission *formSubmission, dom::Event *event,
                               html::SerializedScriptValue *classicHistoryAPIState = nullptr) noexcept;

    /// @brief Fires `beforeunload` at the document's window, and asks the host if the page asks to stay.
    KRYS_NODISCARD bool DispatchBeforeUnloadEvent() noexcept;

    /// @brief Fires `pagehide` (persisted if the document is entering the back/forward cache) and `unload`
    /// (unless it is) at the document's window, as it's left.
    /// @see https://html.spec.whatwg.org/#unload-a-document
    void DispatchUnloadEvents(UnloadEventPolicy unloadEventPolicy) noexcept;

    void LoadPostRequest(FrameLoadRequest &&request, FrameLoadType loadType, dom::Event *event,
                         RefPtr<FormSubmission> &&formSubmission) noexcept;

    /// @brief Asks the client to open a new browsing context for @p action.
    void CheckNewWindowPolicy(const NavigationAction &action, const ResourceRequest &request,
                              const FormSubmission *formSubmission, dom::DOMStringView frameName) noexcept;

    KRYS_NODISCARD bool ShouldPerformFragmentNavigation(bool isFormSubmission, utf8_stringview httpMethod,
                                                        FrameLoadType loadType,
                                                        const url::URL &url) const noexcept;

    void ContinueFragmentScrollAfterNavigationPolicy(const ResourceRequest &request, bool shouldContinue,
                                                     NavigationHistoryBehavior historyHandling) noexcept;

    /// @brief Navigates to @p url, which differs from the document's URL at most in its fragment (or which a
    /// history item with @p stateObject has).
    void LoadInSameDocument(const url::URL &url, html::SerializedScriptValue *stateObject,
                            bool isNewNavigation, NavigationHistoryBehavior historyHandling) noexcept;

    /// @brief Scrolls to @p url's fragment, once the document's style sheets have loaded.
    /// @see https://html.spec.whatwg.org/#try-to-scroll-to-the-fragment
    void ScrollToFragmentWithParentBoundary(const url::URL &url, bool isNewNavigation = true) noexcept;
  };
}
