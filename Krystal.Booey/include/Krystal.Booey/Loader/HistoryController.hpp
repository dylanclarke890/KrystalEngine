#pragma once

#include "Krystal.Booey/History/HistoryItem.hpp"
#include "Krystal.Booey/Loader/FrameLoaderTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  class SerializedScriptValue;
}

namespace krys::boo
{
  class DocumentLoader;
  class LocalFrame;

  /// @brief A frame's part in its page's session history: the history items of the documents it loads (one
  /// for each new load or fragment navigation, the current one updated for reloads and replacements), going
  /// to an item (back and forward), and saving and restoring a document's scroll position and form state with
  /// its item.
  // FIXME: Frame trees, whose items are trees of their frames' items (not planned).
  class HistoryController final
  {
    KRYS_NON_COPY_MOVABLE(HistoryController);

  private:
    LocalFrame &_frame;

    RefPtr<HistoryItem> _currentItem;
    RefPtr<HistoryItem> _previousItem;
    RefPtr<HistoryItem> _provisionalItem;
    RefPtr<HistoryItem> _policyItem;

    bool _frameLoadComplete {true};

  public:
    enum class HistoryUpdateType : uint8
    {
      UpdateAll,
      UpdateAllExceptBackForwardList
    };

    explicit HistoryController(LocalFrame &frame) noexcept;
    ~HistoryController() noexcept;

    /// @brief Saves the frame's scroll position to @p item.
    void SaveScrollPositionAndViewStateToItem(HistoryItem *item) noexcept;

    /// @brief Scrolls the frame to its current item's scroll position, unless the user scrolled it already.
    void RestoreScrollPositionAndViewState() noexcept;

    void UpdateBackForwardListForFragmentScroll() noexcept;
    void UpdateBackForwardListForReplaceState(RefPtr<html::SerializedScriptValue> &&stateObject,
                                              const url::URL &url) noexcept;

    /// @brief Saves the document's form state to its item.
    void SaveDocumentState() noexcept;
    void SaveDocumentAndScrollState() noexcept;
    /// @brief Gives the new document the form state of the item it's loading for.
    void RestoreDocumentState() noexcept;

    void UpdateForBackForwardNavigation() noexcept;
    void UpdateForReloadOrReplace() noexcept;
    void UpdateForStandardLoad(HistoryUpdateType updateType = HistoryUpdateType::UpdateAll) noexcept;
    void UpdateForRedirectWithLockedBackForwardList() noexcept;
    void UpdateForClientRedirect() noexcept;
    void UpdateForCommit() noexcept;
    void UpdateForSameDocumentNavigation() noexcept;
    void UpdateForFrameLoadCompleted() noexcept;

    KRYS_NODISCARD HistoryItem *CurrentItem() const noexcept
    {
      return _currentItem.get();
    }
    void SetCurrentItem(Ref<HistoryItem> &&item) noexcept;
    void SetCurrentItemTitle(dom::DOMString &&title) noexcept;
    /// @brief Whether the next location change replaces the current item: the only one, for `about:blank`.
    KRYS_NODISCARD bool CurrentItemShouldBeReplaced() const noexcept;
    void ReplaceCurrentItem(RefPtr<HistoryItem> &&item) noexcept;

    KRYS_NODISCARD HistoryItem *PreviousItem() const noexcept
    {
      return _previousItem.get();
    }
    void ClearPreviousItem() noexcept;

    KRYS_NODISCARD HistoryItem *ProvisionalItem() const noexcept
    {
      return _provisionalItem.get();
    }
    void SetProvisionalItem(RefPtr<HistoryItem> &&item) noexcept;

    /// @brief Adds an item for the document at @p url, with @p stateObject (`history.pushState()`).
    void PushState(RefPtr<html::SerializedScriptValue> &&stateObject, const url::URL &url) noexcept;
    /// @brief Gives the current item @p url and @p stateObject (`history.replaceState()`).
    void ReplaceState(RefPtr<html::SerializedScriptValue> &&stateObject, const url::URL &url) noexcept;

    /// @brief An item for @p documentLoader's document, which isn't the current one.
    KRYS_NODISCARD Ref<HistoryItem> CreateItemWithLoader(DocumentLoader *documentLoader) noexcept;

    /// @brief A new current item for the frame's document, which becomes the current item.
    KRYS_NODISCARD Ref<HistoryItem> CreateItemTree(LocalFrame &targetFrame, bool clipAtTarget,
                                                   BackForwardItemIdentifier itemID) noexcept;

    void ClearPolicyItem() noexcept
    {
      _policyItem = nullptr;
    }

    /// @brief Whether going to @p targetItem leaves the document, so the frame's load stops first.
    KRYS_NODISCARD bool ShouldStopLoadingForHistoryItem(const HistoryItem &targetItem) const noexcept;

    /// @brief Goes to @p targetItem (back or forward), if the client allows it.
    void GoToItem(HistoryItem &targetItem, FrameLoadType frameLoadType) noexcept;

    /// @brief Goes to @p targetItem for the Navigation API: if its `navigate` is canceled, @p tracker's
    /// promises (@p triggeringFrame's window's) are rejected.
    void GoToItemForNavigationAPI(HistoryItem &targetItem, FrameLoadType frameLoadType,
                                  LocalFrame &triggeringFrame,
                                  struct NavigationAPIMethodTracker *tracker) noexcept;

  private:
    void InitializeItem(HistoryItem &item, DocumentLoader *documentLoader) noexcept;
    KRYS_NODISCARD Ref<HistoryItem> CreateItem(BackForwardItemIdentifier itemID) noexcept;

    void RecursiveSetProvisionalItem(HistoryItem &item, HistoryItem *fromItem) noexcept;
    void RecursiveGoToItem(HistoryItem &item, HistoryItem *fromItem, FrameLoadType type) noexcept;
    KRYS_NODISCARD bool IsReloadTypeWithProvisionalItem(FrameLoadType type) const noexcept;
    KRYS_NODISCARD static bool ItemsAreClones(const HistoryItem &item1, const HistoryItem *item2) noexcept;
    void UpdateBackForwardListClippedAtTarget(bool doClip) noexcept;
    void UpdateCurrentItem() noexcept;
  };
}
