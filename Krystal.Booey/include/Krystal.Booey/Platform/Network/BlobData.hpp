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
