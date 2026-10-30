#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/HTML/SerializedScriptValue.hpp"
#include "Krystal.Booey/Loader/SubstituteData.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  /// @brief Identifies a history item in its back/forward list.
  using BackForwardItemIdentifier = uint64;

  /// @brief A new history item identifier.
  KRYS_NODISCARD BackForwardItemIdentifier GenerateBackForwardItemIdentifier() noexcept;

  /// @brief A random (version 4) UUID, in its string form.
  KRYS_NODISCARD dom::DOMString CreateVersion4UUIDString() noexcept;

  /// @brief An entry of a page's session history: the URL of a document the frame showed, its title, and
  /// what's restored when the frame goes back to it: its scroll position, its form controls' state, its state
  /// object, and the markup it came from if the host gave it.
  /// @see https://html.spec.whatwg.org/#session-history-entry
  // FIXME: Child items, once there are frames other than the main frame (not planned).
  class HistoryItem final : public RefCounted<HistoryItem>, public CanMakeWeakPtr<HistoryItem>
  {
    KRYS_NON_MOVABLE(HistoryItem);

  private:
    url::URL _url;
    url::URL _originalURL;
    dom::DOMString _referrer;
    dom::DOMStringAtom _target {dom::DOMStringAtom::Null()};
    dom::DOMString _title;
    dom::DOMString _displayTitle;

    IntPoint _scrollPosition;
    float _pageScaleFactor {0}; // 0 indicates "unset".
    List<dom::DOMString> _documentState;

    bool _lastVisitWasFailure {false};
    bool _wasCreatedByJSWithoutUserInteraction {false};
    bool _shouldRestoreScrollPosition {true};
    bool _isTargetItem {false};

    // If two HistoryItems have the same item sequence number, then they are clones of one another. Traversing
    // history from one such HistoryItem to another is a no-op. HistoryItem clones are created for parent and
    // sibling frames when only a subframe navigates.
    int64 _itemSequenceNumber {GenerateSequenceNumber()};

    // If two HistoryItems have the same document sequence number, then they refer to the same instance of a
    // document. Traversing history from one such HistoryItem to another preserves the document.
    int64 _documentSequenceNumber {GenerateSequenceNumber()};

    // Support for HTML5 History
    RefPtr<html::SerializedScriptValue> _stateObject;

    // Navigation API
    RefPtr<html::SerializedScriptValue> _navigationAPIStateObject;

    /// @brief The markup the document came from, when the host gave it rather than a URL to load.
    Maybe<SubstituteData> _substituteData;

    BackForwardItemIdentifier _itemID;
    /// @brief The Navigation API's key for the item's slot in the session history.
    dom::DOMString _uuidIdentifier;

  public:
    explicit HistoryItem(url::URL &&url = {}, dom::DOMString &&title = {},
                         dom::DOMString &&alternateTitle = {},
                         Maybe<BackForwardItemIdentifier> itemID = null) noexcept;
    ~HistoryItem() noexcept;

    KRYS_NODISCARD static Ref<HistoryItem> Create(url::URL &&url = {}, dom::DOMString &&title = {},
                                                  dom::DOMString &&alternateTitle = {},
                                                  Maybe<BackForwardItemIdentifier> itemID = null) noexcept
    {
      return CreateRef<HistoryItem>(krys::move(url), krys::move(title), krys::move(alternateTitle), itemID);
    }

    /// @brief A copy of the item, with the same identifier.
    KRYS_NODISCARD Ref<HistoryItem> Copy() const noexcept;

    KRYS_NODISCARD BackForwardItemIdentifier ItemID() const noexcept
    {
      return _itemID;
    }

    /// @brief The Navigation API's key for the item (`NavigationHistoryEntry.key`).
    KRYS_NODISCARD const dom::DOMString &UUIDIdentifier() const noexcept
    {
      return _uuidIdentifier;
    }
    void SetUUIDIdentifier(const dom::DOMString &uuidIdentifier) noexcept
    {
      _uuidIdentifier = uuidIdentifier;
    }

    /// @brief Whether the back/forward cache keeps the item's page.
    KRYS_NODISCARD bool IsInBackForwardCache() const noexcept;
    /// @brief Whether the page the back/forward cache keeps for the item has expired.
    KRYS_NODISCARD bool HasCachedPageExpired() const noexcept;

    /// @brief Resets the HistoryItem to its initial state, as returned by Create().
    void Reset() noexcept;

    KRYS_NODISCARD bool operator==(const HistoryItem &other) const noexcept
    {
      return ItemID() == other.ItemID();
    }

    KRYS_NODISCARD const url::URL &URL() const noexcept
    {
      return _url;
    }

    /// @brief The first URL we loaded to get to where this history item points.
    KRYS_NODISCARD const url::URL &OriginalURL() const noexcept
    {
      return _originalURL;
    }

    KRYS_NODISCARD const dom::DOMString &Title() const noexcept
    {
      return _title;
    }

    KRYS_NODISCARD const dom::DOMString &AlternateTitle() const noexcept
    {
      return _displayTitle;
    }
    void SetAlternateTitle(dom::DOMString &&alternateTitle) noexcept
    {
      _displayTitle = krys::move(alternateTitle);
    }

    KRYS_NODISCARD const dom::DOMString &Referrer() const noexcept
    {
      return _referrer;
    }

    /// @brief The name of the frame the item is for.
    KRYS_NODISCARD dom::DOMStringAtom Target() const noexcept
    {
      return _target;
    }

    KRYS_NODISCARD bool IsTargetItem() const noexcept
    {
      return _isTargetItem;
    }
    void SetIsTargetItem(bool isTargetItem) noexcept
    {
      _isTargetItem = isTargetItem;
    }

    KRYS_NODISCARD bool LastVisitWasFailure() const noexcept
    {
      return _lastVisitWasFailure;
    }
    void SetLastVisitWasFailure(bool wasFailure) noexcept
    {
      _lastVisitWasFailure = wasFailure;
    }

    KRYS_NODISCARD const IntPoint &ScrollPosition() const noexcept
    {
      return _scrollPosition;
    }
    void SetScrollPosition(const IntPoint &position) noexcept
    {
      _scrollPosition = position;
    }
    void ClearScrollPosition() noexcept
    {
      _scrollPosition = IntPoint();
    }

    /// @brief The page's scale when the item was left, or 0 if it wasn't saved.
    KRYS_NODISCARD float PageScaleFactor() const noexcept
    {
      return _pageScaleFactor;
    }
    void SetPageScaleFactor(float scaleFactor) noexcept
    {
      _pageScaleFactor = scaleFactor;
    }

    /// @brief Whether going back to the item restores its scroll position (`history.scrollRestoration`).
    KRYS_NODISCARD bool ShouldRestoreScrollPosition() const noexcept
    {
      return _shouldRestoreScrollPosition;
    }
    void SetShouldRestoreScrollPosition(bool shouldRestore) noexcept
    {
      _shouldRestoreScrollPosition = shouldRestore;
    }

    /// @brief The state of the document's form controls, saved as it's left.
    KRYS_NODISCARD const List<dom::DOMString> &DocumentState() const noexcept
    {
      return _documentState;
    }
    void SetDocumentState(List<dom::DOMString> &&state) noexcept
    {
      _documentState = krys::move(state);
    }
    void ClearDocumentState() noexcept
    {
      _documentState.clear();
    }

    /// @brief Sets the item's URL, which drops its document state.
    void SetURL(const url::URL &url) noexcept;
    /// @brief Sets the item's URL, keeping its document state.
    void SetURLString(const url::URL &url) noexcept
    {
      _url = url;
    }
    void SetOriginalURL(const url::URL &url) noexcept
    {
      _originalURL = url;
    }
    void SetReferrer(dom::DOMString &&referrer) noexcept
    {
      _referrer = krys::move(referrer);
    }
    void SetTarget(dom::DOMStringAtom target) noexcept
    {
      _target = target;
    }
    void SetTitle(dom::DOMString &&title) noexcept
    {
      _title = krys::move(title);
    }

    KRYS_NODISCARD html::SerializedScriptValue *StateObject() const noexcept
    {
      return _stateObject.get();
    }
    void SetStateObject(RefPtr<html::SerializedScriptValue> &&object) noexcept;

    /// @see https://html.spec.whatwg.org/#she-navigation-api-state
    KRYS_NODISCARD html::SerializedScriptValue *NavigationAPIStateObject() const noexcept
    {
      return _navigationAPIStateObject.get();
    }
    void SetNavigationAPIStateObject(RefPtr<html::SerializedScriptValue> &&object) noexcept;

    KRYS_NODISCARD int64 ItemSequenceNumber() const noexcept
    {
      return _itemSequenceNumber;
    }
    void SetItemSequenceNumber(int64 number) noexcept
    {
      _itemSequenceNumber = number;
    }

    KRYS_NODISCARD int64 DocumentSequenceNumber() const noexcept
    {
      return _documentSequenceNumber;
    }
    void SetDocumentSequenceNumber(int64 number) noexcept
    {
      _documentSequenceNumber = number;
    }

    /// @brief The markup the item's document came from, if the host gave it; going back to the item loads it
    /// again.
    KRYS_NODISCARD const Maybe<SubstituteData> &GetSubstituteData() const noexcept
    {
      return _substituteData;
    }
    void SetSubstituteData(Maybe<SubstituteData> &&substituteData) noexcept
    {
      _substituteData = krys::move(substituteData);
    }

    /// @brief Whether going from this item to @p otherItem stays in the document: they're different items
    /// for the same document (a fragment navigation or a pushed state).
    KRYS_NODISCARD bool ShouldDoSameDocumentNavigationTo(const HistoryItem &otherItem) const noexcept;

    /// @brief Whether the item is for @p document.
    KRYS_NODISCARD bool IsCurrentDocument(const dom::Document &document) const noexcept;

    KRYS_NODISCARD bool WasCreatedByJSWithoutUserInteraction() const noexcept
    {
      return _wasCreatedByJSWithoutUserInteraction;
    }
    void SetWasCreatedByJSWithoutUserInteraction(bool wasCreatedByJSWithoutUserInteraction) noexcept
    {
      _wasCreatedByJSWithoutUserInteraction = wasCreatedByJSWithoutUserInteraction;
    }

  private:
    HistoryItem(const HistoryItem &item) noexcept;
    HistoryItem &operator=(const HistoryItem &) = delete;

    KRYS_NODISCARD static int64 GenerateSequenceNumber() noexcept;
  };
}
