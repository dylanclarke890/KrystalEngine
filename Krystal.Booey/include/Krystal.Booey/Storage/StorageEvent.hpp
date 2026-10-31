#pragma once

#include "Krystal.Booey/DOM/Dicts/EventInit.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class Storage;

  /// @see https://html.spec.whatwg.org/#storageeventinit
  struct StorageEventInit : dom::EventInit
  {
    Maybe<dom::DOMString> Key;
    Maybe<dom::DOMString> OldValue;
    Maybe<dom::DOMString> NewValue;
    dom::USVString URL;
    RefPtr<Storage> StorageArea;
  };

  /// @brief Another window of the same origin changed a storage area the window uses.
  /// @see https://html.spec.whatwg.org/#the-storageevent-interface
  class StorageEvent final : public dom::Event
  {
  private:
    Maybe<dom::DOMString> _key;
    Maybe<dom::DOMString> _oldValue;
    Maybe<dom::DOMString> _newValue;
    dom::USVString _url;
    RefPtr<Storage> _storageArea;

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = StorageEventInit;

    /// @see https://html.spec.whatwg.org/#dom-storageevent-storageevent
    StorageEvent(dom::DOMStringAtom type, const StorageEventInit &eventInitDict = {}) noexcept;
    ~StorageEvent() noexcept override;

    /// @brief A trusted `storage` event.
    KRYS_NODISCARD static Ref<StorageEvent> Create(Maybe<dom::DOMString> key, Maybe<dom::DOMString> oldValue,
                                                   Maybe<dom::DOMString> newValue, dom::USVString url,
                                                   RefPtr<Storage> storageArea) noexcept;

    /// @brief The key changed; null for `clear()`.
    /// @see https://html.spec.whatwg.org/#dom-storageevent-key
    KRYS_NODISCARD const Maybe<dom::DOMString> &Key() const noexcept
    {
      return _key;
    }

    /// @see https://html.spec.whatwg.org/#dom-storageevent-oldvalue
    KRYS_NODISCARD const Maybe<dom::DOMString> &OldValue() const noexcept
    {
      return _oldValue;
    }

    /// @see https://html.spec.whatwg.org/#dom-storageevent-newvalue
    KRYS_NODISCARD const Maybe<dom::DOMString> &NewValue() const noexcept
    {
      return _newValue;
    }

    /// @brief The URL of the document whose storage changed.
    /// @see https://html.spec.whatwg.org/#dom-storageevent-url
    KRYS_NODISCARD const dom::USVString &URL() const noexcept
    {
      return _url;
    }

    /// @brief The receiving window's `Storage` object for the area.
    /// @see https://html.spec.whatwg.org/#dom-storageevent-storagearea
    KRYS_NODISCARD Storage *StorageArea() const noexcept;

    /// @note Legacy.
    /// @see https://html.spec.whatwg.org/#dom-storageevent-initstorageevent
    void InitStorageEvent(dom::DOMStringAtom type, bool bubbles = false, bool cancelable = false,
                          Maybe<dom::DOMString> key = null, Maybe<dom::DOMString> oldValue = null,
                          Maybe<dom::DOMString> newValue = null, dom::USVString url = {},
                          Storage *storageArea = nullptr) noexcept;

    KRYS_NODISCARD bool IsStorageEvent() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::StorageEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsStorageEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
