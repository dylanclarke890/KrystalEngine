#include "Krystal.Booey/DOM/DataTransferItem.hpp"
#include "Krystal.Booey/DOM/DataTransfer.hpp"
#include "Krystal.Booey/DOM/DataTransferItemList.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/FileAPI/File.hpp"
#include "Krystal.Booey/HTML/Enums/TaskSource.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"

namespace krys::boo::dom
{
  Ref<DataTransferItem> DataTransferItem::Create(WeakPtr<DataTransferItemList> &&list, DOMStringView type,
                                                 Kind kind) noexcept
  {
    return AdoptRef(*new DataTransferItem(krys::move(list), type, kind));
  }

  Ref<DataTransferItem> DataTransferItem::Create(WeakPtr<DataTransferItemList> &&list, DOMStringView type,
                                                 Ref<File> &&file) noexcept
  {
    return AdoptRef(*new DataTransferItem(krys::move(list), type, krys::move(file)));
  }

  DataTransferItem::DataTransferItem(WeakPtr<DataTransferItemList> &&list, DOMStringView type,
                                     Kind kind) noexcept
      : _list(krys::move(list)), _type(type), _kind(kind)
  {
  }

  DataTransferItem::DataTransferItem(WeakPtr<DataTransferItemList> &&list, DOMStringView type,
                                     Ref<File> &&file) noexcept
      : _list(krys::move(list)), _type(type), _kind(Kind::File), _file(krys::move(file))
  {
  }

  DataTransferItem::~DataTransferItem() noexcept = default;

  void DataTransferItem::ClearListAndPutIntoDisabledMode() noexcept
  {
    _list = nullptr;
  }

  bool DataTransferItem::IsFile() const noexcept
  {
    return _kind == Kind::File;
  }

  // https://html.spec.whatwg.org/#dom-datatransferitem-kind
  DOMString DataTransferItem::GetKind() const noexcept
  {
    // The kind attribute must return the empty string if the DataTransferItem object is in the disabled mode;
    // otherwise it must return the string given in the cell from the second column of the following table
    // from the row whose cell in the first column contains the drag data item kind of the item represented by
    // the DataTransferItem object.
    if (IsInDisabledMode())
    {
      return {};
    }
    switch (_kind)
    {
      case Kind::String: return u8"string";
      case Kind::File:   return u8"file";
    }
    krys_debug_assert(false);
    return {};
  }

  // https://html.spec.whatwg.org/#dom-datatransferitem-type
  // The type attribute must return the empty string if the DataTransferItem object is in the disabled mode;
  // otherwise it must return the drag data item type string of the item represented by the DataTransferItem
  // object.
  DOMString DataTransferItem::Type() const noexcept
  {
    return IsInDisabledMode() ? DOMString() : _type;
  }

  // https://html.spec.whatwg.org/#dom-datatransferitem-getasstring
  void DataTransferItem::GetAsString(Document &document,
                                     Func<void(const DOMString &)> &&callback) const noexcept
  {
    // 1. If the callback is null, return.
    // 3. If the drag data item kind is not text, then return. The callback is never invoked.
    if (!callback || !_list || _file)
    {
      return;
    }

    // 2. If the DataTransferItem object is not in the read/write mode or the read-only mode, return. The
    //    callback is never invoked.
    Ref<DataTransfer> dataTransfer = ShareRef(_list->GetDataTransfer());
    if (!dataTransfer->CanReadData())
    {
      return;
    }

    // 4. Otherwise, queue a task to invoke callback, passing the actual data of the item represented by the
    //    DataTransferItem object as the argument.
    html::EventLoop::Current().QueueTask(
      html::TaskSource::DOMManipulation, document,
      [callback = krys::move(callback), string = dataTransfer->GetDataForItem(document, _type)]
      { callback(string); });
  }

  // https://html.spec.whatwg.org/#dom-datatransferitem-getasfile
  RefPtr<File> DataTransferItem::GetAsFile() const noexcept
  {
    // 1. If the DataTransferItem object is not in the read/write mode or the read-only mode, then return
    // null.
    if (!_list || !_list->GetDataTransfer().CanReadData())
    {
      return nullptr;
    }
    // 2. If the drag data item kind is not File, then return null.
    // 3. Return a new File object representing the actual data of the item represented by the
    //    DataTransferItem object.
    // (The same File each time.)
    return _file;
  }
}
