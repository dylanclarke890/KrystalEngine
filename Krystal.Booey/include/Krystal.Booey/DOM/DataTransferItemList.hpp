#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class File;
}

namespace krys::boo::dom
{
  class DataTransfer;
  class DataTransferItem;
  class Document;

  /// @brief A data transfer's items, made from its types and files when first asked for.
  /// @note The data transfer owns its item list.
  /// @see https://html.spec.whatwg.org/multipage/dnd.html#the-datatransferitemlist-interface
  class DataTransferItemList final : public CanMakeWeakPtr<DataTransferItemList>
  {
    KRYS_NON_COPY_MOVABLE(DataTransferItemList);

  private:
    WeakPtr<Document> _document;
    DataTransfer &_dataTransfer;
    mutable Maybe<List<Ref<DataTransferItem>>> _items;

  public:
    DataTransferItemList(Document &document, DataTransfer &dataTransfer) noexcept;
    ~DataTransferItemList() noexcept;

    // DataTransfer owns DataTransferItemList, and DataTransfer is kept alive as long as DataTransferItemList
    // is alive.
    void AddRef() const noexcept;
    void SubRef() const noexcept;
    KRYS_NODISCARD uint32 GetRefCount() const noexcept;

    KRYS_NODISCARD DataTransfer &GetDataTransfer() const noexcept
    {
      return _dataTransfer;
    }

    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransferitemlist-length
    KRYS_NODISCARD uint32 Length() const noexcept;
    KRYS_NODISCARD RefPtr<DataTransferItem> Item(uint32 index) noexcept;
    KRYS_NODISCARD bool IsSupportedPropertyIndex(uint32 index) noexcept;
    /// @brief Adds the string @p data of @p type.
    /// @return The new item, null if the data transfer can't be written; `NotSupportedError` if it has a
    /// string of that type.
    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransferitemlist-add
    KRYS_NODISCARD ExceptionOr<RefPtr<DataTransferItem>> Add(Document &document, DOMStringView data,
                                                             DOMStringView type) noexcept;
    /// @brief Adds @p file.
    /// @return The new item, or null if the data transfer can't be written.
    RefPtr<DataTransferItem> Add(Ref<File> &&file) noexcept;
    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransferitemlist-remove
    ExceptionOr<void> Remove(uint32 index) noexcept;
    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransferitemlist-clear
    void Clear() noexcept;

    /// @brief Removes the string item of @p type, or all the string items if none.
    void DidClearStringData(const Maybe<DOMString> &type) noexcept;
    void DidSetStringData(DOMStringView type) noexcept;
    KRYS_NODISCARD bool HasItems() const noexcept
    {
      return _items.has_value();
    }
    KRYS_NODISCARD const List<Ref<DataTransferItem>> &Items() const noexcept
    {
      krys_debug_assert(_items);
      return *_items;
    }

  private:
    List<Ref<DataTransferItem>> &EnsureItems() const noexcept;
  };
}
