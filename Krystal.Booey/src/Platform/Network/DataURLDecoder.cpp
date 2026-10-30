#include "Krystal.Booey/Platform/Network/DataURLDecoder.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/Base64.hpp"
#include <algorithm>

namespace krys::boo::data_url_decoder
{
  namespace
  {
    KRYS_NODISCARD bool IsASCIIWhitespaceWithoutFF(char8_t c) noexcept
    {
      return c == u8' ' || c == u8'\t' || c == u8'\n' || c == u8'\r';
    }

    KRYS_NODISCARD utf8_stringview Trim(utf8_stringview input) noexcept
    {
      while (!input.empty() && IsASCIIWhitespaceWithoutFF(input.front()))
      {
        input.remove_prefix(1);
      }
      while (!input.empty() && IsASCIIWhitespaceWithoutFF(input.back()))
      {
        input.remove_suffix(1);
      }
      return input;
    }

    KRYS_NODISCARD utf8_string ToASCIILowercase(utf8_stringview input) noexcept
    {
      utf8_string result(input);
      std::ranges::transform(result, result.begin(), [](char8_t c) { return text::ToASCIILower(c); });
      return result;
    }

    KRYS_NODISCARD int HexValue(char8_t c) noexcept
    {
      if (c >= u8'0' && c <= u8'9')
      {
        return c - u8'0';
      }
      if (c >= u8'a' && c <= u8'f')
      {
        return c - u8'a' + 10;
      }
      if (c >= u8'A' && c <= u8'F')
      {
        return c - u8'A' + 10;
      }
      return -1;
    }

    /// @brief The bytes of @p input with its `%XX` escape sequences decoded.
    KRYS_NODISCARD List<uint8> DecodeURLEscapeSequencesAsData(utf8_stringview input) noexcept
    {
      List<uint8> result;
      result.reserve(input.size());
      for (size_t i = 0; i < input.size(); ++i)
      {
        if (input[i] == u8'%' && i + 2 < input.size())
        {
          const int high = HexValue(input[i + 1]);
          const int low = HexValue(input[i + 2]);
          if (high >= 0 && low >= 0)
          {
            result.push_back(static_cast<uint8>(high * 16 + low));
            i += 2;
            continue;
          }
        }
        result.push_back(static_cast<uint8>(input[i]));
      }
      return result;
    }

    /// @brief The MIME type, charset and serialised content type of @p mediaType, or `text/plain` in US-ASCII
    /// if it isn't valid.
    /// FIXME: Parse the content type fully (ParsedContentType: quoted parameters, token validation).
    KRYS_NODISCARD Result ParseMediaType(utf8_stringview mediaType) noexcept
    {
      auto semicolon = mediaType.find(u8';');
      auto mimeType = ToASCIILowercase(Trim(mediaType.substr(0, semicolon)));
      auto slash = mimeType.find(u8'/');
      if (mimeType.empty() || slash == utf8_string::npos || slash == 0 || slash == mimeType.size() - 1)
      {
        return {u8"text/plain", u8"US-ASCII", u8"text/plain;charset=US-ASCII", {}};
      }

      utf8_string charset;
      while (semicolon != utf8_stringview::npos)
      {
        auto next = mediaType.find(u8';', semicolon + 1);
        auto parameter = Trim(mediaType.substr(
          semicolon + 1, next == utf8_stringview::npos ? utf8_stringview::npos : next - semicolon - 1));
        auto equals = parameter.find(u8'=');
        if (equals != utf8_stringview::npos
            && ToASCIILowercase(Trim(parameter.substr(0, equals))) == u8"charset")
        {
          charset = utf8_string(Trim(parameter.substr(equals + 1)));
        }
        semicolon = next;
      }

      utf8_string contentType = mimeType;
      if (!charset.empty())
      {
        contentType += u8";charset=";
        contentType += charset;
      }
      return {krys::move(mimeType), krys::move(charset), krys::move(contentType), {}};
    }
  }

  Maybe<Result> Decode(const url::URL &url, ShouldValidatePadding shouldValidatePadding) noexcept
  {
    krys_debug_assert(url.ProtocolIsData());

    // Syntax:
    //  url := data:<header>,<encodedData>
    //  header := [<mediatype>][;base64]
    //  mediatype := [<mimetype>][;charset=<charsettype>]

    constexpr utf8_stringview dataString = u8"data:";
    utf8_stringview string = url.String();

    size_t headerStart = dataString.size();
    size_t headerEnd = string.find(u8',', headerStart);
    if (headerEnd == utf8_stringview::npos)
    {
      return null;
    }
    if (string.substr(0, headerEnd).rfind(u8'#') != utf8_stringview::npos)
    {
      return null;
    }
    size_t encodedDataStart = headerEnd + 1;

    auto header = string.substr(headerStart, headerEnd - headerStart);

    // There might one or two semicolons in the header, find the last one.
    size_t mediaTypeEnd = header.rfind(u8';');
    mediaTypeEnd = mediaTypeEnd == utf8_stringview::npos ? header.size() : mediaTypeEnd;

    // formatTypeStart might be at the begining of "base64" or "charset=...".
    size_t formatTypeStart = std::min(mediaTypeEnd + 1, header.size());
    auto formatType = Trim(header.substr(formatTypeStart));

    bool isBase64 = ToASCIILowercase(formatType) == u8"base64";

    // If header does not end with "base64", mediaType should be the whole header.
    utf8_string mediaType(Trim(isBase64 ? header.substr(0, mediaTypeEnd) : header));
    if (mediaType.starts_with(u8';'))
    {
      mediaType = u8"text/plain" + mediaType;
    }

    // The fragment identifier isn't part of the data.
    auto encodedData = string.substr(encodedDataStart);
    if (auto fragment = encodedData.find(u8'#'); fragment != utf8_stringview::npos)
    {
      encodedData = encodedData.substr(0, fragment);
    }

    auto result = ParseMediaType(mediaType);

    auto unescaped = DecodeURLEscapeSequencesAsData(encodedData);
    if (isBase64)
    {
      EnumSet<Base64DecodeOption> options {Base64DecodeOption::IgnoreWhitespace};
      if (shouldValidatePadding == ShouldValidatePadding::Yes)
      {
        options.Add(Base64DecodeOption::ValidatePadding);
      }
      auto decodedData = Base64Decode(unescaped, options);
      if (!decodedData)
      {
        return null;
      }
      result.Data = krys::move(*decodedData);
    }
    else
    {
      result.Data = krys::move(unescaped);
    }

    result.Data.shrink_to_fit();
    return result;
  }
}
