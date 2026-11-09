/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 * Copyright (C) 2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/DOM/DataTransfer/DataTransferItem.hpp"
#include "Krystal.Booey/DOM/DataTransfer/DataTransfer.hpp"
#include "Krystal.Booey/DOM/DataTransfer/DataTransferItemList.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/FileAPI/File.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/HTML/EventLoop/TaskSource.hpp"

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
