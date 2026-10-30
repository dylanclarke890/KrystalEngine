#include "Krystal.Booey/Platform/Network/FormData.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/Platform/Network/BlobData.hpp"
#include "Krystal.Booey/Platform/Network/FormDataBuilder.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <array>

namespace krys::boo
{
  namespace
  {
    /// @brief @p value encoded (as UTF-8) with its line breaks normalised to CRLF.
    KRYS_NODISCARD List<uint8> NormalizeStringData(utf8_stringview value) noexcept
    {
      List<uint8> result;
      result.reserve(value.size());
      for (size_t i = 0; i < value.size(); ++i)
      {
        auto character = static_cast<uint8>(value[i]);
        if (character == '\r')
        {
          result.push_back('\r');
          result.push_back('\n');
          if (i + 1 < value.size() && value[i + 1] == u8'\n')
          {
            ++i;
          }
        }
        else if (character == '\n')
        {
          result.push_back('\r');
          result.push_back('\n');
        }
        else
        {
          result.push_back(character);
        }
      }
      return result;
    }
  }

  Ref<FormData> FormData::Create() noexcept
  {
    return CreateRef<FormData>();
  }

  Ref<FormData> FormData::Create(Span<const uint8> data) noexcept
  {
    auto result = Create();
    result->AppendData(data);
    return result;
  }

  Ref<FormData> FormData::Create(const html::DOMFormData &formData, EncodingType encodingType) noexcept
  {
    auto result = Create();
    result->AppendNonMultiPartKeyValuePairItems(formData, encodingType);
    return result;
  }

  Ref<FormData> FormData::CreateMultiPart(const html::DOMFormData &formData) noexcept
  {
    auto result = Create();
    result->AppendMultiPartKeyValuePairItems(formData);
    return result;
  }

  void FormData::AppendData(Span<const uint8> data) noexcept
  {
    if (!_elements.empty())
    {
      if (auto *vector = std::get_if<List<uint8>>(&_elements.back().Data))
      {
        vector->insert(vector->end(), data.begin(), data.end());
        return;
      }
    }
    _elements.push_back({.Data = List<uint8>(data.begin(), data.end())});
  }

  void FormData::AppendFile(utf8_stringview filename) noexcept
  {
    AppendFileRange(filename, 0, BlobDataItem::ToEndOfFile);
  }

  void FormData::AppendFileRange(utf8_stringview filename, int64 start, int64 length) noexcept
  {
    _elements.push_back({.Data = FormDataElement::EncodedFileData {
                           .Filename = utf8_string(filename), .FileStart = start, .FileLength = length}});
  }

  void FormData::AppendMultiPartFileValue(const File &file, List<uint8> &header) noexcept
  {
    auto name = file.Name();

    // We have to include the filename=".." part in the header, even if the filename is empty
    FormDataBuilder::AddFilenameToMultiPartHeader(header, name);

    // Add the content type if available, or "application/octet-stream" otherwise (RFC 1867).
    utf8_stringview contentType = file.Type();
    if (contentType.empty())
    {
      contentType = u8"application/octet-stream";
    }

    FormDataBuilder::AddContentTypeToMultiPartHeader(header, contentType);

    FormDataBuilder::FinishMultiPartHeader(header);
    AppendData(header);

    if (!file.Path().empty())
    {
      AppendFile(file.Path());
      return;
    }

    // A blob's bytes, resolved to its data and file ranges.
    for (auto &item : file.Data().Items())
    {
      if (item.GetType() == BlobDataItem::Type::Data)
      {
        AppendData(item.Data()->GetSpan().subspan(static_cast<size_t>(item.Offset()),
                                                  static_cast<size_t>(item.Length())));
      }
      else
      {
        AppendFileRange(item.Path(), item.Offset(), item.Length());
      }
    }
  }

  void FormData::AppendMultiPartStringValue(utf8_stringview string, List<uint8> &header) noexcept
  {
    FormDataBuilder::FinishMultiPartHeader(header);
    AppendData(header);

    auto normalizedStringData = NormalizeStringData(string);
    AppendData(normalizedStringData);
  }

  void FormData::AppendMultiPartKeyValuePairItems(const html::DOMFormData &formData) noexcept
  {
    _boundary = FormDataBuilder::GenerateUniqueBoundaryString();

    List<uint8> encodedData;
    for (auto &item : formData.Items())
    {
      auto normalizedName = NormalizeStringData(item.Name);

      List<uint8> header;
      FormDataBuilder::BeginMultiPartHeader(header, _boundary, normalizedName);

      if (auto *file = std::get_if<RefPtr<File>>(&item.Data))
      {
        AppendMultiPartFileValue(**file, header);
      }
      else
      {
        AppendMultiPartStringValue(std::get<dom::DOMString>(item.Data), header);
      }

      constexpr std::array<uint8, 2> newline {'\r', '\n'};
      AppendData(newline);
    }

    FormDataBuilder::AddBoundaryToMultiPartHeader(encodedData, _boundary, true);

    AppendData(encodedData);
  }

  void FormData::AppendNonMultiPartKeyValuePairItems(const html::DOMFormData &formData,
                                                     EncodingType encodingType) noexcept
  {
    List<uint8> encodedData;
    for (auto &item : formData.Items())
    {
      auto *file = std::get_if<RefPtr<File>>(&item.Data);
      utf8_stringview stringValue = file ? (*file)->Name() : std::get<dom::DOMString>(item.Data);

      auto normalizedName = NormalizeStringData(item.Name);
      auto normalizedStringData = NormalizeStringData(stringValue);
      FormDataBuilder::AddKeyValuePairAsFormData(encodedData, normalizedName, normalizedStringData,
                                                 encodingType);
    }

    AppendData(encodedData);
  }

  List<uint8> FormData::Flatten() const noexcept
  {
    // Concatenate all the byte arrays, but omit any files.
    List<uint8> data;
    for (auto &element : _elements)
    {
      if (auto *vector = std::get_if<List<uint8>>(&element.Data))
      {
        data.insert(data.end(), vector->begin(), vector->end());
      }
    }
    return data;
  }

  utf8_string FormData::FlattenToString() const noexcept
  {
    StringBuilder builder;
    for (auto character : Flatten())
    {
      builder.Append(static_cast<char32>(character));
    }
    return krys::move(builder).ToString();
  }
}
