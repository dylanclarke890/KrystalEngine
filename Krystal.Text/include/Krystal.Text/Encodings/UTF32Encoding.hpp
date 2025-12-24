#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"

namespace Krys::Text
{
  /// @brief Represents a UTF-32 little-endian character encoding.
  template <bool IsBigEndian>
  class UTF32Encoding : public Encoding
  {
    static constexpr EncodingInfo EncodingInformation {IsBigEndian ? IANAName_UTF32_BE : IANAName_UTF32_LE};
    static constexpr Endian::Type Endianness {IsBigEndian ? Endian::Big : Endian::Little};

  public:
    constexpr UTF32Encoding() noexcept : Encoding(EncodingInformation)
    {
    }

    constexpr virtual ~UTF32Encoding() noexcept = default;

    NO_DISCARD constexpr List<byte> GetPreamble() const noexcept override
    {
      return IsBigEndian ? List<byte> {byte {0xFE}, byte {0xFF}, byte {0x00}, byte {0x00}}
                         : List<byte> {byte {0x00}, byte {0x00}, byte {0xFF}, byte {0xFE}};
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

    constexpr void Encode(utf8_stringview characters, List<byte> &out) const noexcept
    {
      Reserve(out, characters.size() * 4u);

      const auto EncodeUTF32 = [&](UnicodeCodepoint codepoint) noexcept
      {
        if (Unicode::IsValidCodepoint(codepoint))
        {
          ByteUtils::ToBytes<Endian::System, Endianness>(codepoint.Value, out);
        }
        else
        {
          Encode(_encoderFallback.GetReplacementCharacter(), out);
        }
      };
      Unicode::ForEachCodepoint(characters, EncodeUTF32);
    }
    
    NO_DISCARD constexpr utf8_string Decode(Span<const byte> bytes) const noexcept override
    {
      utf8_string characters;
      Decode(bytes, characters);
      return characters;
    }
    
    constexpr void Decode(Span<const byte> bytes, utf8_string &out) const noexcept override
    {
      Reserve(out, bytes.size() / 4u);

      const auto DecodeUTF32 = [&](Span<const byte, 4u> byteSpan) noexcept
      {
        UnicodeCodepoint codepoint =
          ByteUtils::AsNumeric<Endianness, Endian::System, uint32>(byteSpan.data());

        if (Unicode::IsValidCodepoint(codepoint))
        {
          Unicode::CodepointToUTF8(codepoint, str);
        }
        else
        {
          str.append(_decoderFallback.GetReplacementString());
        }
      };

      ByteUtils::ForEachByteSpan<DecodeUTF32, 4u>(bytes, DecodeUTF32);
    }
  };
}