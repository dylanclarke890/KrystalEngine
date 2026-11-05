#include "Krystal.Booey/DOM/DataTransfer/DataTransferItemList.hpp"
#include "Krystal.Booey/DOM/DataTransfer/DataTransfer.hpp"
#include "Krystal.Booey/DOM/DataTransfer/DataTransferItem.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/FileAPI/FileList.hpp"
#include "Krystal.Booey/Platform/Pasteboard/Pasteboard.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>

namespace krys::boo::dom
{
  namespace
  {
    void RemoveStringItemOfLowercasedType(List<Ref<DataTransferItem>> &items,
                                          DOMStringView lowercasedType) noexcept
    {
      auto it = std::ranges::find_if(items, [&](auto &item)
                                     { return !item->IsFile() && item->Type() == lowercasedType; });
      if (it == items.end())
      {
        return;
      }
      (*it)->ClearListAndPutIntoDisabledMode();
      items.erase(it);
    }
  }

  DataTransferItemList::DataTransferItemList(Document &document, DataTransfer &dataTransfer) noexcept
      : _document(CreateWeakPtr(&document)), _dataTransfer(dataTransfer)
  {
  }

  DataTransferItemList::~DataTransferItemList() noexcept = default;

  void DataTransferItemList::AddRef() const noexcept
  {
    _dataTransfer.AddRef();
  }

  void DataTransferItemList::SubRef() const noexcept
  {
    _dataTransfer.SubRef();
  }

  uint32 DataTransferItemList::GetRefCount() const noexcept
  {
    return _dataTransfer.GetRefCount();
  }

  uint32 DataTransferItemList::Length() const noexcept
  {
    return static_cast<uint32>(EnsureItems().size());
  }

  bool DataTransferItemList::IsSupportedPropertyIndex(uint32 index) noexcept
  {
    return index < EnsureItems().size();
  }

  RefPtr<DataTransferItem> DataTransferItemList::Item(uint32 index) noexcept
  {
    auto &items = EnsureItems();
    if (items.size() <= index)
    {
      return nullptr;
    }
    return items[index];
  }

  // https://html.spec.whatwg.org/#dom-datatransferitemlist-add
  ExceptionOr<RefPtr<DataTransferItem>> DataTransferItemList::Add(Document &document, DOMStringView data,
                                                                  DOMStringView type) noexcept
  {
    // 1. If the DataTransferItemList object is not in the read/write mode, return null.
    if (!_dataTransfer.CanWriteData())
    {
      return RefPtr<DataTransferItem>();
    }

    // 2. If the first argument to the method is a string: If there is already an item in the drag data store
    //    item list whose kind is text and whose type string is equal to the value of the method's second
    //    argument, converted to ASCII lowercase, then throw a "NotSupportedError" DOMException. Otherwise,
    //    add an item to the drag data store item list whose kind is text, whose type string is equal to the
    //    value of the method's second argument, converted to ASCII lowercase, and whose data is the string
    //    given by the method's first argument.
    // 3. Determine the value of the indexed property corresponding to the newly added item, and return that
    //    value (a newly created DataTransferItem object).
    for (auto &item : EnsureItems())
    {
      if (!item->IsFile() && text::IsASCIICaselessEqual(item->Type(), type))
      {
        return Exception {ExceptionCode::NotSupportedError};
      }
    }

    DOMString lowercasedType = text::ToASCIILower(type);

    _dataTransfer.SetDataFromItemList(document, lowercasedType, data);
    krys_debug_assert(_items);
    _items->push_back(
      DataTransferItem::Create(CreateWeakPtr(this), lowercasedType, DataTransferItem::Kind::String));
    return RefPtr<DataTransferItem>(_items->back());
  }

  // https://html.spec.whatwg.org/#dom-datatransferitemlist-add
  RefPtr<DataTransferItem> DataTransferItemList::Add(Ref<File> &&file) noexcept
  {
    // 1. If the DataTransferItemList object is not in the read/write mode, return null.
    // 2. If the first argument to the method is a File: Add an item to the drag data store item list whose
    // kind
    //    is File, whose type string is the type of the File, converted to ASCII lowercase, and whose data is
    //    the same as the File's data.
    // (A File's type is already lowercase.)
    // 3. Determine the value of the indexed property corresponding to the newly added item, and return that
    //    value (a newly created DataTransferItem object).
    if (!_dataTransfer.CanWriteData())
    {
      return nullptr;
    }

    auto type = DOMString(file->Type());
    EnsureItems().push_back(DataTransferItem::Create(CreateWeakPtr(this), type, krys::move(file)));
    _dataTransfer.DidAddFileToItemList();
    return _items->back();
  }

