#include "Krystal.Booey/Platform/Network/FormData.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
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
    _data.insert(_data.end(), data.begin(), data.end());
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

      AppendMultiPartStringValue(item.Data, header);

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
      auto normalizedName = NormalizeStringData(item.Name);
      auto normalizedStringData = NormalizeStringData(item.Data);
      FormDataBuilder::AddKeyValuePairAsFormData(encodedData, normalizedName, normalizedStringData,
                                                 encodingType);
    }

    AppendData(encodedData);
  }

  List<uint8> FormData::Flatten() const noexcept
  {
    return _data;
  }

  utf8_string FormData::FlattenToString() const noexcept
  {
    StringBuilder builder;
    for (auto character : _data)
    {
      builder.Append(static_cast<char32>(character));
    }
    return krys::move(builder).ToString();
  }
}
