#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"

namespace Krys::Text
{
  /// @brief Represents a UTF-8 character encoding.
  class UTF8Encoding : public Encoding
  {
  public:
    static constexpr Array<byte, 3u> BOM = {byte {0xEF}, byte {0xBB}, byte {0xBF}};
    static constexpr utf8_stringview IANA = u8"UTF-8";

    static constexpr EncodingInfo EncodingInformation {IANA, 65'001u};

  public:
    constexpr UTF8Encoding() noexcept : Encoding(EncodingInformation)
    {
    }

    constexpr virtual ~UTF8Encoding() noexcept = default;

    NO_DISCARD constexpr Span<const byte> GetBOM() const noexcept override
    {
      return Span<const byte> {BOM.data(), BOM.size()};
    }

    NO_DISCARD constexpr bool IsSingleByte() const noexcept override
    {
      return true;
    }

    NO_DISCARD constexpr List<byte> Encode(utf8_stringview characters) const noexcept override
    {
      List<byte> bytes;
      Encode(characters, bytes);
      return bytes;
    }

    constexpr void Encode(utf8_stringview characters, List<byte> &out) const noexcept override
    {
      Reserve(out, characters.size());

      for (char8_t ch : characters)
      {
        out.push_back(static_cast<byte>(ch));
      }
    }

    NO_DISCARD constexpr utf8_string Decode(Span<const byte> bytes) const noexcept override
    {
      utf8_string characters;
      Decode(bytes, characters);
      return characters;
    }

    void constexpr Decode(Span<const byte> bytes, utf8_string &out) const noexcept override
    {
      Reserve(out, bytes.size());

      for (byte b : bytes)
      {
        out.push_back(static_cast<char8_t>(b));
      }
    }
  };
}