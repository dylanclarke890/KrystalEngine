#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"

namespace Krys::Text
{
  /// @brief Represents a UTF-16 big-endian character encoding.
  template <bool IsBigEndian>
  class UTF16Encoding : public Encoding
  {
    static constexpr EncodingInfo EncodingInformation {IsBigEndian ? IANAName_UTF16_BE : IANAName_UTF16_LE};

  public:
    constexpr UTF16Encoding() noexcept : Encoding(EncodingInformation)
    {
    }

    constexpr virtual ~UTF16Encoding() noexcept = default;

    NO_DISCARD constexpr List<byte> GetPreamble() const noexcept override
    {
      return IsBigEndian ? List<byte> {byte {0xFE}, byte {0xFF}} : List<byte> {byte {0xFF}, byte {0xFE}};
    }

    NO_DISCARD constexpr bool IsSingleByte() const noexcept override
    {
      return false;
    }

    NO_DISCARD constexpr List<byte> Encode(utf8_stringview characters) const noexcept override
    {
      List<byte> bytes;
      Encode(characters, bytes);
      return bytes;
    }

    constexpr void Encode(utf8_stringview characters, List<byte> &out) const noexcept override
    {
      Reserve(out, characters.size() * 2u);
    }

    NO_DISCARD constexpr utf8_string Decode(Span<const byte> bytes) const noexcept override
    {
      utf8_string characters;
      Decode(bytes, characters);
      return characters;
    }

    constexpr void Decode(Span<const byte> bytes, utf8_string &out) const noexcept override
    {
      Reserve(out, bytes.size() / 2u);
    }
  };
}