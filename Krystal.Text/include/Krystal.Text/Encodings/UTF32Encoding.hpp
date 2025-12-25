#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Endian.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"

namespace Krys::Text
{
  /// @brief Represents a UTF-32 character encoding.
  template <bool IsBigEndian>
  class UTF32Encoding : public Encoding
  {
  public:
    static constexpr Array<byte, 4u> BOM_BE = {byte {0x00}, byte {0x00}, byte {0xFF}, byte {0xFE}};
    static constexpr utf8_stringview IANA_BE = u8"UTF-32BE";

    static constexpr Array<byte, 4u> BOM_LE = {byte {0xFE}, byte {0xFF}, byte {0x00}, byte {0x00}};
    static constexpr utf8_stringview IANA_LE = u8"UTF-32LE";

    static constexpr Endian::Type Endianness {IsBigEndian ? Endian::Big : Endian::Little};
    static constexpr EncodingInfo EncodingInformation =
      IsBigEndian ? EncodingInfo {IANA_BE, 12'001u} : EncodingInfo {IANA_LE, 12'000u};

  public:
    constexpr UTF32Encoding() noexcept : Encoding(EncodingInformation)
    {
    }

    constexpr virtual ~UTF32Encoding() noexcept = default;

    NO_DISCARD constexpr Span<const byte> GetBOM() const noexcept override
    {
      return IsBigEndian ? Span<const byte> {BOM_BE.data(), BOM_BE.size()}
                         : Span<const byte> {BOM_LE.data(), BOM_LE.size()};
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

      const auto EncodeUTF32 = [&](UnicodeCodepoint codepoint, bool wasInvalid) noexcept
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

    NO_DISCARD constexpr utf8_string Decode(Span<const byte> bytes) const noexcept override
    {
      utf8_string characters;
      Decode(bytes, characters);
      return characters;
    }

    constexpr void Decode(Span<const byte> bytes, utf8_string &out) const noexcept override
    {
      Reserve(out, bytes.size() / 4u);

      const auto DecodeUTF32 = [&](FixedSpan<const byte, 4u> byteSpan) noexcept
      {
        auto codepoint =
          UnicodeCodepoint(ByteUtils::AsNumeric<Endianness, Endian::System, uint32>(byteSpan.data()));

        if (Unicode::IsValidCodepoint(codepoint))
        {
          Unicode::CodepointToUTF8(codepoint, out);
        }
        else
        {
          out += _decoderFallback.GetReplacementCharacter();
        }
      };

      ByteUtils::ForEachNBytes<4u>(bytes, DecodeUTF32);
    }
  };

  /// @brief Represents a UTF-32 little-endian character encoding.
  using UTF32EncodingLE = UTF32Encoding<false>;

  /// @brief Represents a UTF-32 big-endian character encoding.
  using UTF32EncodingBE = UTF32Encoding<true>;
}