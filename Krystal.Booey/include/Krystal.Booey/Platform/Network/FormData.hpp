#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::html
{
  class DOMFormData;
}

namespace krys::boo
{
  class File;

  /// @brief A part of a request's body: bytes, or a range of a file.
  struct FormDataElement
  {
    struct EncodedFileData
    {
      utf8_string Filename;
      int64 FileStart {0};
      /// @brief The range's length, or `BlobDataItem::ToEndOfFile`.
      int64 FileLength {0};

      KRYS_NODISCARD bool operator==(const EncodedFileData &) const noexcept = default;
    };

    using DataType = Variant<List<uint8>, EncodedFileData>;

    DataType Data;

    KRYS_NODISCARD bool operator==(const FormDataElement &) const noexcept = default;
  };

  /// @brief The body of a request: the encoded form data a form submits.
  class FormData : public RefCounted<FormData>
  {
  public:
    enum class EncodingType : uint8
    {
      FormURLEncoded,    // for application/x-www-form-urlencoded
      TextPlain,         // for text/plain
      MultipartFormData, // for multipart/form-data
    };

  private:
    List<FormDataElement> _elements;
    int64 _identifier {0};
    List<uint8> _boundary;

  public:
    FormData() noexcept = default;

    KRYS_NODISCARD static Ref<FormData> Create() noexcept;
    KRYS_NODISCARD static Ref<FormData> Create(Span<const uint8> data) noexcept;
    KRYS_NODISCARD static Ref<FormData>
      Create(const html::DOMFormData &formData,
             EncodingType encodingType = EncodingType::FormURLEncoded) noexcept;
    KRYS_NODISCARD static Ref<FormData> CreateMultiPart(const html::DOMFormData &formData) noexcept;

    void AppendData(Span<const uint8> data) noexcept;
    void AppendFile(utf8_stringview filename) noexcept;
    void AppendFileRange(utf8_stringview filename, int64 start, int64 length) noexcept;

    KRYS_NODISCARD const List<FormDataElement> &Elements() const noexcept
    {
      return _elements;
    }

    /// @brief The bytes of the body, without its files.
    KRYS_NODISCARD List<uint8> Flatten() const noexcept;

    /// @brief The bytes of the body (without its files), each taken as a Latin-1 character.
    KRYS_NODISCARD utf8_string FlattenToString() const noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _elements.empty();
    }

    KRYS_NODISCARD Span<const uint8> Boundary() const noexcept
    {
      return _boundary;
    }

    KRYS_NODISCARD int64 Identifier() const noexcept
    {
      return _identifier;
    }

    void SetIdentifier(int64 identifier) noexcept
    {
      _identifier = identifier;
    }

    KRYS_NODISCARD static EncodingType ParseEncodingType(utf8_stringview type) noexcept
    {
      if (text::IsASCIICaselessEqual(type, utf8_stringview(u8"text/plain")))
      {
        return EncodingType::TextPlain;
      }
      if (text::IsASCIICaselessEqual(type, utf8_stringview(u8"multipart/form-data")))
      {
        return EncodingType::MultipartFormData;
      }
      return EncodingType::FormURLEncoded;
    }

  private:
    void AppendMultiPartFileValue(const File &file, List<uint8> &header) noexcept;
    void AppendMultiPartStringValue(utf8_stringview string, List<uint8> &header) noexcept;
    void AppendMultiPartKeyValuePairItems(const html::DOMFormData &formData) noexcept;
    void AppendNonMultiPartKeyValuePairItems(const html::DOMFormData &formData,
                                             EncodingType encodingType) noexcept;
  };
}
