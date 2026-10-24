#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/HTML/SerializedScriptValue.hpp"
#include "Krystal.Booey/Page/NavigationHistoryEntry.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief Where a navigation goes, as `navigate`'s event reports it: a URL, and the entry for a traversal.
  /// @see https://html.spec.whatwg.org/#navigationdestination
  class NavigationDestination final : public RefCounted<NavigationDestination>
  {
    KRYS_NON_COPY_MOVABLE(NavigationDestination);

  private:
    RefPtr<NavigationHistoryEntry> _entry;
    url::URL _url;
    bool _isSameDocument;
    RefPtr<html::SerializedScriptValue> _stateObject;

  public:
    NavigationDestination(const url::URL &url, RefPtr<NavigationHistoryEntry> &&entry,
                          bool isSameDocument) noexcept
        : _entry(krys::move(entry)), _url(url), _isSameDocument(isSameDocument)
    {
    }

    KRYS_NODISCARD static Ref<NavigationDestination>
      Create(const url::URL &url, RefPtr<NavigationHistoryEntry> &&entry, bool isSameDocument) noexcept
    {
      return CreateRef<NavigationDestination>(url, krys::move(entry), isSameDocument);
    }

    /// @see https://html.spec.whatwg.org/#dom-navigationdestination-url
    KRYS_NODISCARD const url::URL &URL() const noexcept
    {
      return _url;
    }

    /// @brief The destination entry's key; empty if it isn't an entry (not a traversal).
    /// @see https://html.spec.whatwg.org/#dom-navigationdestination-key
    KRYS_NODISCARD dom::DOMString Key() const noexcept
    {
      return _entry ? _entry->Key() : dom::DOMString();
    }

    /// @see https://html.spec.whatwg.org/#dom-navigationdestination-id
    KRYS_NODISCARD dom::DOMString Id() const noexcept
    {
      return _entry ? _entry->Id() : dom::DOMString();
    }

    /// @see https://html.spec.whatwg.org/#dom-navigationdestination-index
    KRYS_NODISCARD int64 Index() const noexcept
    {
      return _entry ? _entry->Index() : -1;
    }

    /// @see https://html.spec.whatwg.org/#dom-navigationdestination-samedocument
    KRYS_NODISCARD bool SameDocument() const noexcept
    {
      return _isSameDocument;
    }

    /// @brief The state the navigation gives the destination (`navigate()`'s, `pushState()`'s); null if none.
    /// @see https://html.spec.whatwg.org/#dom-navigationdestination-getstate
    KRYS_NODISCARD html::SerializedScriptValue *GetState() const noexcept
    {
      return _stateObject.get();
    }

    void SetStateObject(html::SerializedScriptValue *stateObject) noexcept
    {
      _stateObject = ShareRefPtr(stateObject);
    }

    /// @brief Whether the destination names a history entry (it's a traversal).
    KRYS_NODISCARD bool HasEntry() const noexcept
    {
      return _entry != nullptr;
    }
  };
}
