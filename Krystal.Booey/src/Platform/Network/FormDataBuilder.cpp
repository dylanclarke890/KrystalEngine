#include "Krystal.Booey/Platform/Network/FormDataBuilder.hpp"
#include <array>
#include <random>

namespace krys::boo::FormDataBuilder
{
  namespace
  {
    void Append(List<uint8> &buffer, char character) noexcept
    {
      buffer.push_back(static_cast<uint8>(character));
    }

    void Append(List<uint8> &buffer, Span<const uint8> bytes) noexcept
    {
      buffer.insert(buffer.end(), bytes.begin(), bytes.end());
    }

    void Append(List<uint8> &buffer, std::string_view string) noexcept
    {
      for (char character : string)
      {
        Append(buffer, character);
      }
    }

    void AppendQuoted(List<uint8> &buffer, Span<const uint8> string) noexcept
    {
      // Append a string as a quoted value, escaping quotes and line breaks.
      // FIXME: Is it correct to use percent escaping here? When this code was originally written, other
      // browsers were not encoding these characters, so someone should test servers or do research to find
      // out if there is an encoding form that works well.
      // FIXME: If we want to use percent escaping sensibly, we need to escape "%" characters too.
      for (auto character : string)
      {
        switch (character)
        {
          case 0xA: Append(buffer, "%0A"); break;
          case 0xD: Append(buffer, "%0D"); break;
          case '"': Append(buffer, "%22"); break;
          default:  buffer.push_back(character);
        }
      }
    }

    // https://url.spec.whatwg.org/#concept-urlencoded-uint8-serializer
    void AppendFormURLEncoded(List<uint8> &buffer, Span<const uint8> string) noexcept
    {
      constexpr std::string_view safeCharacters = "-._*";
      for (size_t i = 0; i < string.size(); ++i)
      {
        auto character = string[i];
        if (text::IsASCIIAlphanumeric(static_cast<char>(character))
            || (character != '\0'
                && safeCharacters.find(static_cast<char>(character)) != std::string_view::npos))
        {
          buffer.push_back(character);
        }
        else if (character == ' ')
        {
          Append(buffer, '+');
        }
        else if (character == '\n'
                 || (character == '\r' && (i + 1 >= string.size() || string[i + 1] != '\n')))
        {
          // FIXME: Unclear exactly where this rule about normalizing line endings to CRLF comes from.
          Append(buffer, "%0D%0A");
        }
        else if (character != '\r')
        {
          constexpr std::string_view hexDigits = "0123456789ABCDEF";
          Append(buffer, '%');
          Append(buffer, hexDigits[character >> 4]);
          Append(buffer, hexDigits[character & 0xF]);
        }
      }
    }
  }

  List<uint8> GenerateUniqueBoundaryString() noexcept
  {
    List<uint8> boundary;

    // The RFC 2046 spec says the alphanumeric characters plus the following characters are legal for
    // boundaries:  '()+_,-./:=?
    // However the following characters, though legal, cause some sites to fail: (),./:=+
    // Note that our algorithm makes it twice as much likely for 'A' or 'B' to appear in the boundary string,
    // because 0x41 and 0x42 are present in the below array twice.
    static constexpr std::array<char, 64> alphaNumericEncodingMap {
      0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
      0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
      0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
      0x77, 0x78, 0x79, 0x7A, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42};

    // Start with an informative prefix.
    Append(boundary, "----BooeyFormBoundary");

    // Append 16 random 7-bit ASCII alphanumeric characters.
    std::random_device randomDevice;
    for (uint32 i = 0; i < 4; ++i)
    {
      auto randomness = static_cast<uint32>(randomDevice());
      Append(boundary, alphaNumericEncodingMap[(randomness >> 24) & 0x3F]);
      Append(boundary, alphaNumericEncodingMap[(randomness >> 16) & 0x3F]);
      Append(boundary, alphaNumericEncodingMap[(randomness >> 8) & 0x3F]);
      Append(boundary, alphaNumericEncodingMap[randomness & 0x3F]);
    }

    return boundary;
  }

  void BeginMultiPartHeader(List<uint8> &buffer, Span<const uint8> boundary, Span<const uint8> name) noexcept
  {
    AddBoundaryToMultiPartHeader(buffer, boundary);

    // FIXME: This loses data irreversibly if the input name includes characters you can't encode in the
    // website's character set.
    Append(buffer, "Content-Disposition: form-data; name=\"");
    AppendQuoted(buffer, name);
    Append(buffer, '"');
  }

  void AddBoundaryToMultiPartHeader(List<uint8> &buffer, Span<const uint8> boundary,
                                    bool isLastBoundary) noexcept
  {
    Append(buffer, "--");
    Append(buffer, boundary);

    if (isLastBoundary)
    {
      Append(buffer, "--");
    }

    Append(buffer, "\r\n");
  }

  void AddFilenameToMultiPartHeader(List<uint8> &buffer, utf8_stringview filename) noexcept
  {
    Append(buffer, "; filename=\"");
    AppendQuoted(buffer,
                 Span<const uint8>(reinterpret_cast<const uint8 *>(filename.data()), filename.size()));
    Append(buffer, '"');
  }

  void AddContentTypeToMultiPartHeader(List<uint8> &buffer, utf8_stringview mimeType) noexcept
  {
    Append(buffer, "\r\nContent-Type: ");
    Append(buffer, Span<const uint8>(reinterpret_cast<const uint8 *>(mimeType.data()), mimeType.size()));
  }

  void FinishMultiPartHeader(List<uint8> &buffer) noexcept
  {
    Append(buffer, "\r\n\r\n");
  }

  void AddKeyValuePairAsFormData(List<uint8> &buffer, Span<const uint8> key, Span<const uint8> value,
                                 FormData::EncodingType encodingType) noexcept
  {
    if (encodingType == FormData::EncodingType::TextPlain)
    {
      Append(buffer, key);
      Append(buffer, '=');
      Append(buffer, value);
      Append(buffer, "\r\n");
    }
    else
    {
      if (!buffer.empty())
      {
        Append(buffer, '&');
      }
      AppendFormURLEncoded(buffer, key);
      Append(buffer, '=');
      AppendFormURLEncoded(buffer, value);
    }
  }

  void EncodeStringAsFormData(List<uint8> &buffer, Span<const uint8> string) noexcept
  {
    AppendFormURLEncoded(buffer, string);
  }
}
