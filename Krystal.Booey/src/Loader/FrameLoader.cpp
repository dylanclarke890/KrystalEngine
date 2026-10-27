#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/History/BackForwardCache.hpp"
#include "Krystal.Booey/History/BackForwardController.hpp"
#include "Krystal.Booey/History/CachedPage.hpp"
#include "Krystal.Booey/History/HistoryItem.hpp"
#include "Krystal.Booey/HTML/BeforeUnloadEvent.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/SerializedScriptValue.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/DocumentLoader.hpp"
#include "Krystal.Booey/Loader/FormSubmission.hpp"
#include "Krystal.Booey/Loader/FrameLoadRequest.hpp"
#include "Krystal.Booey/Loader/HistoryController.hpp"
#include "Krystal.Booey/Loader/NavigationAction.hpp"
#include "Krystal.Booey/Loader/NavigationScheduler.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Navigation.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  FrameLoader::FrameLoader(LocalFrame &frame, UniquePtr<LocalFrameLoaderClient> &&client) noexcept
      : _frame(frame), _client(client ? krys::move(client) : CreateUnique<EmptyFrameLoaderClient>()),
        _history(CreateUnique<HistoryController>(frame))
  {
  }

  FrameLoader::~FrameLoader() noexcept
  {
    if (_provisionalDocumentLoader)
    {
      _provisionalDocumentLoader->DetachFromFrame();
    }
    if (_documentLoader)
    {
      _documentLoader->DetachFromFrame();
    }
  }

  DocumentLoader *FrameLoader::GetDocumentLoader() const noexcept
  {
    return _documentLoader.get();
  }

  DocumentLoader *FrameLoader::ProvisionalDocumentLoader() const noexcept
  {
    return _provisionalDocumentLoader.get();
  }

  bool FrameLoader::IsLoading() const noexcept
  {
    return _provisionalDocumentLoader || (_documentLoader && _documentLoader->IsLoadingMainResource())
           || _state != FrameState::Complete;
  }

  void FrameLoader::Init() noexcept
  {
    // This somewhat odd set of steps gives the frame an initial empty document.
    auto loader = DocumentLoader::Create(ResourceRequest(url::URL(u8"about:blank")), SubstituteData());
    loader->AttachToFrame(_frame);
    _documentLoader = loader;
    _isDisplayingInitialEmptyDocument = true;
    loader->CommitInitialEmptyDocument();
    _state = FrameState::Complete;
  }

  CachePolicy FrameLoader::SubresourceCachePolicy() const noexcept
  {
    if (_isComplete)
    {
      return CachePolicy::Verify;
    }

    switch (_loadType)
    {
      case FrameLoadType::Reload:                            return CachePolicy::Revalidate;
      case FrameLoadType::Back:
      case FrameLoadType::Forward:
      case FrameLoadType::IndexedBackForward:                return CachePolicy::HistoryBuffer;
      case FrameLoadType::ReloadFromOrigin:                  return CachePolicy::Reload;
      case FrameLoadType::RedirectWithLockedBackForwardList:
      case FrameLoadType::MultipartReplace:
      case FrameLoadType::NavigationAPIReplace:
      case FrameLoadType::Same:
      case FrameLoadType::Standard:                          return CachePolicy::Verify;
      case FrameLoadType::ReloadExpiredOnly:
        // We know about expiration for HTTP and data. Do a normal reload otherwise.
        // FIXME: The document's URL's scheme, once memory cache entries expire by it.
        return CachePolicy::Verify;
    }
    return CachePolicy::Verify;
  }

  void FrameLoader::Load(FrameLoadRequest &&request) noexcept
  {
    auto &resourceRequest = request.GetResourceRequest();
    NavigationAction action(resourceRequest, FrameLoadType::Standard, false, nullptr,
                            dom::DOMStringAtom::Null(), request.SourceElement());
    action.SetIsRequestFromClientOrUserInput(true);

    // A load of a fragment of the document's URL (without data of its own) navigates within the document.
    auto *document = _frame.Document();
    if (!request.GetSubstituteData() && document
        && ShouldPerformFragmentNavigation(false, resourceRequest.HTTPMethod(), FrameLoadType::Standard,
                                           resourceRequest.URL()))
    {
      auto policy = _client->DispatchDecidePolicyForNavigationAction(action, resourceRequest, nullptr);
      ContinueFragmentScrollAfterNavigationPolicy(resourceRequest, policy == PolicyAction::Use,
                                                  NavigationHistoryBehavior::Push);
      return;
    }

    auto policy = _client->DispatchDecidePolicyForNavigationAction(action, resourceRequest, nullptr);
    if (policy != PolicyAction::Use)
    {
      return;
    }

    // Loading the current item's URL again replaces the item.
    auto type =
      ShouldTreatURLAsSameAsCurrent(resourceRequest.URL()) ? FrameLoadType::Same : FrameLoadType::Standard;
    action.SetNavigationAPIType(DetermineNavigationType(type, type == FrameLoadType::Same
                                                                ? NavigationHistoryBehavior::Replace
                                                                : NavigationHistoryBehavior::Push));
    auto substituteData = request.GetSubstituteData();
    (void)LoadWithNavigationAction(resourceRequest, krys::move(action), type, krys::move(substituteData));
  }

  bool FrameLoader::LoadWithNavigationAction(const ResourceRequest &request, NavigationAction &&action,
                                             FrameLoadType loadType,
                                             Maybe<SubstituteData> &&substituteData) noexcept
  {
    // Reloading a document the host gave the markup of loads the markup again.
    if (!substituteData && IsReload(loadType) && _documentLoader && _documentLoader->GetSubstituteData()
        && request.URL() == _documentLoader->DocumentURL())
    {
      substituteData = _documentLoader->GetSubstituteData();
    }

    auto loader = DocumentLoader::Create(ResourceRequest(request), krys::move(substituteData));
    loader->SetTriggeringAction(krys::move(action));
    return LoadWithDocumentLoader(*loader, loadType);
  }

  bool FrameLoader::LoadWithDocumentLoader(DocumentLoader &loader, FrameLoadType loadType) noexcept
  {
    Ref<DocumentLoader> protectedLoader = ShareRef(loader);

    auto pendingDispatchNavigateEvent = std::exchange(_pendingDispatchNavigateEvent, nullptr);

    // Check if unloading is canceled: the page may ask to stay.
    if (!ShouldClose())
    {
      _pendingNavigationAPIItem = nullptr;
      return false;
    }

    // For a traversal, the navigate event fires after beforeunload (and after a page from the back/forward
    // cache is restored).
    if (RefPtr<HistoryItem> item = _pendingNavigationAPIItem)
    {
      auto *provisionalItem = _history->ProvisionalItem();
      bool willLoadFromBackForwardCache = provisionalItem && provisionalItem->IsInBackForwardCache();
      if (!willLoadFromBackForwardCache)
      {
        _pendingNavigationAPIItem = nullptr;
        auto *window = _frame.Document() ? _frame.Document()->Window() : nullptr;
        if (window
            && window->GetNavigation().DispatchTraversalNavigateEvent(*item)
                 == Navigation::DispatchResult::Aborted)
        {
          return false;
        }
      }
    }
    else if (pendingDispatchNavigateEvent && !pendingDispatchNavigateEvent())
    {
      return false;
    }

    // A new navigation is in progress, so don't clear the history's provisional item.
    StopAllLoaders(ClearProvisionalItem::No);

    _loadType = loadType;
    loader.AttachToFrame(_frame);
    _provisionalDocumentLoader = protectedLoader;
    SetState(FrameState::Provisional);

    _client->DispatchDidStartProvisionalLoad();

    // The client may have stopped the load.
    if (_provisionalDocumentLoader != &loader)
    {
      return true;
    }

    if (IsBackForwardLoadType(loadType))
    {
      if (auto *provisionalItem = _history->ProvisionalItem();
          provisionalItem && provisionalItem->IsInBackForwardCache())
      {
        LoadProvisionalItemFromCachedPage();
        return true;
      }
    }

    _loadingFromCachedPage = false;
    loader.StartLoadingMainResource();
    return true;
  }

  void FrameLoader::LoadProvisionalItemFromCachedPage() noexcept
  {
    _loadingFromCachedPage = true;
    CommitProvisionalLoad();
  }

  void FrameLoader::StopAllLoaders(ClearProvisionalItem clearProvisionalItem) noexcept
  {
    // If no new load is in progress, we should clear the provisional item from history before we call
    // stopLoading.
    if (clearProvisionalItem == ClearProvisionalItem::Yes)
    {
      _history->SetProvisionalItem(nullptr);
    }

    if (RefPtr<DocumentLoader> provisional = _provisionalDocumentLoader)
    {
      provisional->StopLoading();
    }
  }

  void FrameLoader::StopAllLoadersAndCheckCompleteness() noexcept
  {
    StopAllLoaders();
    CheckLoadComplete();
  }

  void FrameLoader::Reload(bool isRequestFromClientOrUserInput) noexcept
  {
    RefPtr<DocumentLoader> documentLoader = _documentLoader;
    if (!documentLoader)
    {
      return;
    }

    // If a window is created by javascript, its main frame can have an empty but non-nil URL. Reloading in
    // this case will lose the current contents.
    if (documentLoader->Request().URL().IsEmpty())
    {
      return;
    }

    // FIXME: Re-posting a POST's body (a FormResubmitted action), once requests carry them.
    ResourceRequest request(documentLoader->Request());
    NavigationAction action(request, FrameLoadType::Reload, false, nullptr);
    action.SetNavigationAPIType(NavigationNavigationType::Reload);
    action.SetIsRequestFromClientOrUserInput(
      documentLoader->TriggeringAction().IsRequestFromClientOrUserInput() || isRequestFromClientOrUserInput);

    auto policy = _client->DispatchDecidePolicyForNavigationAction(action, request, nullptr);
    if (policy != PolicyAction::Use)
    {
      return;
    }
    (void)LoadWithNavigationAction(request, krys::move(action), FrameLoadType::Reload, null);
  }

  void FrameLoader::ClientRedirected(bool lockBackForwardList) noexcept
  {
    // If a "quick" redirect comes in, we set a special mode so we treat the next load as part of the original
    // navigation. If we don't have a document loader, we have no "original" load on which to base a redirect,
    // so we treat the redirect as a normal load.
    _quickRedirectComing =
      (lockBackForwardList || _history->CurrentItemShouldBeReplaced()) && _documentLoader;
  }

  void FrameLoader::SetState(FrameState newState) noexcept
  {
    _state = newState;
    if (newState == FrameState::Complete)
    {
      FrameLoadCompleted();
    }
  }

  void FrameLoader::FrameLoadCompleted() noexcept
  {
    // Note: Can be called multiple times.
    _history->UpdateForFrameLoadCompleted();
  }

  void FrameLoader::DidFirstLayout() noexcept
  {
    if (_frame.GetPage() && IsBackForwardLoadType(_loadType))
    {
      RestoreScrollPositionAndViewStateSoon();
    }
  }

  void FrameLoader::RestoreScrollPositionAndViewStateSoon() noexcept
  {
    if (_shouldRestoreScrollPositionAndViewState)
    {
      return;
    }
    _shouldRestoreScrollPositionAndViewState = true;
    if (auto *document = _frame.Document())
    {
      document->ScheduleRenderingUpdate(RenderingUpdateStep::RestoreScrollPositionAndViewState);
    }
  }

  void FrameLoader::RestoreScrollPositionAndViewStateNowIfNeeded() noexcept
  {
    if (!_shouldRestoreScrollPositionAndViewState)
    {
      return;
    }
    _shouldRestoreScrollPositionAndViewState = false;
    // FIXME: Unless the Navigation API suppresses it (B3e).
    _history->RestoreScrollPositionAndViewState();
  }

  bool FrameLoader::ShouldTreatURLAsSameAsCurrent(const url::URL &url) const noexcept
  {
    auto *currentHistoryItem = _history->CurrentItem();
    return currentHistoryItem && url == currentHistoryItem->URL();
  }

  void FrameLoader::LoadSameDocumentItem(HistoryItem &item) noexcept
  {
    krys_debug_assert(_history->CurrentItem()
                      && item.DocumentSequenceNumber() == _history->CurrentItem()->DocumentSequenceNumber());

    Ref<LocalFrame> frame = ShareRef(_frame);

    // Save user view state to the current history item here since we don't do a normal load.
    _history->SaveScrollPositionAndViewStateToItem(_history->CurrentItem());
    if (RefPtr<LocalFrameView> view = ShareRefPtr(frame->View()))
    {
      view->SetLastUserScrollType(null);
    }

    _history->SetCurrentItem(ShareRef(item));

    // loadInSameDocument() actually changes the URL and notifies load delegates of a "fake" load
    LoadInSameDocument(item.URL(), item.StateObject(), false, NavigationHistoryBehavior::Auto);

    // Restore user view state from the current history item here since we don't do a normal load.
    auto *window = frame->Document() ? frame->Document()->Window() : nullptr;
    auto *navigation = window ? window->NavigationIfExists() : nullptr;
    if (!navigation || !navigation->SuppressNormalScrollRestoration())
    {
      _history->RestoreScrollPositionAndViewState();
    }
  }

  void FrameLoader::LoadDifferentDocumentItem(HistoryItem &item, HistoryItem *fromItem,
                                              FrameLoadType loadType) noexcept
  {
    Ref<LocalFrame> frame = ShareRef(_frame);

    // Remember this item so we can traverse any child items as child frames load
    _history->SetProvisionalItem(ShareRefPtr(&item));

    (void)fromItem;
    bool started = false;
    if (auto *cachedPage = BackForwardCache::Singleton().Get(item, frame->GetPage()))
    {
      // The page the back/forward cache kept loads with its own loader.
      RefPtr<DocumentLoader> documentLoader = ShareRefPtr(cachedPage->GetDocumentLoader());
      NavigationAction action(documentLoader->Request(), loadType, false, nullptr);
      action.SetNavigationAPIType(NavigationNavigationType::Traverse);
      auto policy =
        _client->DispatchDecidePolicyForNavigationAction(action, documentLoader->Request(), nullptr);
      if (policy == PolicyAction::Use)
      {
        documentLoader->SetTriggeringAction(krys::move(action));
        started = LoadWithDocumentLoader(*documentLoader, loadType);
      }
    }
    else
    {
      // FIXME: Re-posting a POST's body and the referrer, once requests carry them.
      ResourceRequest request(url::URL(item.URL()));
      ResourceRequest requestForOriginalURL(url::URL(item.OriginalURL()));
      NavigationAction action(requestForOriginalURL, loadType, false, nullptr);
      action.SetNavigationAPIType(NavigationNavigationType::Traverse);

      auto policy = _client->DispatchDecidePolicyForNavigationAction(action, request, nullptr);
      started = policy == PolicyAction::Use
                && LoadWithNavigationAction(request, krys::move(action), loadType,
                                            Maybe<SubstituteData>(item.GetSubstituteData()));
    }
    if (started)
    {
      return;
    }

    // If the navigation request came from the back/forward menu, and we punt on it, we have the problem that
    // we have optimistically moved the b/f cursor already, so move it back.
    if (auto *page = frame->GetPage())
    {
      if (RefPtr<HistoryItem> resetItem = ShareRefPtr(_history->CurrentItem()))
      {
        page->BackForward().SetCurrentItem(*resetItem);
      }
    }
  }

  // Loads content into this frame, as specified by history item
  void FrameLoader::LoadItem(HistoryItem &item, HistoryItem *fromItem, FrameLoadType loadType) noexcept
  {
    _requestedHistoryItem = ShareRefPtr(&item);
    RefPtr<HistoryItem> currentItem = ShareRefPtr(_history->CurrentItem());

    bool sameDocumentNavigation = currentItem && item.ShouldDoSameDocumentNavigationTo(*currentItem);

    // Only dispatch navigate event if this history item belongs to the current frame and the navigation is
    // same-origin.
    // FIXME: The same origin check, once there are origins.
    if (auto *window = _frame.Document() ? _frame.Document()->Window() : nullptr; window && fromItem)
    {
      if (sameDocumentNavigation)
      {
        // For same-document navigation, dispatch navigate event immediately.
        // https://html.spec.whatwg.org/multipage/nav-history-apis.html#fire-a-traverse-navigate-event
        Ref<Navigation> navigation = ShareRef(window->GetNavigation());
        if (navigation->DispatchTraversalNavigateEvent(item) == Navigation::DispatchResult::Aborted)
        {
          return;
        }
        // In case the event detached the frame.
        if (!_frame.GetPage())
        {
          return;
        }
      }
      else
      {
        // For cross-document navigation, save the item for later dispatch. Navigate event will be dispatched
        // after beforeunload.
        _pendingNavigationAPIItem = ShareRefPtr(&item);
      }
    }

    if (sameDocumentNavigation)
    {
      _loadType = loadType;
      LoadSameDocumentItem(item);
    }
    else
    {
      LoadDifferentDocumentItem(item, fromItem, loadType);
    }
  }

  void FrameLoader::TransitionToCommittedForNewPage() noexcept
  {
    RefPtr<LocalFrameView> oldView = ShareRefPtr(_frame.View());
    if (!oldView)
    {
      return;
    }
    _frame.CreateView(IntSize(oldView->Width(), oldView->Height()), oldView->FixedLayoutSize(),
                      oldView->UseFixedLayout());
  }

  void FrameLoader::CommitProvisionalLoad() noexcept
  {
    RefPtr<DocumentLoader> loader = _provisionalDocumentLoader;
    if (!loader)
    {
      return;
    }

    UniquePtr<CachedPage> cachedPage;
    if (_loadingFromCachedPage && _history->ProvisionalItem())
    {
      cachedPage = BackForwardCache::Singleton().Take(*_history->ProvisionalItem(), _frame.GetPage());
    }

    // Check to see if we need to cache the page we are navigating away from into the back/forward cache. We
    // are doing this here because we know for sure that a new page is about to be loaded.
    if (auto *currentItem = _history->CurrentItem();
        currentItem
        && (!_history->ProvisionalItem() || currentItem->ItemID() != _history->ProvisionalItem()->ItemID()))
    {
      (void)BackForwardCache::Singleton().AddIfCacheable(*currentItem, _frame.GetPage());
    }

    _history->UpdateForCommit();

    // The old document's form state is saved to its item, its scheduled navigations go with it, and it's
    // unloaded (beforeunload was asked before the load): only `unload` if it entered the back/forward cache,
    // which fired its `pagehide`.
    if (_documentLoader)
    {
      _history->SaveDocumentState();
      _frame.GetNavigationScheduler().Cancel();
      auto *currentDocument = _frame.Document();
      UnloadEventPolicy unloadEventPolicy;
      if (_frame.GetPage() && _frame.GetPage()->GetChromeClient().IsSVGImageChromeClient())
      {
        // If this is the SVGDocument of an SVGImage, no need to dispatch events or recalcStyle.
        unloadEventPolicy = UnloadEventPolicy::None;
      }
      else
      {
        unloadEventPolicy = currentDocument
                                && currentDocument->GetBackForwardCacheState()
                                     == dom::BackForwardCacheState::NotInBackForwardCache
                              ? UnloadEventPolicy::UnloadAndPageHide
                              : UnloadEventPolicy::UnloadOnly;
      }
      DispatchUnloadEvents(unloadEventPolicy);
    }
    // The unload handlers may have started another load.
    if (_provisionalDocumentLoader != loader)
    {
      return;
    }

    if (_documentLoader && _documentLoader != loader)
    {
      _documentLoader->DetachFromFrame();
    }
    _documentLoader = loader;
    _provisionalDocumentLoader = nullptr;

    // Nothing else can interrupt this commit - set the Provisional->Committed transition in stone
    SetState(FrameState::CommittedPage);
    _isDisplayingInitialEmptyDocument = false;
    _isComplete = false;
    _didCallImplicitClose = false;
    _wasUnloadEventEmitted = false;
    _shouldRestoreScrollPositionAndViewState = false;

    // Handle adding the URL to the back/forward list.
    switch (_loadType)
    {
      case FrameLoadType::Forward:
      case FrameLoadType::Back:
      case FrameLoadType::IndexedBackForward:
        if (_frame.GetPage())
        {
          // If the first load within a frame is a navigation within a back/forward list that was attached
          // without any of the items being loaded then we need to update the history in a similar manner as
          // for a standard load with the exception of updating the back/forward list.
          if (!_committedFirstRealDocumentLoad)
          {
            _history->UpdateForStandardLoad(
              HistoryController::HistoryUpdateType::UpdateAllExceptBackForwardList);
          }
          _history->UpdateForBackForwardNavigation();

          // A page from the back/forward cache brings its own view.
          if (!cachedPage)
          {
            TransitionToCommittedForNewPage();
          }
        }
        break;

      case FrameLoadType::Reload:
      case FrameLoadType::ReloadFromOrigin:
      case FrameLoadType::ReloadExpiredOnly:
      case FrameLoadType::Same:
      case FrameLoadType::MultipartReplace:
      case FrameLoadType::NavigationAPIReplace:
        _history->UpdateForReloadOrReplace();
        TransitionToCommittedForNewPage();
        break;

      case FrameLoadType::Standard:
        _history->UpdateForStandardLoad();
        TransitionToCommittedForNewPage();
        break;

      case FrameLoadType::RedirectWithLockedBackForwardList:
        _history->UpdateForRedirectWithLockedBackForwardList();
        TransitionToCommittedForNewPage();
        break;
    }
    _committedFirstRealDocumentLoad = true;

    if (_loadType == FrameLoadType::Standard && loader->IsClientRedirect())
    {
      _history->UpdateForClientRedirect();
    }

    if (cachedPage && cachedPage->Document())
    {
      _client->DispatchDidCommitLoad();

      if (RefPtr<Page> page = ShareRefPtr(_frame.GetPage()))
      {
        cachedPage->Restore(*page);
      }

      // Dispatch any pending navigate event after the restore is complete.
      if (RefPtr<HistoryItem> item = std::exchange(_pendingNavigationAPIItem, nullptr))
      {
        if (auto *window = _frame.Document() ? _frame.Document()->Window() : nullptr)
        {
          (void)window->GetNavigation().DispatchTraversalNavigateEvent(*item);
        }
      }

      // Force a layout to update view size and thereby update scrollbars.
      if (RefPtr<dom::Document> document = ShareRefPtr(_frame.Document()))
      {
        (void)document->UpdateLayout();
      }

      _loadingFromCachedPage = false;
      CheckCompleted();
    }
    _loadingFromCachedPage = false;
  }

  void FrameLoader::Open(CachedFrame &cachedFrame) noexcept
  {
    RefPtr<dom::Document> document = ShareRefPtr(cachedFrame.Document());
    krys_debug_assert(document && document->Window());

    document->SetBackForwardCacheState(dom::BackForwardCacheState::NotInBackForwardCache);

    // The document loaded before it was cached: its load event doesn't fire again.
    _isComplete = false;
    _didCallImplicitClose = true;

    RefPtr<LocalFrameView> view = ShareRefPtr(cachedFrame.View());
    krys_debug_assert(view);
    view->SetLastUserScrollType(null);

    // Use the previous view's frame rect.
    Maybe<IntRect> previousViewFrameRect;
    if (auto *previousView = _frame.View())
    {
      previousViewFrameRect = previousView->FrameRect();
    }
    _frame.SetView(RefPtr<LocalFrameView>(view));
    if (previousViewFrameRect)
    {
      view->SetFrameRect(*previousViewFrameRect);
    }

    // Setting the document builds its render tree.
    _frame.SetDocument(RefPtr<dom::Document>(document));

    // FIXME: The window's resumption for its observers, once there are any.

    cachedFrame.Restore();
  }

  void FrameLoader::UpdateURLAndHistory(const url::URL &newURL,
                                        RefPtr<html::SerializedScriptValue> &&stateObject,
                                        NavigationHistoryBehavior historyHandling) noexcept
  {
    krys_debug_assert(_frame.Document() && _documentLoader);

    if (_isDisplayingInitialEmptyDocument)
    {
      historyHandling = NavigationHistoryBehavior::Replace;
    }

    // FIXME: Restoring the history object state (step 2 of the steps), once state is deserialized.

    if (auto *document = _frame.Document())
    {
      document->UpdateURLForPushOrReplaceState(newURL);
    }

    if (historyHandling == NavigationHistoryBehavior::Replace)
    {
      _history->ReplaceState(krys::move(stateObject), newURL);
      _client->DispatchDidReplaceStateWithinPage();
    }
    else
    {
      _history->PushState(krys::move(stateObject), newURL);
      _client->DispatchDidPushStateWithinPage();
    }
  }

  void FrameLoader::StopLoading(UnloadEventPolicy unloadEventPolicy) noexcept
  {
    // FIXME: Stopping the parser midway, once a document can stop parsing before its data is all there.
    if (unloadEventPolicy != UnloadEventPolicy::None)
    {
      DispatchUnloadEvents(unloadEventPolicy);
    }

    _isComplete = true;           // to avoid calling completed() in finishedParsing()
    _didCallImplicitClose = true; // don't want that one either

    // FIXME: This will cancel redirection timer, which really needs to be restarted when restoring the frame
    // from b/f cache.
    _frame.GetNavigationScheduler().Cancel();
  }

  void FrameLoader::StopForBackForwardCache() noexcept
  {
    // Stop current loads.
    if (RefPtr<DocumentLoader> documentLoader = _documentLoader)
    {
      documentLoader->StopLoading();
    }

    // We cancel pending navigations after cancelling loads because cancelling loads might end up running
    // script, which could schedule new navigations.
    _frame.GetNavigationScheduler().Cancel();
  }

  namespace
  {
    /// @brief Counts an unload event's dispatch in the document's unload counter, for as long as it lasts.
    class UnloadCountIncrementer
    {
      KRYS_NON_COPY_MOVABLE(UnloadCountIncrementer);

    private:
      RefPtr<dom::Document> _document;

    public:
      explicit UnloadCountIncrementer(dom::Document *document) noexcept : _document(ShareRefPtr(document))
      {
        if (_document)
        {
          _document->IncrementUnloadCounter();
        }
      }

      ~UnloadCountIncrementer() noexcept
      {
        if (_document)
        {
          _document->DecrementUnloadCounter();
        }
      }
    };
  }

  bool FrameLoader::ShouldClose() noexcept
  {
    Ref<LocalFrame> frame = ShareRef(_frame);
    if (!frame->GetPage() || _pageDismissalEventBeingDispatched)
    {
      return true;
    }

    UnloadCountIncrementer unloadCountIncrementer(frame->Document());
    return DispatchBeforeUnloadEvent();
  }

  bool FrameLoader::DispatchBeforeUnloadEvent() noexcept
  {
    RefPtr<dom::Document> document = ShareRefPtr(_frame.Document());
    RefPtr<LocalDOMWindow> window = document ? ShareRefPtr(document->Window()) : nullptr;
    if (!window || !document->BodyOrFrameset())
    {
      return true;
    }

    auto beforeUnloadEvent = html::BeforeUnloadEvent::Create();
    {
      SetForScope dismissal(_pageDismissalEventBeingDispatched, true);
      (void)dom::EventDispatcher::DispatchToTarget(*beforeUnloadEvent, window.get(), true);
    }

    // Web pages can request we ask for confirmation before navigating by cancelling the event, or setting its
    // returnValue to a non-empty string; only if the user has interacted with the page (it has sticky
    // activation).
    bool asksToStay = beforeUnloadEvent->DefaultPrevented() || !beforeUnloadEvent->ReturnValue().empty();
    if (!asksToStay || !window->HasStickyActivation())
    {
      return true;
    }

    auto *page = _frame.GetPage();
    return !page
           || page->GetChromeClient().RunBeforeUnloadConfirmPanel(beforeUnloadEvent->ReturnValue(), _frame);
  }

  void FrameLoader::DispatchUnloadEvents(UnloadEventPolicy unloadEventPolicy) noexcept
  {
    RefPtr<dom::Document> document = ShareRefPtr(_frame.Document());
    if (!document || _pageDismissalEventBeingDispatched)
    {
      return;
    }

    UnloadCountIncrementer unloadCountIncrementer(document.get());

    if (_didCallImplicitClose && !_wasUnloadEventEmitted)
    {
      SetForScope dismissal(_pageDismissalEventBeingDispatched, true);
      if (unloadEventPolicy == UnloadEventPolicy::UnloadAndPageHide)
      {
        document->DispatchPagehideEvent(document->GetBackForwardCacheState()
                                        == dom::BackForwardCacheState::AboutToEnterBackForwardCache);
      }

      // FIXME: The visibility (visibilitychange), once there's one.

      if (document->GetBackForwardCacheState() == dom::BackForwardCacheState::NotInBackForwardCache)
      {
        if (RefPtr<LocalDOMWindow> window = ShareRefPtr(document->Window()))
        {
          auto unloadEvent = dom::Event::Create(dom::DOMStringAtom(EventNames::Unload), false, false);
          (void)dom::EventDispatcher::DispatchToTarget(*unloadEvent, window.get(), true);
        }
      }
      _wasUnloadEventEmitted = true;
    }
  }

  void FrameLoader::DidBeginDocument(LocalDOMWindow *previousWindow) noexcept
  {
    _client->DispatchDidClearWindowObject();
    if (_state == FrameState::CommittedPage)
    {
      _client->DispatchDidCommitLoad();
    }

    if (auto *window = _frame.Document() ? _frame.Document()->Window() : nullptr)
    {
      Maybe<NavigationNavigationType> navigationType;
      if (_documentLoader)
      {
        navigationType = _documentLoader->TriggeringAction().NavigationAPIType();
      }
      window->GetNavigation().InitializeForNewWindow(navigationType, previousWindow);
    }

    _history->RestoreDocumentState();
  }

  void FrameLoader::StopForUserCancel() noexcept
  {
    Ref<LocalFrame> frame = ShareRef(_frame);
    StopAllLoaders();

    if (auto *window = _frame.Document() ? _frame.Document()->Window() : nullptr)
    {
      window->GetNavigation().AbortOngoingNavigationIfNeeded();
    }
  }

  NavigationNavigationType
    FrameLoader::DetermineNavigationType(FrameLoadType loadType,
                                         NavigationHistoryBehavior historyHandling) noexcept
  {
    if (historyHandling == NavigationHistoryBehavior::Push)
    {
      return NavigationNavigationType::Push;
    }
    if (historyHandling == NavigationHistoryBehavior::Replace)
    {
      return NavigationNavigationType::Replace;
    }
    if (historyHandling == NavigationHistoryBehavior::Reload)
    {
      return NavigationNavigationType::Reload;
    }

    if (IsBackForwardLoadType(loadType))
    {
      return NavigationNavigationType::Traverse;
    }
    if (IsReload(loadType))
    {
      return NavigationNavigationType::Reload;
    }
    if (loadType == FrameLoadType::MultipartReplace || loadType == FrameLoadType::NavigationAPIReplace)
    {
      return NavigationNavigationType::Replace;
    }
    return NavigationNavigationType::Push;
  }

  bool FrameLoader::DispatchNavigateEvent(FrameLoadType loadType, NavigationHistoryBehavior historyHandling,
                                          const url::URL &newURL, dom::DOMStringAtom downloadAttribute,
                                          bool isSameDocument, const FormSubmission *formSubmission,
                                          dom::Event *event,
                                          html::SerializedScriptValue *classicHistoryAPIState) noexcept
  {
    RefPtr<dom::Document> document = ShareRefPtr(_frame.Document());
    RefPtr<LocalDOMWindow> window = document ? ShareRefPtr(document->Window()) : nullptr;
    if (!window)
    {
      return true;
    }

    // Download events are handled later, in the navigation policy.
    if (downloadAttribute != dom::DOMStringAtom::Null())
    {
      return true;
    }

    // Only URLs with a fetch scheme navigate to another document with a navigate event.
    bool hasFetchScheme = newURL.ProtocolIsInHTTPFamily() || newURL.ProtocolIsAbout()
                          || newURL.ProtocolIsData() || newURL.ProtocolIsFile()
                          || newURL.ProtocolIs(u8"blob");
    if (!isSameDocument && !hasFetchScheme)
    {
      return true;
    }

    auto navigationType = DetermineNavigationType(loadType, historyHandling);

    // Traversals are handled earlier, in LoadItem().
    if (navigationType == NavigationNavigationType::Traverse)
    {
      return true;
    }

    auto *targetNode = event ? DynamicDowncast<dom::Node>(event->Target().get()) : nullptr;
    auto *sourceElement = targetNode ? DynamicDowncast<dom::Element>(*targetNode) : nullptr;
    return window->GetNavigation().DispatchPushReplaceReloadNavigateEvent(
      newURL, navigationType, isSameDocument, formSubmission, classicHistoryAPIState, sourceElement);
  }

  void FrameLoader::FinishedLoading(DocumentLoader &loader) noexcept
  {
    if (&loader != _documentLoader.get())
    {
      return;
    }
    CheckCompleted();
  }

  void FrameLoader::LoadDone() noexcept
  {
    CheckCompleted();
  }

  void FrameLoader::CheckCompleted() noexcept
  {
    // Have we completed before?
    if (_isComplete)
    {
      return;
    }

    RefPtr<dom::Document> document = ShareRefPtr(_frame.Document());
    if (!document)
    {
      return;
    }

    // Resource loads are kicked off by style resolution, so it's brought up to date first.
    document->UpdateStyleIfNeeded();

    // Are we still parsing, or waiting for the main resource?
    if (document->Parsing() || (_documentLoader && _documentLoader->IsLoadingMainResource()))
    {
      return;
    }

    // Still waiting for images, style sheets or fonts?
    if (document->GetCachedResourceLoader().RequestCount() || !document->HaveStylesheetsLoaded())
    {
      return;
    }

    // Still waiting for elements that don't go through a frame loader?
    if (document->IsDelayingLoadEvent())
    {
      return;
    }

    // OK, completed.
    _isComplete = true;
    _requestedHistoryItem = nullptr;
    dom::DocumentAlgorithms::UpdateCurrentDocumentReadiness(*document, html::DocumentReadyState::Complete);

    CheckCallImplicitClose();
    // A redirect waits for the document to have loaded.
    _frame.GetNavigationScheduler().StartTimer();
    CheckLoadComplete();
    _quickRedirectComing = false;
  }

  void FrameLoader::CheckCallImplicitClose() noexcept
  {
    RefPtr<dom::Document> document = ShareRefPtr(_frame.Document());
    if (_didCallImplicitClose || !document || document->Parsing() || document->IsDelayingLoadEvent())
    {
      return;
    }
    _didCallImplicitClose = true;
    document->ImplicitClose();
  }

  void FrameLoader::CheckLoadComplete() noexcept
  {
    if (_state == FrameState::Complete)
    {
      _loadType = FrameLoadType::Standard;
      FrameLoadCompleted();
      return;
    }

    if (_state != FrameState::CommittedPage || !_isComplete)
    {
      return;
    }
    SetState(FrameState::Complete);

    // If the user had a scroll point, scroll to it, overriding the anchor point if any.
    if (_frame.GetPage() && (IsBackForwardLoadType(_loadType) || IsReload(_loadType)))
    {
      _history->RestoreScrollPositionAndViewState();
    }

    _client->DispatchDidFinishLoad();
  }

  void FrameLoader::ReceivedMainResourceError(DocumentLoader &loader, const ResourceError &error) noexcept
  {
    if (&loader == _provisionalDocumentLoader.get())
    {
      // Reset the back forward list to the last committed history item at the top level, but only if we
      // aren't already going to a new provisional item.
      RefPtr<HistoryItem> item =
        IsBackForwardLoadType(_loadType) ? ShareRefPtr(_history->CurrentItem()) : nullptr;
      bool shouldReset = !_history->ProvisionalItem();

      // The frame keeps its document.
      _provisionalDocumentLoader->DetachFromFrame();
      _provisionalDocumentLoader = nullptr;
      SetState(FrameState::Complete);
      _client->DispatchDidFailProvisionalLoad(error);

      if (shouldReset && item)
      {
        if (auto *page = _frame.GetPage())
        {
          page->BackForward().SetCurrentItem(*item);
        }
      }
      return;
    }

    if (&loader == _documentLoader.get())
    {
      SetState(FrameState::Complete);
      _client->DispatchDidFailLoad(error);
    }
  }

  void FrameLoader::ChangeLocation(const url::URL &url, dom::DOMStringAtom passedTarget,
                                   dom::Event *triggeringEvent, ReferrerPolicy referrerPolicy,
                                   Maybe<NewFrameOpenerPolicy> openerPolicy,
                                   dom::DOMStringAtom downloadAttribute,
                                   NavigationHistoryBehavior historyBehavior,
                                   dom::Element *sourceElement) noexcept
  {
    auto *document = _frame.Document();
    if (!document)
    {
      return;
    }

    FrameLoadRequest frameLoadRequest(ShareRef(*document), ResourceRequest(url), passedTarget,
                                      downloadAttribute);
    frameLoadRequest.SetNewFrameOpenerPolicy(
      openerPolicy.value_or(referrerPolicy == ReferrerPolicy::NoReferrer ? NewFrameOpenerPolicy::Suppress
                                                                         : NewFrameOpenerPolicy::Allow));
    frameLoadRequest.SetReferrerPolicy(referrerPolicy);
    frameLoadRequest.SetNavigationHistoryBehavior(historyBehavior);
    frameLoadRequest.SetSourceElement(sourceElement);
    ChangeLocation(krys::move(frameLoadRequest), triggeringEvent);
  }

  void FrameLoader::ChangeLocation(FrameLoadRequest &&frameRequest, dom::Event *triggeringEvent) noexcept
  {
    krys_assert(frameRequest.GetResourceRequest().HTTPMethod() == u8"GET");

    if (frameRequest.FrameName().View().empty())
    {
      if (auto *document = _frame.Document())
      {
        frameRequest.SetFrameName(dom::DOMStringAtom(document->BaseTarget()));
      }
    }

    LoadFrameRequest(krys::move(frameRequest), triggeringEvent, nullptr);
  }

  void FrameLoader::SubmitForm(Ref<FormSubmission> &&submission) noexcept
  {
    krys_assert(submission->GetMethod() == FormSubmission::Method::Post
                || submission->GetMethod() == FormSubmission::Method::Get);

    if (!_frame.GetPage())
    {
      return;
    }

    if (submission->Action().IsEmpty())
    {
      return;
    }

    auto *form = submission->Form();
    auto &requestingDocument = form ? form->NodeDocument() : *_frame.Document();

    // A target naming another browsing context is kept, for the client's new window policy; one naming this
    // frame is cleared.
    if (FindFrameForNavigation(submission->Target()))
    {
      submission->ClearTarget();
    }

    if (submission->WasCancelled())
    {
      return;
    }

    (void)requestingDocument;
    _frame.GetNavigationScheduler().ScheduleFormSubmission(krys::move(submission));
  }

  void FrameLoader::LoadFrameRequest(FrameLoadRequest &&request, dom::Event *event,
                                     RefPtr<FormSubmission> &&formSubmission) noexcept
  {
    krys_assert(_frame.Document());

    // A reload (location.reload()) loads the document again, even to a fragment of its URL; a navigation that
    // replaces the current history item locks the back/forward list.
    FrameLoadType loadType;
    if (request.GetNavigationHistoryBehavior() == NavigationHistoryBehavior::Reload)
    {
      loadType = FrameLoadType::Reload;
    }
    else if (request.GetNavigationHistoryBehavior() == NavigationHistoryBehavior::Replace)
    {
      loadType = request.IsFromNavigationAPI() ? FrameLoadType::NavigationAPIReplace
                                               : FrameLoadType::RedirectWithLockedBackForwardList;
    }
    else
    {
      loadType = FrameLoadType::Standard;
    }

    if (loadType == FrameLoadType::Reload)
    {
      RefPtr<html::SerializedScriptValue> stateObject;
      if (auto *currentItem = _history->CurrentItem())
      {
        stateObject = ShareRefPtr(currentItem->NavigationAPIStateObject());
      }
      if (!DispatchNavigateEvent(loadType, request.GetNavigationHistoryBehavior(),
                                 request.GetResourceRequest().URL(), request.DownloadAttribute(), false,
                                 formSubmission.get(), event, stateObject.get()))
      {
        return;
      }
      if (!_frame.GetPage())
      {
        return;
      }
    }

    if (request.GetResourceRequest().HTTPMethod() == u8"POST")
    {
      LoadPostRequest(krys::move(request), loadType, event, krys::move(formSubmission));
    }
    else
    {
      LoadURL(krys::move(request), loadType, event, krys::move(formSubmission));
    }
  }

  void FrameLoader::LoadURL(FrameLoadRequest &&frameLoadRequest, FrameLoadType newLoadType, dom::Event *event,
                            RefPtr<FormSubmission> &&formSubmission) noexcept
  {
    krys_assert(frameLoadRequest.GetResourceRequest().HTTPMethod() == u8"GET");

    // The target is ignored when the download attribute is set, as the link is downloaded rather than
    // followed.
    auto effectiveFrameName = frameLoadRequest.DownloadAttribute() == dom::DOMStringAtom::Null()
                                ? frameLoadRequest.FrameName()
                                : dom::DOMStringAtom::Null();
    bool isFormSubmission = formSubmission != nullptr;

    auto *targetFrame = isFormSubmission ? nullptr : FindFrameForNavigation(effectiveFrameName.View());
    if (targetFrame && targetFrame != &_frame)
    {
      frameLoadRequest.SetFrameName(dom::DOMStringAtom(u8"_self"));
      targetFrame->Loader().LoadURL(krys::move(frameLoadRequest), newLoadType, event,
                                    krys::move(formSubmission));
      return;
    }

    const auto &newURL = frameLoadRequest.GetResourceRequest().URL();
    ResourceRequest request(newURL);

    NavigationAction action(request, newLoadType, isFormSubmission, event,
                            frameLoadRequest.DownloadAttribute(), frameLoadRequest.SourceElement());
    action.SetIsRequestFromClientOrUserInput(frameLoadRequest.IsRequestFromClientOrUserInput());
    action.SetNewFrameOpenerPolicy(frameLoadRequest.GetNewFrameOpenerPolicy());
    action.SetWindowFeatures(Maybe<WindowFeatures>(frameLoadRequest.GetWindowFeatures()));

    auto historyHandling = frameLoadRequest.GetNavigationHistoryBehavior();
    auto &document = *_frame.Document();
    if (!IsReload(newLoadType) && historyHandling == NavigationHistoryBehavior::Auto)
    {
      historyHandling =
        document.GetURL() == newURL ? NavigationHistoryBehavior::Replace : NavigationHistoryBehavior::Push;
    }
    frameLoadRequest.SetNavigationHistoryBehavior(historyHandling);
    action.SetNavigationAPIType(DetermineNavigationType(newLoadType, historyHandling));

    if (!targetFrame && !effectiveFrameName.View().empty())
    {
      CheckNewWindowPolicy(action, request, formSubmission.get(), effectiveFrameName.View());
      return;
    }

    // Scroll to the fragment even if the URL is exactly the same, so pages with `#` links work.
    if (ShouldPerformFragmentNavigation(isFormSubmission, request.HTTPMethod(), newLoadType, newURL))
    {
      if (!DispatchNavigateEvent(newLoadType, historyHandling, newURL, frameLoadRequest.DownloadAttribute(),
                                 true, formSubmission.get(), event))
      {
        _quickRedirectComing = false;
        return;
      }
      auto policy = _client->DispatchDecidePolicyForNavigationAction(action, request, formSubmission.get());
      ContinueFragmentScrollAfterNavigationPolicy(request, policy == PolicyAction::Use, historyHandling);
      return;
    }

    bool sameURL = ShouldTreatURLAsSameAsCurrent(newURL);
    auto policy = _client->DispatchDecidePolicyForNavigationAction(action, request, formSubmission.get());
    if (policy != PolicyAction::Use)
    {
      // FIXME: Downloads, once there are any.
      _quickRedirectComing = false;
      return;
    }

    // The navigate event fires once the page that's left allows the load (beforeunload).
    // FIXME: Not for an initial frame src load, once there are frames other than the main frame.
    if (newLoadType != FrameLoadType::Reload)
    {
      _pendingDispatchNavigateEvent = [this, newLoadType, historyHandling, newURL,
                                       downloadAttribute = frameLoadRequest.DownloadAttribute(),
                                       formSubmission, event = RefPtr<dom::Event>(ShareRefPtr(event))]
      {
        return DispatchNavigateEvent(newLoadType, historyHandling, newURL, downloadAttribute, false,
                                     formSubmission.get(), event.get());
      };
    }

    // Must grab this now, since this load may stop the previous load and clear this flag.
    bool isRedirect = _quickRedirectComing;
    if (!LoadWithNavigationAction(request, krys::move(action), newLoadType, null))
    {
      return;
    }
    if (isRedirect)
    {
      _quickRedirectComing = false;
      if (_provisionalDocumentLoader)
      {
        _provisionalDocumentLoader->SetIsClientRedirect(true);
      }
    }
    else if (sameURL && !IsReload(newLoadType))
    {
      // Example of this case are sites that reload the same URL with a different cookie driving the generated
      // content, or a master frame with links that drive a target frame, where the user has clicked on the
      // same link repeatedly.
      _loadType = FrameLoadType::Same;
    }
  }

  void FrameLoader::LoadPostRequest(FrameLoadRequest &&request, FrameLoadType loadType, dom::Event *event,
                                    RefPtr<FormSubmission> &&formSubmission) noexcept
  {
    auto frameName = request.FrameName();

    ResourceRequest workingResourceRequest(request.GetResourceRequest().URL());
    workingResourceRequest.SetHTTPMethod(u8"POST");

    auto *targetFrame =
      formSubmission || frameName.View().empty() ? nullptr : FindFrameForNavigation(frameName.View());

    NavigationAction action(workingResourceRequest, loadType, true, event);
    action.SetIsRequestFromClientOrUserInput(request.IsRequestFromClientOrUserInput());
    action.SetNewFrameOpenerPolicy(request.GetNewFrameOpenerPolicy());
    action.SetSourceElement(request.SourceElement());

    if (!frameName.View().empty() && !targetFrame)
    {
      CheckNewWindowPolicy(action, workingResourceRequest, formSubmission.get(), frameName.View());
      return;
    }

    // FIXME: Only for a request of the document's origin, once there are origins.
    action.SetNavigationAPIType(DetermineNavigationType(loadType, request.GetNavigationHistoryBehavior()));
    if (!DispatchNavigateEvent(loadType, request.GetNavigationHistoryBehavior(), workingResourceRequest.URL(),
                               dom::DOMStringAtom::Null(), false, formSubmission.get(), event))
    {
      return;
    }

    auto policy =
      _client->DispatchDecidePolicyForNavigationAction(action, workingResourceRequest, formSubmission.get());
    if (policy != PolicyAction::Use)
    {
      return;
    }

    // FIXME: The body of the request, once requests carry them; the host's loader strategy gets the URL.
    (void)LoadWithNavigationAction(workingResourceRequest, krys::move(action), loadType, null);
  }

  void FrameLoader::CheckNewWindowPolicy(const NavigationAction &action, const ResourceRequest &request,
                                         const FormSubmission *formSubmission,
                                         dom::DOMStringView frameName) noexcept
  {
    auto policy = _client->DispatchDecidePolicyForNewWindowAction(action, request, formSubmission, frameName);
    // FIXME: Opening the new browsing context (continueLoadAfterNewWindowPolicy): there are no windows, so
    // the host opens one itself and answers `Ignore`.
    (void)policy;
  }

  LocalFrame *FrameLoader::FindFrameForNavigation(dom::DOMStringView name) const noexcept
  {
    // https://html.spec.whatwg.org/#the-rules-for-choosing-a-navigable
    if (name.empty() || text::IsASCIICaselessEqual(name, utf8_stringview(u8"_self"))
        || text::IsASCIICaselessEqual(name, utf8_stringview(u8"_parent"))
        || text::IsASCIICaselessEqual(name, utf8_stringview(u8"_top")))
    {
      return &_frame;
    }
    // A navigable whose target name is name: the frame itself, as there's no frame tree.
    if (!text::IsASCIICaselessEqual(name, utf8_stringview(u8"_blank")) && name == _frame.SpecifiedName())
    {
      return &_frame;
    }
    // `_blank`, and names no frame has.
    return nullptr;
  }

  bool FrameLoader::ShouldReload(const url::URL &currentURL, const url::URL &destinationURL) noexcept
  {
    // This function implements the rule: "Don't reload if navigating by fragment within the same URL, but do
    // reload if going to a new URL or to the same URL with no fragment identifier at all."
    if (!destinationURL.HasFragmentIdentifier())
    {
      return true;
    }
    return !url::EqualIgnoringFragmentIdentifier(currentURL, destinationURL);
  }

  bool FrameLoader::ShouldPerformFragmentNavigation(bool isFormSubmission, utf8_stringview httpMethod,
                                                    FrameLoadType loadType,
                                                    const url::URL &url) const noexcept
  {
    // We don't do this if we are submitting a form with method other than "GET", explicitly reloading, or if
    // the URL does not have a fragment.
    // FIXME: The back and forward cache, framesets and the initial empty document, once there are any.
    return (!isFormSubmission || text::IsASCIICaselessEqual(httpMethod, utf8_stringview(u8"get")))
           && !IsReload(loadType) && loadType != FrameLoadType::Same
           && !ShouldReload(_frame.Document()->GetURL(), url);
  }

  void FrameLoader::ContinueFragmentScrollAfterNavigationPolicy(
    const ResourceRequest &request, bool shouldContinue, NavigationHistoryBehavior historyHandling) noexcept
  {
    if (!shouldContinue)
    {
      return;
    }

    // FIXME: Canceling a provisional load of another document, once there are any.
    bool isRedirect = _quickRedirectComing || _loadType == FrameLoadType::RedirectWithLockedBackForwardList;
    LoadInSameDocument(request.URL(), nullptr, !isRedirect, historyHandling);
    _quickRedirectComing = false;
  }

  // This does the same kind of work that didOpenURL does, except it relies on the fact that a higher level
  // already checked that the URLs match and the scrolling is the right thing to do.
  void FrameLoader::LoadInSameDocument(const url::URL &url, html::SerializedScriptValue *stateObject,
                                       bool isNewNavigation,
                                       NavigationHistoryBehavior historyHandling) noexcept
  {
    // If we have a state object, we cannot also be a new navigation.
    krys_debug_assert(!stateObject || !isNewNavigation);

    Ref<LocalFrame> protectedFrame = ShareRef(_frame);
    RefPtr<dom::Document> document = ShareRefPtr(_frame.Document());
    if (!document)
    {
      return;
    }

    auto oldURL = document->GetURL();
    document->SetURL(url::URL(url));
    if (isNewNavigation && !ShouldTreatURLAsSameAsCurrent(url) && !stateObject)
    {
      // NB: Must happen before we openURL and displace the scroll position, since adding the BF item will
      // save away scroll state.
      if (historyHandling == NavigationHistoryBehavior::Replace)
      {
        _history->UpdateBackForwardListForReplaceState(nullptr, url);
      }
      else
      {
        _history->UpdateBackForwardListForFragmentScroll();
      }

      if (!document->Window() || !document->Window()->HasTransientActivation())
      {
        if (!_documentLoader || !_documentLoader->TriggeringAction().IsRequestFromClientOrUserInput())
        {
          if (auto *currentItem = _history->CurrentItem())
          {
            currentItem->SetWasCreatedByJSWithoutUserInteraction(true);
          }
        }
      }
    }

    _history->UpdateForSameDocumentNavigation();

    auto navigationType = DetermineNavigationType(_loadType, historyHandling);
    if (auto *window = document->Window(); window && _history->CurrentItem())
    {
      auto &navigation = window->GetNavigation();
      // Skip UpdateForNavigation() for intercepted traverse navigations, as it was already called after
      // intercept detection in InnerDispatchNavigateEvent().
      if (navigationType != NavigationNavigationType::Traverse
          || !navigation.HasInterceptedOngoingNavigateEvent())
      {
        navigation.UpdateForNavigation(ShareRef(*_history->CurrentItem()), navigationType,
                                       ShouldCopyStateObjectFromCurrentEntry::Yes);
      }
    }

    bool hashChange = url::EqualIgnoringFragmentIdentifier(url, oldURL)
                      && (url.FragmentIdentifier() != oldURL.FragmentIdentifier()
                          || url.HasFragmentIdentifier() != oldURL.HasFragmentIdentifier());

    // If we were in the autoscroll mode we want to stop it before following the link to the anchor.
    if (hashChange)
    {
      _frame.GetEventHandler().StopAutoscrollTimer();
    }

    // We need to scroll to the fragment whether or not a hash change occurred, since the user might have
    // scrolled since the previous navigation.
    ScrollToFragmentWithParentBoundary(url, isNewNavigation);

    // It's important to model this as a load that starts and immediately finishes.
    _isComplete = false;
    CheckCompleted();

    if (isNewNavigation)
    {
      // This will clear previousItem from the rest of the frame tree that didn't doing any loading. We need
      // to make a pass on this now, since for fragment navigation we'll not go through a real load and reach
      // Completed state.
      CheckLoadComplete();
    }

    _client->DispatchDidNavigateWithinPage();

    document->StatePopped(ShareRefPtr(stateObject));
    _client->DispatchDidPopStateWithinPage();

    if (hashChange)
    {
      document->EnqueueHashchangeEvent(dom::USVString(oldURL.String()), dom::USVString(url.String()));
      _client->DispatchDidChangeLocationWithinPage();
    }
  }

  void FrameLoader::FinishedParsing() noexcept
  {
    // FIXME: Unsuppressing the scrollbars, once they're suppressed while a document loads.
    Ref<LocalFrame> frame = ShareRef(_frame);
    if (_state == FrameState::CommittedPage)
    {
      _client->DispatchDidFinishDocumentLoad();
    }

    CheckCompleted();

    if (auto *document = frame->Document())
    {
      ScrollToFragmentWithParentBoundary(document->GetURL());
    }
  }

  void FrameLoader::ScrollToFragmentWithParentBoundary(const url::URL &url, bool isNewNavigation) noexcept
  {
    RefPtr<LocalFrameView> view = ShareRefPtr(_frame.View());
    RefPtr<dom::Document> document = ShareRefPtr(_frame.Document());
    if (!view || !document)
    {
      return;
    }

    auto itemAllowsScrollRestoration = [](HistoryItem *historyItem, FrameLoadType loadType)
    {
      if (!historyItem)
      {
        return true;
      }
      if (IsBackForwardLoadType(loadType))
      {
        return historyItem->ShouldRestoreScrollPosition();
      }
      return true;
    };
    bool isSameDocumentReload = !isNewNavigation && !IsBackForwardLoadType(_loadType);

    if (isSameDocumentReload || itemAllowsScrollRestoration(_history->CurrentItem(), _loadType))
    {
      // https://html.spec.whatwg.org/multipage/browsing-the-web.html#try-to-scroll-to-the-fragment
      if (!document->HaveStylesheetsLoaded())
      {
        document->SetGotoAnchorNeededAfterStylesheetsLoad(true);
      }
      else
      {
        (void)view->ScrollToFragment(url);
      }
    }
  }
}
