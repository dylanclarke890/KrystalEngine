#include "Krystal.Booey/Loader/HistoryController.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/History/BackForwardCache.hpp"
#include "Krystal.Booey/History/BackForwardController.hpp"
#include "Krystal.Booey/HTML/SerializedScriptValue.hpp"
#include "Krystal.Booey/Loader/DocumentLoader.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Navigation.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <utility>

namespace krys::boo
{
  HistoryController::HistoryController(LocalFrame &frame) noexcept : _frame(frame)
  {
  }

  HistoryController::~HistoryController() noexcept = default;

  void HistoryController::SaveScrollPositionAndViewStateToItem(HistoryItem *item) noexcept
  {
    RefPtr<LocalFrameView> frameView = ShareRefPtr(_frame.View());
    if (!item || !frameView)
    {
      return;
    }

    auto *document = _frame.Document();
    if (document && document->GetBackForwardCacheState() != dom::BackForwardCacheState::NotInBackForwardCache)
    {
      item->SetScrollPosition(frameView->CachedScrollPosition());
    }
    else
    {
      item->SetScrollPosition(frameView->GetScrollPosition());
    }

    // FIXME: Divided by the page's view scale factor, once it has one; only for the main frame, once there
    // are subframes.
    if (auto *page = _frame.GetPage())
    {
      item->SetPageScaleFactor(page->PageScaleFactor());
    }
  }

  /*
   There is a race condition between the layout and load completion that affects restoring the scroll
   position. We try to restore the scroll position at both the first layout and upon load completion.

   1) If first layout happens before the load completes, we want to restore the scroll position then so that
   the first time we draw the page is already scrolled to the right place, instead of starting at the top and
   later jumping down.  It is possible that the old scroll position is past the part of the doc laid out so
   far, in which case the restore silent fails and we will fix it in when we try to restore on doc completion.
   2) If the layout happens after the load completes, the attempt to restore at load completion time silently
   fails.  We then successfully restore it when the layout happens.
  */
  void HistoryController::RestoreScrollPositionAndViewState() noexcept
  {
    RefPtr<HistoryItem> currentItem = _currentItem;
    if (!currentItem || !_frame.Loader().CommittedFirstRealDocumentLoad())
    {
      return;
    }

    RefPtr<LocalFrameView> view = ShareRefPtr(_frame.View());
    if (view && !view->WasScrolledByUser())
    {
      // FIXME: Scrolling to the focused element first (scrollToFocusedElementImmediatelyIfNeeded).
      auto desiredScrollPosition = currentItem->ShouldRestoreScrollPosition() ? currentItem->ScrollPosition()
                                                                              : view->GetScrollPosition();
      if (auto *page = _frame.GetPage(); page && currentItem->PageScaleFactor())
      {
        page->SetPageScaleFactor(currentItem->PageScaleFactor(), desiredScrollPosition);
      }
      else
      {
        view->SetScrollPosition(desiredScrollPosition);
      }
    }
  }

  void HistoryController::UpdateBackForwardListForFragmentScroll() noexcept
  {
    UpdateBackForwardListClippedAtTarget(false);
  }

  void HistoryController::SaveDocumentState() noexcept
  {
    // For a standard page load, we will have a previous item set, which will be used to store the form state.
    // However, in some cases we will have no previous item, and the current item is the right place to save
    // the state.

    // Because of previousItem's "masking" of currentItem for this purpose, it's important that we keep track
    // of the end of a page transition with _frameLoadComplete. We leverage the checkLoadComplete recursion to
    // achieve this goal.
    RefPtr<HistoryItem> item = _frameLoadComplete ? _currentItem : _previousItem;
    if (!item)
    {
      return;
    }

    RefPtr<dom::Document> document = ShareRefPtr(_frame.Document());
    if (document && item->IsCurrentDocument(*document) && document->HasLivingRenderTree())
    {
      item->SetDocumentState(document->FormElementsState());
    }
  }

  // Tells the frame to save its form state and scroll position into its current history item.
  void HistoryController::SaveDocumentAndScrollState() noexcept
  {
    SaveDocumentState();
    SaveScrollPositionAndViewStateToItem(_currentItem.get());
  }

