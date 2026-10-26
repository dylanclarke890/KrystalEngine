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

  DOMString DataTransferItem::GetKind() const noexcept
  {
    switch (_kind)
    {
      case Kind::String: return u8"string";
      case Kind::File:   return u8"file";
    }
    krys_debug_assert(false);
    return {};
  }

  DOMString DataTransferItem::Type() const noexcept
  {
    return IsInDisabledMode() ? DOMString() : _type;
  }

  void DataTransferItem::GetAsString(Document &document,
                                     Func<void(const DOMString &)> &&callback) const noexcept
  {
    if (!callback || !_list || _file)
    {
      return;
    }

    Ref<DataTransfer> dataTransfer = ShareRef(_list->GetDataTransfer());
    if (!dataTransfer->CanReadData())
    {
      return;
    }

    // FIXME: Make this async.
    html::EventLoop::Current().QueueTask(
      html::TaskSource::DOMManipulation, document,
      [callback = krys::move(callback), string = dataTransfer->GetDataForItem(document, _type)]
      { callback(string); });
  }

  RefPtr<File> DataTransferItem::GetAsFile() const noexcept
  {
    if (!_list || !_list->GetDataTransfer().CanReadData())
    {
      return nullptr;
    }
    return _file;
  }
}
