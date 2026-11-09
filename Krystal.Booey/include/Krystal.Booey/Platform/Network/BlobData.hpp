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

#pragma once

#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief A range of a blob's bytes: in memory, or in a file.
  class BlobDataItem
  {
    friend class BlobData;

  public:
    static constexpr int64 ToEndOfFile {-1};

    enum class Type : bool
    {
      Data,
      File
    };

  private:
    Type _type;
    RefPtr<SharedBuffer> _data;
    utf8_string _path;
    int64 _offset {0};
    int64 _length {0};

    BlobDataItem(Ref<SharedBuffer> &&data, int64 offset, int64 length) noexcept
        : _type(Type::Data), _data(krys::move(data)), _offset(offset), _length(length)
    {
    }

    BlobDataItem(utf8_stringview path, int64 offset, int64 length) noexcept
        : _type(Type::File), _path(path), _offset(offset), _length(length)
    {
    }

  public:
    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    /// @brief For data items, the bytes the range is in.
    KRYS_NODISCARD SharedBuffer *Data() const noexcept
    {
      return _data.get();
    }

    /// @brief For file items, the file's path.
    KRYS_NODISCARD utf8_stringview Path() const noexcept
    {
      return _path;
    }

    KRYS_NODISCARD int64 Offset() const noexcept
    {
      return _offset;
    }

    /// @brief The range's length; a file item to the end of its file computes it (0 if the file can't be
    /// read).
    KRYS_NODISCARD int64 Length() const noexcept;
  };

  using BlobDataItemList = List<BlobDataItem>;

  /// @brief A blob's bytes, as a list of data and file ranges, and its type.
  class BlobData final : public RefCounted<BlobData>
  {
    KRYS_NON_COPY_MOVABLE(BlobData);

  private:
    utf8_string _contentType;
    BlobDataItemList _items;

  public:
    explicit BlobData(utf8_stringview contentType) noexcept : _contentType(contentType)
    {
    }

    KRYS_NODISCARD static Ref<BlobData> Create(utf8_stringview contentType) noexcept
    {
      return CreateRef<BlobData>(contentType);
    }

    KRYS_NODISCARD utf8_stringview ContentType() const noexcept
    {
      return _contentType;
    }

    KRYS_NODISCARD const BlobDataItemList &Items() const noexcept
    {
      return _items;
    }

    void AppendData(Ref<SharedBuffer> &&data) noexcept;
    void AppendFile(utf8_stringview path) noexcept;
    /// @brief Appends the items of another blob's data.
    void AppendItems(const BlobDataItemList &items) noexcept;
    /// @brief Appends @p length bytes of @p items, from @p offset.
    void AppendStorageItems(const BlobDataItemList &items, int64 offset, int64 length) noexcept;

    /// @brief The total length of the items.
    KRYS_NODISCARD uint64 Size() const noexcept;

    /// @brief The data of a slice of this data, from @p start to @p end (negative values count from the end),
    /// clamped to it.
    KRYS_NODISCARD Ref<BlobData> CreateSlice(int64 start, int64 end,
                                             utf8_stringview contentType) const noexcept;

  private:
    void AppendData(Ref<SharedBuffer> &&data, int64 offset, int64 length) noexcept;
    void AppendFile(utf8_stringview path, int64 offset, int64 length) noexcept;
  };
}