  void HistoryController::RestoreDocumentState() noexcept
  {
    switch (_frame.Loader().LoadType())
    {
      case FrameLoadType::Reload:
      case FrameLoadType::ReloadFromOrigin:
      case FrameLoadType::ReloadExpiredOnly:
      case FrameLoadType::Same:
      case FrameLoadType::MultipartReplace:
        // Not restoring the document state.
        return;
      case FrameLoadType::Back:
      case FrameLoadType::Forward:
      case FrameLoadType::IndexedBackForward:
      case FrameLoadType::RedirectWithLockedBackForwardList:
      case FrameLoadType::Standard:
      case FrameLoadType::NavigationAPIReplace:              break;
    }

    RefPtr<HistoryItem> currentItem = _currentItem;
    if (!currentItem)
    {
      return;
    }
    auto *requestedHistoryItem = _frame.Loader().RequestedHistoryItem();
    if (!requestedHistoryItem || requestedHistoryItem->ItemID() != currentItem->ItemID())
    {
      return;
    }
    auto *documentLoader = _frame.Loader().GetDocumentLoader();
    if (!documentLoader || documentLoader->IsClientRedirect())
    {
      return;
    }

    if (auto *document = _frame.Document())
    {
      document->SetStateForNewFormElements(currentItem->DocumentState());
    }
  }

  bool HistoryController::ShouldStopLoadingForHistoryItem(const HistoryItem &targetItem) const noexcept
  {
    RefPtr<HistoryItem> currentItem = _currentItem;
    if (!currentItem)
    {
      return false;
    }

    // Don't abort the current load unless it's associated with a different document.
    return currentItem->DocumentSequenceNumber() != targetItem.DocumentSequenceNumber();
  }

  // Main funnel for navigating to a previous location (back/forward, non-search snap-back)
  void HistoryController::GoToItem(HistoryItem &targetItem, FrameLoadType frameLoadType) noexcept
  {
    Ref<LocalFrame> frame = ShareRef(_frame);
    RefPtr<Page> page = ShareRefPtr(frame->GetPage());
    if (!page)
    {
      return;
    }

    Ref<HistoryItem> protectedTargetItem = ShareRef(targetItem);
    _policyItem = protectedTargetItem;

    RefPtr<HistoryItem> current = _currentItem;
    bool sameDocumentNavigation = current && targetItem.ShouldDoSameDocumentNavigationTo(*current);
    auto result = frame->Loader().Client().ShouldGoToHistoryItem(
      targetItem, sameDocumentNavigation ? IsSameDocumentNavigation::Yes : IsSameDocumentNavigation::No);

    if (_policyItem != protectedTargetItem.get())
    {
      return;
    }
    _policyItem = nullptr;

    if (result != ShouldGoToHistoryItem::Yes)
    {
      return;
    }

    // Set the BF cursor before commit, which lets the user quickly click back/forward again.
    auto &backForward = page->BackForward();
    RefPtr<HistoryItem> currentItem = backForward.CurrentItem();
    backForward.SetCurrentItem(targetItem);

    // First set the provisional item of any frames that are not actually navigating.
    RecursiveSetProvisionalItem(targetItem, currentItem.get());

    // Now that all other frames have provisional items, do the actual navigation.
    RecursiveGoToItem(targetItem, currentItem.get(), frameLoadType);
  }

