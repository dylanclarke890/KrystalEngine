#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Storage/StorageArea.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class LocalDOMWindow;
  class LocalFrame;

  /// @brief `sessionStorage` or `localStorage`: a window's view of a storage area. Changing it fires
  /// `storage` at the other windows of the same origin that use the same area.
  /// @see https://html.spec.whatwg.org/#the-storage-interface
  class Storage final : public RefCounted<Storage>
  {
    KRYS_NON_COPY_MOVABLE(Storage);

  private:
    WeakPtr<LocalDOMWindow> _window;
    Ref<StorageArea> _storageArea;

  public:
    Storage(LocalDOMWindow &window, Ref<StorageArea> &&storageArea) noexcept;
    ~Storage() noexcept;

    KRYS_NODISCARD static Ref<Storage> Create(LocalDOMWindow &window, Ref<StorageArea> &&storageArea) noexcept
    {
      return CreateRef<Storage>(window, krys::move(storageArea));
    }

    /// @see https://html.spec.whatwg.org/#dom-storage-length
    KRYS_NODISCARD uint32 Length() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-storage-key
    KRYS_NODISCARD Maybe<dom::DOMString> Key(uint32 index) const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-storage-getitem
    KRYS_NODISCARD Maybe<dom::DOMString> GetItem(dom::DOMStringView key) const noexcept;

    /// @return `QuotaExceededError` if the area's quota doesn't allow the item.
    /// @see https://html.spec.whatwg.org/#dom-storage-setitem
    dom::ExceptionOr<void> SetItem(const dom::DOMString &key, const dom::DOMString &value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-storage-removeitem
    dom::ExceptionOr<void> RemoveItem(const dom::DOMString &key) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-storage-clear
    dom::ExceptionOr<void> Clear() noexcept;

    /// @brief The keys, which are the object's supported property names.
    KRYS_NODISCARD List<dom::DOMString> SupportedPropertyNames() const noexcept;

    KRYS_NODISCARD StorageArea &Area() const noexcept
    {
      return *_storageArea;
    }

  private:
    KRYS_NODISCARD LocalFrame *Frame() const noexcept;

    /// @brief Queues `storage` at the other windows of the window's origin that use the area.
    /// @see https://html.spec.whatwg.org/#concept-storage-broadcast
    void Broadcast(const Maybe<dom::DOMString> &key, const Maybe<dom::DOMString> &oldValue,
                   const Maybe<dom::DOMString> &newValue) noexcept;
  };
}
