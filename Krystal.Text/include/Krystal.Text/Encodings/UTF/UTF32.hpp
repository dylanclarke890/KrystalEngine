#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Core/Endian.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"

namespace Krys::Text
{
  template <bool IsBigEndian>
  class UTF32Encoding : public Encoding
  {
  public:
    static constexpr utf8_stringview Name_BE = u8"UTF-32BE";
    static constexpr uint32 MIBenum_BE = 1'018u;
    static constexpr uint32 WindowsCodePage_BE = 12'001u;
    static constexpr Array<byte, 4u> BOM_BE = {byte {0x00}, byte {0x00}, byte {0xFF}, byte {0xFE}};

    static constexpr utf8_stringview Name_LE = u8"UTF-32LE";
    static constexpr uint32 MIBenum_LE = 1'019u;
    static constexpr uint32 WindowsCodePage_LE = 12'000u;
    static constexpr Array<byte, 4u> BOM_LE = {byte {0xFE}, byte {0xFF}, byte {0x00}, byte {0x00}};

    static constexpr Endian::Type Endianness {IsBigEndian ? Endian::Big : Endian::Little};

  public:
    constexpr UTF32Encoding() noexcept
        : Encoding(IsBigEndian ? EncodingInfo {Name_BE, MIBenum_BE, WindowsCodePage_BE}
                               : EncodingInfo {Name_LE, MIBenum_LE, WindowsCodePage_LE})
    {
    }

    constexpr virtual ~UTF32Encoding() noexcept = default;

    KRYS_NODISCARD constexpr Span<const byte> GetBOM() const noexcept override
    {
      return IsBigEndian ? Span<const byte> {BOM_BE.data(), BOM_BE.size()}
                         : Span<const byte> {BOM_LE.data(), BOM_LE.size()};
    }

    KRYS_NODISCARD constexpr bool IsSingleByte() const noexcept override
    {
      return false;
    }

    KRYS_NODISCARD constexpr List<byte> Encode(utf8_stringview characters) const noexcept override
    {
      List<byte> bytes;
      Encode(characters, bytes);
      return bytes;
    }

    KRYS_NODISCARD constexpr utf8_string Decode(Span<const byte> bytes) const noexcept override
    {
      utf8_string characters;
      Decode(bytes, characters);
      return characters;
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

    constexpr void Decode(Span<const byte> bytes, utf8_string &out) const noexcept override
    {
      Reserve(out, bytes.size() / 4u);

      const auto EncodeCodepoint = [&](FixedSpan<const byte, 4u> byteSpan) noexcept
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

      ByteUtils::ForEachNBytes<4u>(bytes, EncodeCodepoint);
    }
  };

  /// @brief Represents a UTF-32 little-endian character encoding.
  using UTF32EncodingLE = UTF32Encoding<false>;

  /// @brief Represents a UTF-32 big-endian character encoding.
  using UTF32EncodingBE = UTF32Encoding<true>;
}