  void HistoryController::GoToItemForNavigationAPI(HistoryItem &targetItem, FrameLoadType frameLoadType,
                                                   LocalFrame &triggeringFrame,
                                                   NavigationAPIMethodTracker *tracker) noexcept
  {
    Ref<LocalFrame> frame = ShareRef(_frame);
    RefPtr<Page> page = ShareRefPtr(frame->GetPage());
    if (!page)
    {
      return;
    }

    Ref<HistoryItem> protectedTargetItem = ShareRef(targetItem);
    RefPtr<NavigationAPIMethodTracker> protectedTracker = ShareRefPtr(tracker);
    _policyItem = protectedTargetItem;

    RefPtr<HistoryItem> current = _currentItem;
    bool sameDocumentNavigation = current && targetItem.ShouldDoSameDocumentNavigationTo(*current);
    auto result = frame->Loader().Client().ShouldGoToHistoryItem(
      targetItem, sameDocumentNavigation ? IsSameDocumentNavigation::Yes : IsSameDocumentNavigation::No);

    if (_policyItem != protectedTargetItem.get())
    {
      return;
    }
    _policyItem = nullptr;

    // Items the client doesn't know of are traversed to anyway.
    if (result == ShouldGoToHistoryItem::No)
    {
      return;
    }

    // Set the BF cursor before commit, which lets the user quickly click back/forward again.
    auto &backForward = page->BackForward();
    RefPtr<HistoryItem> currentItem = backForward.CurrentItem();
    backForward.SetCurrentItem(targetItem);

    RecursiveSetProvisionalItem(targetItem, currentItem.get());

    if (!ItemsAreClones(targetItem, currentItem.get()))
    {
      auto *window = frame->Document() ? frame->Document()->Window() : nullptr;
      if (!window)
      {
        return;
      }
      auto abortHandler = window->GetNavigation().RegisterAbortHandler();
      frame->Loader().LoadItem(targetItem, currentItem.get(), frameLoadType);
      // If the navigation was aborted (the navigate event was canceled), then reject the promises.
      if (abortHandler->WasAborted())
      {
        if (auto *triggeringWindow =
              triggeringFrame.Document() ? triggeringFrame.Document()->Window() : nullptr)
        {
          triggeringWindow->GetNavigation().RejectFinishedPromise(protectedTracker.get());
        }
      }
    }
  }

  void HistoryController::UpdateForBackForwardNavigation() noexcept
  {
    // Must grab the current scroll position before disturbing it
    if (!_frameLoadComplete)
    {
      SaveScrollPositionAndViewStateToItem(_previousItem.get());
    }

    UpdateCurrentItem();
  }

  void HistoryController::UpdateForReloadOrReplace() noexcept
  {
    if (RefPtr<HistoryItem> currentItem = _currentItem)
    {
      BackForwardCache::Singleton().Remove(*currentItem);

      auto loadType = _frame.Loader().LoadType();
      if (loadType == FrameLoadType::Reload || loadType == FrameLoadType::ReloadFromOrigin)
      {
        SaveScrollPositionAndViewStateToItem(currentItem.get());
      }
    }

    UpdateCurrentItem();
  }

  // There are 3 things you might think of as "history", all of which are handled by these functions.
  //
  //     1) Back/forward: The _currentItem is part of this mechanism.
  //     2) Global history: Handled by the client.
  //     3) Visited links: Handled by the PageGroup.
  void HistoryController::UpdateForStandardLoad(HistoryUpdateType updateType) noexcept
  {
    // FIXME: The client's global history and visited links, once there are those.
    auto &frameLoader = _frame.Loader();
    RefPtr<DocumentLoader> documentLoader = ShareRefPtr(frameLoader.GetDocumentLoader());
    if (!documentLoader)
    {
      return;
    }

    if (!documentLoader->IsClientRedirect())
    {
      if (!documentLoader->URLForHistory().IsEmpty()
          && updateType != HistoryUpdateType::UpdateAllExceptBackForwardList)
      {
        UpdateBackForwardListClippedAtTarget(true);
      }
    }
    else
    {
      // The client redirect replaces the current history item.
      UpdateCurrentItem();
    }
  }

  void HistoryController::UpdateForRedirectWithLockedBackForwardList() noexcept
  {
    RefPtr<DocumentLoader> documentLoader = ShareRefPtr(_frame.Loader().GetDocumentLoader());
    if (documentLoader && documentLoader->IsClientRedirect())
    {
      if (!_currentItem && !documentLoader->URLForHistory().IsEmpty())
      {
        UpdateBackForwardListClippedAtTarget(true);
      }
      // The client redirect replaces the current history item.
      UpdateCurrentItem();
    }
    // FIXME: A subframe's item in its parent's, once there are frames other than the main frame (not
    // planned).
  }

  void HistoryController::UpdateForClientRedirect() noexcept
  {
    // Clear out form data so we don't try to restore it into the incoming page. Must happen after webcore has
    // closed the URL and saved away the form state.
    if (RefPtr<HistoryItem> currentItem = _currentItem)
    {
      currentItem->ClearDocumentState();
      currentItem->ClearScrollPosition();
    }
  }

