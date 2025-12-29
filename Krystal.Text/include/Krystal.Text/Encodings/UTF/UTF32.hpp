#pragma once

#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Endian.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"

namespace Krys
{
  template <bool IsBigEndian>
  class UTF32Encoding : public Encoding
  {
  private:
    KRYS_NODISCARD static EncodingInfo GetEncodingInfo() noexcept
    {
      if constexpr (IsBigEndian)
      {
        static EncodingInfo info {
          .Name = u8"UTF-32BE",
          .Aliases = {},
          .MIBenum = MIBenum {1'018u},
          .Win32CodePage = Win32CodePage {12'001u},
          .IsSingleByte = IsSingleByteEncoding {false},
          .ByteOrderMark = {Rune(0x00'00'FF'FEu), 4u},
        };

        return info;
      }
      else
      {
        static EncodingInfo info {
          .Name = u8"UTF-32LE",
          .Aliases = {},
          .MIBenum = MIBenum {1'019u},
          .Win32CodePage = Win32CodePage {12'000u},
          .IsSingleByte = IsSingleByteEncoding {false},
          .ByteOrderMark = {Rune(0xFE'FF'00'00u), 4u},
        };

        return info;
      }
    }

  public:
    static constexpr Endian::Type Endianness {IsBigEndian ? Endian::Big : Endian::Little};

  public:
    UTF32Encoding() noexcept
        : Encoding(GetEncodingInfo(), EncoderFallback(EncodingReplacement_UTF),
                   DecoderFallback(EncodingReplacement_UTF))
    {
    }

    virtual ~UTF32Encoding() noexcept = default;

    KRYS_NODISCARD List<byte> Encode(utf8_stringview characters) const noexcept override
    {
      List<byte> bytes;
      Encode(characters, bytes);
      return bytes;
    }

    KRYS_NODISCARD utf8_string Decode(Span<const byte> bytes) const noexcept override
    {
      utf8_string characters;
      Decode(bytes, characters);
      return characters;
    }

    void Encode(utf8_stringview characters, List<byte> &out) const noexcept
    {
      Reserve(out, characters.size() * 4u);

      const auto EncodeUTF32 = [&](Rune codepoint, bool wasInvalid) noexcept
      {
        if (wasInvalid)
        {
          Encode(_encoderFallback.GetReplacementCharacter(), out);
        }
        else
        {
          ByteUtils::ToBytes<Endian::System, Endianness>(codepoint.Value, out);
        }
      };

      Unicode::ForEachCodepoint(characters, EncodeUTF32);
    }

    void Decode(Span<const byte> bytes, utf8_string &out) const noexcept override
    {
      Reserve(out, bytes.size() / 4u);

      const auto EncodeCodepoint = [&](FixedSpan<const byte, 4u> byteSpan) noexcept
      {
        uint32 codepoint = ByteUtils::AsNumeric<Endianness, Endian::System, uint32>(byteSpan.data());
        if (Unicode::IsValidCodepoint(codepoint))
        {
          Unicode::ToUTF8(Rune(codepoint), out);
        }
        else
        {
          out += _decoderFallback.GetReplacementCharacter();
        }
      };

      ByteUtils::ForEachNBytes<4u>(bytes, EncodeCodepoint);
    }
  };

  /// @brief Represents a UTF-32 little-endian character encoding.
  using UTF32EncodingLE = UTF32Encoding<false>;

  /// @brief Represents a UTF-32 big-endian character encoding.
  using UTF32EncodingBE = UTF32Encoding<true>;
}