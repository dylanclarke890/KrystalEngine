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
