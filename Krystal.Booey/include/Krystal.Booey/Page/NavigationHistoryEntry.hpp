#pragma once

#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/History/HistoryItem.hpp"
#include "Krystal.Booey/HTML/SerializedScriptValue.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  class Navigation;

  /// @brief One of a window's `navigation.entries()`: a session history entry of its document's (or one of
  /// the same origin's), with a key that names the slot in the history and an ID that names the entry, its
  /// URL and its state; `dispose` fires at it once it's no longer in the history.
  /// @see https://html.spec.whatwg.org/#navigationhistoryentry
  class NavigationHistoryEntry final : public dom::EventTarget
  {
  private:
    WeakPtr<Navigation> _navigation;
    WeakPtr<dom::Document> _document;
    dom::USVString _urlString;
    dom::DOMString _key;
    dom::DOMString _id;
    RefPtr<html::SerializedScriptValue> _state;
    Ref<HistoryItem> _associatedHistoryItem;
    bool _hasDispatchedDisposeEvent {false};

  public:
    NavigationHistoryEntry(Navigation &navigation, Ref<HistoryItem> &&historyItem, dom::USVString &&urlString,
                           dom::DOMString &&key, RefPtr<html::SerializedScriptValue> &&state,
                           dom::DOMString &&id) noexcept;
    ~NavigationHistoryEntry() noexcept override;

    /// @brief An entry for @p historyItem.
    KRYS_NODISCARD static Ref<NavigationHistoryEntry> Create(Navigation &navigation,
                                                             Ref<HistoryItem> &&historyItem) noexcept;
    /// @brief A copy of @p other (another window's), with its key, ID and state.
    KRYS_NODISCARD static Ref<NavigationHistoryEntry> Create(Navigation &navigation,
                                                             const NavigationHistoryEntry &other) noexcept;

    /// @brief The entry's URL; null if its document isn't fully active.
    /// @see https://html.spec.whatwg.org/#dom-navigationhistoryentry-url
    KRYS_NODISCARD Maybe<dom::USVString> URL() const noexcept;
    /// @see https://html.spec.whatwg.org/#dom-navigationhistoryentry-key
    KRYS_NODISCARD dom::DOMString Key() const noexcept;
    /// @see https://html.spec.whatwg.org/#dom-navigationhistoryentry-id
    KRYS_NODISCARD dom::DOMString Id() const noexcept;
    /// @brief The entry's index in `navigation.entries()`; -1 if it isn't in them.
    /// @see https://html.spec.whatwg.org/#dom-navigationhistoryentry-index
    KRYS_NODISCARD int64 Index() const noexcept;
    /// @brief Whether the entry is for the current document.
    /// @see https://html.spec.whatwg.org/#dom-navigationhistoryentry-samedocument
    KRYS_NODISCARD bool SameDocument() const noexcept;
    /// @brief The entry's state (`navigate()`'s, `reload()`'s or `updateCurrentEntry()`'s); null if it has
    /// none or its document isn't fully active.
    /// @see https://html.spec.whatwg.org/#dom-navigationhistoryentry-getstate
    KRYS_NODISCARD html::SerializedScriptValue *GetState() const noexcept;

    void SetState(RefPtr<html::SerializedScriptValue> &&state) noexcept;
    /// @brief The state, whether or not the document is fully active.
    KRYS_NODISCARD html::SerializedScriptValue *State() const noexcept
    {
      return _state.get();
    }

    KRYS_NODISCARD HistoryItem &AssociatedHistoryItem() const noexcept
    {
      return *_associatedHistoryItem;
    }

    /// @brief Fires `dispose` at the entry, which is no longer in the session history.
    void DispatchDisposeEvent() noexcept;

  private:
    KRYS_NODISCARD bool IsDocumentFullyActive() const noexcept;
  };
}