  void HistoryController::UpdateForCommit() noexcept
  {
    auto &frameLoader = _frame.Loader();
    auto type = frameLoader.LoadType();
    if (IsBackForwardLoadType(type) || IsReloadTypeWithProvisionalItem(type))
    {
      // Once committed, we want to use current item for saving DocState, and the provisional item for
      // restoring state. Note previousItem must be set before we close the URL, which will happen when the
      // data source is made non-provisional below
      krys_debug_assert(_provisionalItem);
      if (RefPtr<HistoryItem> provisionalItem = krys::move(_provisionalItem))
      {
        SetCurrentItem(ShareRef(*provisionalItem));
      }
    }
  }

  bool HistoryController::IsReloadTypeWithProvisionalItem(FrameLoadType type) const noexcept
  {
    return (type == FrameLoadType::Reload || type == FrameLoadType::ReloadFromOrigin) && _provisionalItem;
  }

  void HistoryController::UpdateForSameDocumentNavigation() noexcept
  {
    auto *document = _frame.Document();
    if (!document || document->GetURL().IsEmpty() || !_frame.GetPage())
    {
      return;
    }

    _policyItem = nullptr;

    // FIXME: Visited links and the client's global history, once there are those.
    if (RefPtr<HistoryItem> currentItem = _currentItem)
    {
      currentItem->SetURL(document->GetURL());
    }
  }

  void HistoryController::UpdateForFrameLoadCompleted() noexcept
  {
    // Even if already complete, we might have set a previous item on a frame that didn't do any data loading
    // on the past transaction. Make sure to track that the load is complete so that we use the current item
    // instead.
    _frameLoadComplete = true;
  }

  void HistoryController::SetCurrentItem(Ref<HistoryItem> &&item) noexcept
  {
    _frameLoadComplete = false;
    _previousItem = std::exchange(_currentItem, RefPtr<HistoryItem>(krys::move(item)));
  }

  void HistoryController::SetCurrentItemTitle(dom::DOMString &&title) noexcept
  {
    if (RefPtr<HistoryItem> currentItem = _currentItem)
    {
      currentItem->SetTitle(krys::move(title));
    }
  }

  bool HistoryController::CurrentItemShouldBeReplaced() const noexcept
  {
    // From the HTML5 spec for location.assign():
    //  "If the browsing context's session history contains only one Document,
    //   and that was the about:blank Document created when the browsing context
    //   was created, then the navigation must be done with replacement enabled."
    // An about:blank item with the host's markup isn't that document.
    RefPtr<HistoryItem> currentItem = _currentItem;
    return currentItem && !_previousItem && !currentItem->GetSubstituteData()
           && text::IsASCIICaselessEqual(currentItem->URL().String(), utf8_stringview(u8"about:blank"));
  }

  void HistoryController::ClearPreviousItem() noexcept
  {
    _previousItem = nullptr;
  }

  void HistoryController::SetProvisionalItem(RefPtr<HistoryItem> &&item) noexcept
  {
    _provisionalItem = krys::move(item);
  }

  void HistoryController::InitializeItem(HistoryItem &item, DocumentLoader *documentLoader) noexcept
  {
    krys_debug_assert(documentLoader);
    if (!documentLoader)
    {
      return;
    }

    auto url = documentLoader->DocumentURL();
    auto originalURL = documentLoader->Request().URL();

    // Frames that have never successfully loaded any content may have no URL at all. Currently our history
    // code can't deal with such things, so we nip that in the bud here.
    if (url.IsEmpty())
    {
      url = url::URL(u8"about:blank");
    }
    if (originalURL.IsEmpty())
    {
      originalURL = url::URL(u8"about:blank");
    }

    item.SetURL(url);
    item.SetTarget(dom::DOMStringAtom(_frame.SpecifiedName()));
    // FIXME: The document's title (documentLoader->title()), once documents tell their loader when it
    // changes.
    item.SetOriginalURL(originalURL);

    // FIXME: Noting a failed visit (an HTTP error status), once responses have statuses.

    // The host's markup is kept, to load it again.
    item.SetSubstituteData(Maybe<SubstituteData>(documentLoader->GetSubstituteData()));

    // FIXME: The referrer and a POST's body (setFormInfoFromRequest), once requests carry them.
  }

