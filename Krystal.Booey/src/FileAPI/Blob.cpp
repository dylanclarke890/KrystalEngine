#include "Krystal.Booey/FileAPI/Blob.hpp"
#include "Krystal.Core/Detection/OS.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <filesystem>
#include <fstream>
#include <system_error>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD List<uint8> NormalizeLineEndingsToLF(List<uint8> &&vector) noexcept
    {
      size_t inputIndex = 0;
      size_t outputIndex = 0;
      while (inputIndex < vector.size())
      {
        auto character = vector[inputIndex++];
        if (character == '\r')
        {
          // Turn CRLF and CR into LF.
          if (inputIndex < vector.size() && vector[inputIndex] == '\n')
          {
            ++inputIndex;
          }
          vector[outputIndex++] = '\n';
        }
        else
        {
          // Leave other characters alone.
          vector[outputIndex++] = character;
        }
      }
      vector.resize(outputIndex);
      return krys::move(vector);
    }

    KRYS_NODISCARD List<uint8> NormalizeLineEndingsToCRLF(List<uint8> &&source) noexcept
    {
      List<uint8> result;
      result.reserve(source.size());
      size_t sourceIndex = 0;
      while (sourceIndex < source.size())
      {
        auto character = source[sourceIndex++];
        if (character == '\r')
        {
          // Turn CR or CRLF into CRLF;
          if (sourceIndex < source.size() && source[sourceIndex] == '\n')
          {
            ++sourceIndex;
          }
          result.push_back('\r');
          result.push_back('\n');
        }
        else if (character == '\n')
        {
          // Turn LF into CRLF.
          result.push_back('\r');
          result.push_back('\n');
        }
        else
        {
          // Leave other characters alone.
          result.push_back(character);
        }
      }
      return result;
    }

    KRYS_NODISCARD List<uint8> NormalizeLineEndingsToNative(List<uint8> &&from) noexcept
    {
#if KRYS_OS(WINDOWS)
      return NormalizeLineEndingsToCRLF(krys::move(from));
#else
      return NormalizeLineEndingsToLF(krys::move(from));
#endif
    }

    /// @brief Builds a blob's data from its parts, merging neighbouring bytes and strings into one item.
    class BlobBuilder
    {
    private:
      EndingType _endings;
      Ref<BlobData> _data;
      List<uint8> _appendableData;

    public:
      BlobBuilder(EndingType endings, utf8_stringview contentType) noexcept
          : _endings(endings), _data(BlobData::Create(contentType))
      {
      }

      void Append(List<uint8> &&bytes) noexcept
      {
        _appendableData.insert(_appendableData.end(), bytes.begin(), bytes.end());
      }

      void Append(RefPtr<Blob> &&blob) noexcept
      {
        if (!blob)
        {
          return;
        }
        Flush();
        _data->AppendItems(blob->Data().Items());
      }

      void Append(const utf8_string &text) noexcept
      {
        List<uint8> bytes(text.begin(), text.end());
        if (_endings == EndingType::Native)
        {
          bytes = NormalizeLineEndingsToNative(krys::move(bytes));
        }
        Append(krys::move(bytes));
      }

      void Append(Ref<SharedBuffer> &&buffer) noexcept
      {
        Flush();
        _data->AppendData(krys::move(buffer));
      }

      KRYS_NODISCARD Ref<BlobData> Finalize() noexcept
      {
        Flush();
        return _data;
      }

    private:
      void Flush() noexcept
      {
        if (!_appendableData.empty())
        {
          _data->AppendData(SharedBuffer::Create(std::exchange(_appendableData, {})));
        }
      }
    };

    KRYS_NODISCARD size_t ComputeMemoryCost(const List<BlobPartVariant> &blobPartVariants) noexcept
    {
      size_t memoryCost = 0;
      for (auto &blobPartVariant : blobPartVariants)
      {
        std::visit(Visitor {[&](const RefPtr<Blob> &blob) { memoryCost += blob ? blob->MemoryCost() : 0; },
                            [&](const List<uint8> &bytes) { memoryCost += bytes.size(); },
                            [&](const utf8_string &string) { memoryCost += string.size(); }},
                   blobPartVariant);
      }
      return memoryCost;
    }

    KRYS_NODISCARD Ref<BlobData> BuildBlobData(List<BlobPartVariant> &&blobPartVariants, EndingType endings,
                                               utf8_stringview contentType) noexcept
    {
      BlobBuilder builder(endings, contentType);
      for (auto &blobPartVariant : blobPartVariants)
      {
        std::visit([&](auto &part) { builder.Append(krys::move(part)); }, blobPartVariant);
      }
      return builder.Finalize();
    }

    /// @brief Appends @p length bytes of the file at @p path, from @p offset (to its end for
    /// `BlobDataItem::ToEndOfFile`), to @p bytes.
    KRYS_NODISCARD Maybe<dom::Exception> ReadFileRange(utf8_stringview path, int64 offset, int64 length,
                                                       List<uint8> &bytes) noexcept
    {
      std::filesystem::path filePath(path);
      std::error_code error;
      if (!std::filesystem::is_regular_file(filePath, error))
      {
        return dom::Exception {dom::ExceptionCode::NotFoundError};
      }

      std::ifstream file(filePath, std::ios::binary);
      if (!file)
      {
        return dom::Exception {dom::ExceptionCode::NotReadableError};
      }
      if (length == BlobDataItem::ToEndOfFile)
      {
        auto size = static_cast<int64>(std::filesystem::file_size(filePath, error));
        if (error)
        {
          return dom::Exception {dom::ExceptionCode::NotReadableError};
        }
        length = size > offset ? size - offset : 0;
      }

      auto start = bytes.size();
      bytes.resize(start + static_cast<size_t>(length));
      file.seekg(offset);
      file.read(reinterpret_cast<char *>(bytes.data() + start), static_cast<std::streamsize>(length));
      if (file.gcount() != static_cast<std::streamsize>(length))
      {
        return dom::Exception {dom::ExceptionCode::NotReadableError};
      }
      return std::nullopt;
    }
  }

  Blob::Blob(Ref<BlobData> &&data, utf8_stringview type, Maybe<uint64> size, size_t memoryCost) noexcept
      : _type(type), _size(size), _memoryCost(memoryCost), _data(krys::move(data))
  {
  }

  Blob::Blob(ReferencingExistingBlobConstructor, const Blob &blob) noexcept
      : _type(blob.Type()), _size(blob.Size()), _memoryCost(blob.MemoryCost()), _data(blob._data)
  {
  }

  Blob::Blob(List<BlobPartVariant> &&blobPartVariants, const BlobPropertyBag &propertyBag) noexcept
      : _type(NormalizedContentType(propertyBag.Type)), _memoryCost(ComputeMemoryCost(blobPartVariants)),
        _data(BuildBlobData(krys::move(blobPartVariants), propertyBag.Endings, _type))
  {
  }

  Ref<Blob> Blob::Create() noexcept
  {
    return AdoptRef(*new Blob(BlobData::Create({}), {}, 0, 0));
  }

  Ref<Blob> Blob::Create(List<BlobPartVariant> &&blobPartVariants,
                         const BlobPropertyBag &propertyBag) noexcept
  {
    return AdoptRef(*new Blob(krys::move(blobPartVariants), propertyBag));
  }

  Ref<Blob> Blob::Create(List<uint8> &&data, utf8_stringview contentType) noexcept
  {
    return Create(SharedBuffer::Create(krys::move(data)), contentType);
  }

  Ref<Blob> Blob::Create(Ref<SharedBuffer> &&buffer, utf8_stringview contentType) noexcept
  {
    auto size = buffer->Size();
    BlobBuilder builder(EndingType::Transparent, contentType);
    builder.Append(krys::move(buffer));
    return AdoptRef(*new Blob(builder.Finalize(), contentType, size, size));
  }

  uint64 Blob::Size() const noexcept
  {
    if (!_size)
    {
      _size = _data->Size();
    }
    return *_size;
  }

  bool Blob::IsValidContentType(utf8_stringview contentType) noexcept
  {
    // FIXME: Do we really want to treat the empty string and null string as valid content types?
    for (auto character : contentType)
    {
      if (character < 0x20 || character > 0x7e)
      {
        return false;
      }
    }
    return true;
  }

  utf8_string Blob::NormalizedContentType(utf8_stringview contentType) noexcept
  {
    if (!IsValidContentType(contentType))
    {
      return {};
    }
    return text::ToASCIILower(contentType);
  }

  Ref<Blob> Blob::Slice(int64 start, int64 end, utf8_stringview contentType) const noexcept
  {
    uint64 sliceMemoryCost = 0;
    if (auto totalMemoryCost = static_cast<uint64>(MemoryCost()))
    {
      uint64 positiveStart =
        start > 0 ? std::min<uint64>(static_cast<uint64>(start), totalMemoryCost)
                  : totalMemoryCost - std::min<uint64>(static_cast<uint64>(-start), totalMemoryCost);
      uint64 positiveEnd = end > 0
                             ? std::min<uint64>(static_cast<uint64>(end), totalMemoryCost)
                             : totalMemoryCost - std::min<uint64>(static_cast<uint64>(-end), totalMemoryCost);
      if (positiveStart < positiveEnd)
      {
        sliceMemoryCost = positiveEnd - positiveStart;
      }
      krys_debug_assert(sliceMemoryCost <= totalMemoryCost);
    }
    auto type = NormalizedContentType(contentType);
    // The size isn't necessarily end - start, so it's computed when asked for.
    return AdoptRef(*new Blob(_data->CreateSlice(start, end, type), type, std::nullopt,
                              static_cast<size_t>(sliceMemoryCost)));
  }

  dom::ExceptionOr<List<uint8>> Blob::Bytes() const noexcept
  {
    List<uint8> bytes;
    for (auto &item : _data->Items())
    {
      if (item.GetType() == BlobDataItem::Type::Data)
      {
        auto span = item.Data()->GetSpan().subspan(static_cast<size_t>(item.Offset()),
                                                   static_cast<size_t>(item.Length()));
        bytes.insert(bytes.end(), span.begin(), span.end());
      }
      else if (auto exception = ReadFileRange(item.Path(), item.Offset(), item.Length(), bytes))
      {
        return krys::move(*exception);
      }
    }
    return bytes;
  }

  dom::ExceptionOr<utf8_string> Blob::Text() const noexcept
  {
    auto result = Bytes();
    if (result.HasException())
    {
      return result.ReleaseException();
    }
    auto bytes = result.ReleaseValue();

    // UTF-8 decode: a byte order mark is dropped, and invalid sequences become replacement characters.
    Span<const char8> data(reinterpret_cast<const char8 *>(bytes.data()), bytes.size());
    if (data.size() >= 3 && data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF)
    {
      data = data.subspan(3);
    }
    StringBuilder builder;
    while (!data.empty())
    {
      auto decoded = text::DecodeUTF8(data);
      builder.Append(decoded.CodePoint);
      data = data.subspan(decoded.Length);
    }
    return krys::move(builder).ToString();
  }
}
