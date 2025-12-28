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
  class UTF16Encoding : public Encoding
  {
  private:
    KRYS_NODISCARD static EncodingInfo GetEncodingInfo() noexcept
    {
      if constexpr (IsBigEndian)
      {
        static EncodingInfo info {
          .Name = u8"UTF-16BE",
          .Aliases = {u8"utf-16be", u8"unicodefffe"},
          .MIBenum = MIBenum {1'013u},
          .Win32CodePage = Win32CodePage {1'201u},
          .IsSingleByte = IsSingleByteEncoding {false},
          .ByteOrderMark = {UnicodeCodepoint(0xFE'FF'00'00u), 2u},
        };

        return info;
      }
      else
      {
        static EncodingInfo info {
          .Name = u8"UTF-16LE",
          .Aliases = {u8"utf-16le", u8"csunicode", u8"iso-10646-ucs-2", u8"ucs-2", u8"unicode",
                      u8"unicodefeff", u8"utf-16"},
          .MIBenum = MIBenum {1'014u},
          .Win32CodePage = Win32CodePage {1'200u},
          .IsSingleByte = IsSingleByteEncoding {false},
          .ByteOrderMark = {UnicodeCodepoint(0xFF'FE'00'00u), 2u},
        };

        return info;
      }
    }

  public:
    static constexpr Endian::Type Endianness {IsBigEndian ? Endian::Big : Endian::Little};

  public:
    UTF16Encoding() noexcept
        : Encoding(GetEncodingInfo(), EncoderFallback(EncodingReplacement_UTF),
                   DecoderFallback(EncodingReplacement_UTF))
    {
    }

    virtual ~UTF16Encoding() noexcept = default;

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

    void Encode(utf8_stringview characters, List<byte> &out) const noexcept override
    {
      Reserve(out, characters.size() * 2u);

      const auto EncodeCodepoint = [&](UnicodeCodepoint codepoint, bool wasInvalid) noexcept
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

      Unicode::ForEachCodepoint(characters, EncodeCodepoint);
    }

    void Decode(Span<const byte> bytes, utf8_string &out) const noexcept
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