  Ref<HistoryItem> HistoryController::CreateItem(BackForwardItemIdentifier itemID) noexcept
  {
    auto item = HistoryItem::Create({}, {}, {}, itemID);
    InitializeItem(*item, _frame.Loader().GetDocumentLoader());

    // Set the item for which we will save document state
    SetCurrentItem(Ref<HistoryItem>(item));

    return item;
  }

  Ref<HistoryItem> HistoryController::CreateItemWithLoader(DocumentLoader *documentLoader) noexcept
  {
    auto item = HistoryItem::Create();
    InitializeItem(*item, documentLoader);
    return item;
  }

  Ref<HistoryItem> HistoryController::CreateItemTree(LocalFrame &targetFrame, bool clipAtTarget,
                                                     BackForwardItemIdentifier itemID) noexcept
  {
    auto item = CreateItem(itemID);
    if (!_frameLoadComplete)
    {
      SaveScrollPositionAndViewStateToItem(_previousItem.get());
    }

    if (!clipAtTarget || &_frame != &targetFrame)
    {
      // save frame state for items that aren't loading (khtml doesn't save those)
      SaveDocumentState();

      // clipAtTarget is false for navigations within the same document, so we should copy the
      // documentSequenceNumber over to the newly create item. Non-target items are just clones, and they
      // should therefore preserve the same itemSequenceNumber.
      if (RefPtr<HistoryItem> previousItem = _previousItem)
      {
        if (&_frame != &targetFrame)
        {
          item->SetItemSequenceNumber(previousItem->ItemSequenceNumber());
        }
        item->SetDocumentSequenceNumber(previousItem->DocumentSequenceNumber());
      }
    }

    // FIXME: Eliminate the isTargetItem flag in favor of itemSequenceNumber.
    if (&_frame == &targetFrame)
    {
      item->SetIsTargetItem(true);
    }
    return item;
  }

  // The general idea here is to traverse the frame tree and the item tree in parallel, tracking whether each
  // frame already has the content the item requests. If there is a match, we set the provisional item and
  // recurse. Otherwise we will reload that frame and all its kids in recursiveGoToItem.
  void HistoryController::RecursiveSetProvisionalItem(HistoryItem &item, HistoryItem *fromItem) noexcept
  {
    if (!ItemsAreClones(item, fromItem))
    {
      return;
    }

    // Set provisional item, which will be committed in recursiveUpdateForCommit.
    _provisionalItem = ShareRefPtr(&item);
  }

  // We now traverse the frame tree and item tree a second time, loading frames that do have the content the
  // item requests.
  void HistoryController::RecursiveGoToItem(HistoryItem &item, HistoryItem *fromItem,
                                            FrameLoadType type) noexcept
  {
    if (!ItemsAreClones(item, fromItem))
    {
      _frame.Loader().LoadItem(item, fromItem, type);
    }
  }

  bool HistoryController::ItemsAreClones(const HistoryItem &item1, const HistoryItem *item2) noexcept
  {
    // If the item we're going to is a clone of the item we're at, then we do not need to load it again. The
    // current frame tree and the frame tree snapshot in the item have to match.
    // Note: Some clients treat a navigation to the current history item as a reload. Thus, if item1 and item2
    // are the same, we need to create a new document and should not consider them clones.
    return item2 && item1.ItemID() != item2->ItemID()
           && item1.ItemSequenceNumber() == item2->ItemSequenceNumber();
  }

  void HistoryController::UpdateBackForwardListClippedAtTarget(bool doClip) noexcept
  {
    // In the case of saving state about a page with frames, we store a tree of items that mirrors the frame
    // tree. The item that was the target of the user's navigation is designated as the "targetItem".
    RefPtr<Page> page = ShareRefPtr(_frame.GetPage());
    auto *documentLoader = _frame.Loader().GetDocumentLoader();
    if (!page || !documentLoader || documentLoader->URLForHistory().IsEmpty())
    {
      return;
    }

    page->BackForward().AddItem(CreateItemTree(_frame, doClip, GenerateBackForwardItemIdentifier()));
  }

