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
  template <bool IsBigEndian>
  class UTF16Encoding : public Encoding
  {
  public:
    static constexpr Array<byte, 2u> BOM_BE = {byte {0xFE}, byte {0xFF}};
    static constexpr utf8_stringview IANA_BE = u8"UTF-16BE";

    static constexpr Array<byte, 2u> BOM_LE = {byte {0xFF}, byte {0xFE}};
    static constexpr utf8_stringview IANA_LE = u8"UTF-16LE";

    static constexpr Endian::Type Endianness {IsBigEndian ? Endian::Big : Endian::Little};
    static constexpr EncodingInfo EncodingInformation =
      IsBigEndian ? EncodingInfo {IANA_BE, 1'201u} : EncodingInfo {IANA_LE, 1'200u};

  public:
    constexpr UTF16Encoding() noexcept : Encoding(EncodingInformation)
    {
    }

    constexpr virtual ~UTF16Encoding() noexcept = default;

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

    NO_DISCARD constexpr utf8_string Decode(Span<const byte> bytes) const noexcept override
    {
      utf8_string characters;
      Decode(bytes, characters);
      return characters;
    }

    constexpr void Encode(utf8_stringview characters, List<byte> &out) const noexcept override
    {
      Reserve(out, characters.size() * 2u);

      const auto EncodeUTF16 = [&](UnicodeCodepoint codepoint, bool wasInvalid) noexcept
      {
        if (wasInvalid || Unicode::IsSurrogateCodepoint(codepoint))
        {
          Encode(_encoderFallback.GetReplacementCharacter(), out);
          return;
        }

        if (Unicode::IsBasicMultilingualPlaneCodepoint(codepoint))
        {
          uint16 cu = static_cast<uint16>(codepoint.Value);
          ByteUtils::ToBytes<Endian::System, Endianness>(cu, out);
          return;
        }

        uint32 surrogate = codepoint.Value - 0x10000;

        uint16 high = Unicode::SurrogateHighStart + (surrogate >> 10);
        ByteUtils::ToBytes<Endian::System, Endianness>(high, out);

        uint16 low = Unicode::SurrogateLowStart + (surrogate & 0x3FF);
        ByteUtils::ToBytes<Endian::System, Endianness>(low, out);
      };

      Unicode::ForEachCodepoint(characters, EncodeUTF16);
    }

    constexpr void Decode(Span<const byte> bytes, utf8_string &out) const noexcept
    {
      Reserve(out, bytes.size() / 2u);

      auto it = bytes.begin();
      const auto end = bytes.end();

      while (it != end)
      {
        // Need at least one code unit
        if (static_cast<size_t>(end - it) < 2u)
        {
          out += _decoderFallback.GetReplacementCharacter();
          break;
        }

        auto *current = bytes.data() + static_cast<size_t>(it - bytes.begin());
        UnicodeCodepoint first {ByteUtils::AsNumeric<Endianness, Endian::System, uint16>(current)};
        it += 2u;

        if (!Unicode::IsSurrogateCodepoint(first))
        {
          Unicode::CodepointToUTF8(first, out);
          continue;
        }

        if (Unicode::IsHighSurrogate(first))
        {
          if (static_cast<size_t>(end - it) < 2u)
          {
            out += _decoderFallback.GetReplacementCharacter();
            break;
          }

          uint16 second = ByteUtils::AsNumeric<Endianness, Endian::System, uint16>(current + 2u);
          if (Unicode::IsLowSurrogate(UnicodeCodepoint(second)))
          {
            uint32 codepoint = 0x10000u + ((first.Value - Unicode::SurrogateHighStart) << 10u)
                               + (second - Unicode::SurrogateLowStart);
            Unicode::CodepointToUTF8(UnicodeCodepoint(codepoint), out);
            it += 2u;
          }
          else
          {
            // High surrogate not followed by low surrogate
            out += _decoderFallback.GetReplacementCharacter();
          }

          continue;
        }

        // Lone low surrogate
        out += _decoderFallback.GetReplacementCharacter();
      }
    }
  };

  /// @brief Represents a UTF-16 little-endian character encoding.
  using UTF16EncodingLE = UTF16Encoding<false>;

  /// @brief Represents a UTF-16 big-endian character encoding.
  using UTF16EncodingBE = UTF16Encoding<true>;
}