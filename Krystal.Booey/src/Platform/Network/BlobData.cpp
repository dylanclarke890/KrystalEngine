/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Platform/Network/BlobData.hpp"
#include <filesystem>
#include <system_error>

namespace krys::boo
{
  int64 BlobDataItem::Length() const noexcept
  {
    if (_length != ToEndOfFile)
    {
      return _length;
    }

    krys_debug_assert(_type == Type::File);
    std::error_code error;
    auto size = std::filesystem::file_size(std::filesystem::path(_path), error);
    if (error)
    {
      return 0;
    }
    auto length = static_cast<int64>(size) - _offset;
    return length > 0 ? length : 0;
  }

  void BlobData::AppendData(Ref<SharedBuffer> &&data) noexcept
  {
    auto length = static_cast<int64>(data->Size());
    AppendData(krys::move(data), 0, length);
  }

  void BlobData::AppendData(Ref<SharedBuffer> &&data, int64 offset, int64 length) noexcept
  {
    _items.push_back(BlobDataItem(krys::move(data), offset, length));
  }

  void BlobData::AppendFile(utf8_stringview path) noexcept
  {
    AppendFile(path, 0, BlobDataItem::ToEndOfFile);
  }

  void BlobData::AppendFile(utf8_stringview path, int64 offset, int64 length) noexcept
  {
    _items.push_back(BlobDataItem(path, offset, length));
  }

  void BlobData::AppendItems(const BlobDataItemList &items) noexcept
  {
    _items.insert(_items.end(), items.begin(), items.end());
  }

  void BlobData::AppendStorageItems(const BlobDataItemList &items, int64 offset, int64 length) noexcept
  {
    krys_debug_assert(length != BlobDataItem::ToEndOfFile);

    size_t itemsIndex = 0;
    if (offset)
    {
      for (; itemsIndex < items.size(); ++itemsIndex)
      {
        auto &item = items[itemsIndex];
        if (offset >= item.Length())
        {
          offset -= item.Length();
        }
        else
        {
          break;
        }
      }
    }

    for (; itemsIndex < items.size() && length > 0; ++itemsIndex)
    {
      auto &item = items[itemsIndex];
      int64 currentLength = item.Length() - offset;
      int64 newLength = currentLength > length ? length : currentLength;
      if (item.GetType() == BlobDataItem::Type::Data)
      {
        AppendData(ShareRef(*item.Data()), item.Offset() + offset, newLength);
      }
      else
      {
        AppendFile(item.Path(), item.Offset() + offset, newLength);
      }
      length -= newLength;
      offset = 0;
    }
    krys_debug_assert(!length);
  }

  uint64 BlobData::Size() const noexcept
  {
    uint64 result = 0;
    for (auto &item : _items)
    {
      result += static_cast<uint64>(item.Length());
    }
    return result;
  }

  Ref<BlobData> BlobData::CreateSlice(int64 start, int64 end, utf8_stringview contentType) const noexcept
  {
    auto originalSize = Size();

    // Convert the negative value that is used to select from the end.
    if (start < 0)
    {
      start = start + static_cast<int64>(originalSize);
    }
    if (end < 0)
    {
      end = end + static_cast<int64>(originalSize);
    }

    // Clamp the range if it exceeds the size limit.
    if (start < 0)
    {
      start = 0;
    }
    if (end < 0)
    {
      end = 0;
    }
    if (static_cast<uint64>(start) >= originalSize)
    {
      start = 0;
      end = 0;
    }
    else if (end < start)
    {
      end = start;
    }
    else if (static_cast<uint64>(end) > originalSize)
    {
      end = static_cast<int64>(originalSize);
    }

    auto newLength = end - start;
    auto newData = Create(contentType);
    newData->AppendStorageItems(_items, start, newLength);
    return newData;
  }
}