  void HistoryController::UpdateCurrentItem() noexcept
  {
    RefPtr<HistoryItem> currentItem = _currentItem;
    if (!currentItem)
    {
      return;
    }

    auto *documentLoader = _frame.Loader().GetDocumentLoader();
    if (!documentLoader)
    {
      return;
    }

    if (currentItem->URL() != documentLoader->DocumentURL())
    {
      // We ended up on a completely different URL this time, so the HistoryItem needs to be re-initialized.
      // Preserve the isTargetItem flag as it is a property of how this HistoryItem was originally created and
      // is not dependent on the document.
      bool isTargetItem = currentItem->IsTargetItem();
      auto uuidIdentifier = currentItem->UUIDIdentifier();
      currentItem->Reset();
      InitializeItem(*currentItem, documentLoader);
      // FIXME: Only for the same origin, once there are origins.
      currentItem->SetUUIDIdentifier(uuidIdentifier);
      currentItem->SetIsTargetItem(isTargetItem);
    }
    else
    {
      // Even if the final URL didn't change, the document may have come from other markup.
      currentItem->SetSubstituteData(Maybe<SubstituteData>(documentLoader->GetSubstituteData()));
    }
  }

  void
    HistoryController::UpdateBackForwardListForReplaceState(RefPtr<html::SerializedScriptValue> &&stateObject,
                                                            const url::URL &url) noexcept
  {
    RefPtr<HistoryItem> currentItem = _currentItem;
    if (!currentItem)
    {
      return;
    }

    if (!url.IsEmpty())
    {
      currentItem->SetURLString(url);
    }
    currentItem->SetStateObject(krys::move(stateObject));
  }

  void HistoryController::PushState(RefPtr<html::SerializedScriptValue> &&stateObject,
                                    const url::URL &url) noexcept
  {
    RefPtr<HistoryItem> currentItem = _currentItem;
    if (!currentItem)
    {
      return;
    }

    RefPtr<Page> page = ShareRefPtr(_frame.GetPage());
    if (!page)
    {
      return;
    }

    bool shouldRestoreScrollPosition = currentItem->ShouldRestoreScrollPosition();

    // Get a HistoryItem tree for the current frame tree.
    auto topItem = CreateItemTree(_frame, false, GenerateBackForwardItemIdentifier());

    auto *window = _frame.Document() ? _frame.Document()->Window() : nullptr;
    if (!window || !window->HasTransientActivation())
    {
      topItem->SetWasCreatedByJSWithoutUserInteraction(true);
    }

    // Override data in the current item (created by createItemTree) to reflect the pushState() arguments.
    currentItem = _currentItem;
    currentItem->SetStateObject(krys::move(stateObject));
    currentItem->SetURLString(url);
    currentItem->SetShouldRestoreScrollPosition(shouldRestoreScrollPosition);

    page->BackForward().AddItem(krys::move(topItem));

    // FIXME: Visited links and the client's global history, once there are those.
    if (window)
    {
      window->GetNavigation().UpdateForNavigation(ShareRef(*currentItem), NavigationNavigationType::Push);
    }
  }

  void HistoryController::ReplaceState(RefPtr<html::SerializedScriptValue> &&stateObject,
                                       const url::URL &url) noexcept
  {
    if (!_currentItem)
    {
      return;
    }

    UpdateBackForwardListForReplaceState(krys::move(stateObject), url);

    // FIXME: Visited links and the client's global history, once there are those.
    if (auto *window = _frame.Document() ? _frame.Document()->Window() : nullptr)
    {
      RefPtr<HistoryItem> currentItem = _currentItem;
      currentItem->SetNavigationAPIStateObject(nullptr);
      window->GetNavigation().UpdateForNavigation(ShareRef(*currentItem), NavigationNavigationType::Replace);
    }
  }

  void HistoryController::ReplaceCurrentItem(RefPtr<HistoryItem> &&item) noexcept
  {
    if (!item)
    {
      return;
    }

    _previousItem = nullptr;
    if (_provisionalItem)
    {
      _provisionalItem = krys::move(item);
    }
    else
    {
      _currentItem = krys::move(item);
    }
  }
}