  // https://html.spec.whatwg.org/#dom-datatransferitemlist-remove
  ExceptionOr<void> DataTransferItemList::Remove(uint32 index) noexcept
  {
    // 1. If the DataTransferItemList object is not in the read/write mode, throw an "InvalidStateError"
    //    DOMException.
    if (!_dataTransfer.CanWriteData())
    {
      return Exception {ExceptionCode::InvalidStateError};
    }

    // 2. If the drag data store does not contain an indexth item, then return.
    auto &items = EnsureItems();
    if (items.size() <= index)
    {
      return {};
    }

    // 3. Remove the indexth item from the drag data store.

    // FIXME: Remove the file from the pasteboard object once we add support for it.
    Ref<DataTransferItem> removedItem = items[index];
    if (!removedItem->IsFile())
    {
      _dataTransfer.GetPasteboard().Clear(removedItem->Type());
    }
    removedItem->ClearListAndPutIntoDisabledMode();
    items.erase(items.begin() + index);
    if (removedItem->IsFile())
    {
      _dataTransfer.UpdateFileList(_document.get());
    }

    return {};
  }

  // https://html.spec.whatwg.org/#dom-datatransferitemlist-clear
  void DataTransferItemList::Clear() noexcept
  {
    // The clear() method, if the DataTransferItemList object is in the read/write mode, must remove all the
    // items from the drag data store. Otherwise, it must do nothing.
    if (!_dataTransfer.CanWriteData())
    {
      return;
    }

    _dataTransfer.GetPasteboard().Clear();
    bool removedItemContainingFile = false;
    if (_items)
    {
      for (auto &item : *_items)
      {
        removedItemContainingFile |= item->IsFile();
        item->ClearListAndPutIntoDisabledMode();
      }
      _items->clear();
    }

    if (removedItemContainingFile)
    {
      _dataTransfer.UpdateFileList(_document.get());
    }
  }

  List<Ref<DataTransferItem>> &DataTransferItemList::EnsureItems() const noexcept
  {
    if (_items)
    {
      return *_items;
    }

    auto *self = const_cast<DataTransferItemList *>(this);
    List<Ref<DataTransferItem>> items;
    if (RefPtr<Document> document = ShareRefPtr(_document.get()))
    {
      for (auto &type : _dataTransfer.TypesForItemList(*document))
      {
        items.push_back(DataTransferItem::Create(CreateWeakPtr(self), text::ToASCIILower(type),
                                                 DataTransferItem::Kind::String));
      }

      if (_dataTransfer.CanReadData())
      {
        for (auto &file : _dataTransfer.Files(*document).Files())
        {
          items.push_back(DataTransferItem::Create(CreateWeakPtr(self), file->Type(), Ref<File>(file)));
        }
      }
    }

    _items = krys::move(items);

    return *_items;
  }

  void DataTransferItemList::DidClearStringData(const Maybe<DOMString> &type) noexcept
  {
    if (!_items)
    {
      return;
    }

    auto &items = *_items;
    if (type)
    {
      return RemoveStringItemOfLowercasedType(items, text::ToASCIILower(*type));
    }

    for (auto &item : items)
    {
      if (!item->IsFile())
      {
        item->ClearListAndPutIntoDisabledMode();
      }
    }
    std::erase_if(items, [](auto &item) { return !item->IsFile(); });
  }

  // https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransfer-setdata
  void DataTransferItemList::DidSetStringData(DOMStringView type) noexcept
  {
    if (!_items)
    {
      return;
    }

    DOMString lowercasedType = text::ToASCIILower(type);
    RemoveStringItemOfLowercasedType(*_items, lowercasedType);

    _items->push_back(
      DataTransferItem::Create(CreateWeakPtr(this), lowercasedType, DataTransferItem::Kind::String));
  }
}
