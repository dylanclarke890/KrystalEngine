#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class File;
}

namespace krys::boo::dom
{
  class DataTransferItemList;
  class Document;

  /// @brief One of a data transfer's items: a string of a type, or a file.
  /// @see https://html.spec.whatwg.org/multipage/dnd.html#the-datatransferitem-interface
  // FIXME: Entries (getAsEntry), with the file system API.
  class DataTransferItem final : public RefCounted<DataTransferItem>
  {
  public:
    enum class Kind : uint8
    {
      String,
      File
    };

  private:
    WeakPtr<DataTransferItemList> _list;
    const DOMString _type;
    Kind _kind {Kind::String};
    RefPtr<File> _file;

    DataTransferItem(WeakPtr<DataTransferItemList> &&list, DOMStringView type, Kind kind) noexcept;
    DataTransferItem(WeakPtr<DataTransferItemList> &&list, DOMStringView type, Ref<File> &&file) noexcept;

  public:
    ~DataTransferItem() noexcept;

    KRYS_NODISCARD static Ref<DataTransferItem> Create(WeakPtr<DataTransferItemList> &&list,
                                                       DOMStringView type, Kind kind) noexcept;
    KRYS_NODISCARD static Ref<DataTransferItem> Create(WeakPtr<DataTransferItemList> &&list,
                                                       DOMStringView type, Ref<File> &&file) noexcept;

    KRYS_NODISCARD RefPtr<File> GetFile() const noexcept
    {
      return _file;
    }
    /// @brief Detaches the item from its list, after which it has no type and gives no data.
    void ClearListAndPutIntoDisabledMode() noexcept;

    KRYS_NODISCARD bool IsFile() const noexcept;
    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransferitem-kind
    KRYS_NODISCARD DOMString GetKind() const noexcept;
    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransferitem-type
    KRYS_NODISCARD DOMString Type() const noexcept;
    /// @brief Queues a task that calls @p callback with the item's string, if it's a string the data
    /// transfer can read.
    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransferitem-getasstring
    void GetAsString(Document &document, Func<void(const DOMString &)> &&callback) const noexcept;
    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransferitem-getasfile
    KRYS_NODISCARD RefPtr<File> GetAsFile() const noexcept;

  private:
    KRYS_NODISCARD bool IsInDisabledMode() const noexcept
    {
      return !_list;
    }
  };
}
