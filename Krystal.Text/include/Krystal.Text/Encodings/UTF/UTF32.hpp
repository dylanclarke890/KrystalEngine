#pragma once

#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Endian.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"
#include "Krystal.Text/Unicode.hpp"

namespace Krys
{
  template <bool IsBigEndian>
  class UTF32Encoding : public Encoding
  {
  private:
    constexpr static utf8_stringview ReplacementCharacterStr = u8"\xFF\xFD";

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
          .ByteOrderMark = {{byte {0x00}, byte {0x00}, byte {0xFF}, byte {0xFE}}, 4u},
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
          .ByteOrderMark = {{byte {0xFE}, byte {0xFF}, byte {0x00}, byte {0x00}}, 4u},
        };

        return info;
      }
    }

  public:
    static constexpr Endian::Type Endianness {IsBigEndian ? Endian::Big : Endian::Little};

  public:
    UTF32Encoding() noexcept : Encoding(GetEncodingInfo())
    {
    }

    virtual ~UTF32Encoding() noexcept = default;

    void Encode(utf8_stringview characters, List<byte> &out) const noexcept
    {
      Unicode::ForEachCodepoint(characters, [&](Rune codepoint, bool wasInvalid) noexcept
                                { ByteUtils::ToBytes<Endian::System, Endianness>(codepoint.Value, out); });
    }

    void Decode(Span<const byte> bytes, utf8_string &out) const noexcept override
    {
      const auto EncodeCodepoint = [&](FixedSpan<const byte, 4u> byteSpan) noexcept
      {
        uint32 codepoint = ByteUtils::AsNumeric<Endianness, Endian::System, uint32>(byteSpan.data());
        if (Unicode::IsValidCodepoint(codepoint))
        {
          Unicode::ToUTF8(Rune(codepoint), out);
        }
        else
        {
          Unicode::ToUTF8(Unicode::ReplacementCharacter, out);
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