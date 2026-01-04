#pragma once

#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Endian.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/Old/Encodings/Encoding.hpp"
#include "Krystal.Text/Old/Unicode.hpp"

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
          .ByteOrderMark = {{byte {0xFE}, byte {0xFF}}, 2u},
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
          .ByteOrderMark = {{byte {0xFF}, byte {0xFE}}, 2u},
        };

        return info;
      }
    }

  public:
    static constexpr Endian::Type Endianness {IsBigEndian ? Endian::Big : Endian::Little};

  public:
    UTF16Encoding() noexcept : Encoding(GetEncodingInfo())
    {
    }

    virtual ~UTF16Encoding() noexcept = default;

    void Encode(Span<const Rune> characters, List<byte> &out) const noexcept override
    {
      for (Rune ch : characters)
      {
        if (Unicode::IsSurrogateCodepoint(static_cast<uint16>(ch)))
        {
          // Invalid codepoint, encode replacement character
          uint16 cu = static_cast<uint16>(Unicode::ReplacementCharacter);
          ByteUtils::ToBytes<Endian::System, Endianness>(cu, out);
          continue;
        }

        if (Unicode::IsBasicMultilingualPlaneCodepoint(ch))
        {
          uint16 cu = static_cast<uint16>(ch);
          ByteUtils::ToBytes<Endian::System, Endianness>(cu, out);
          continue;
        }

        uint16 high, low;
        Unicode::EncodeSurrogatePair(ch, high, low);

        ByteUtils::ToBytes<Endian::System, Endianness>(high, out);
        ByteUtils::ToBytes<Endian::System, Endianness>(low, out);
      }
    }

    KRYS_NODISCARD virtual size_t GetMaxByteCount(size_t charCount) const noexcept
    {
      return charCount * 4u; // Up to 4 bytes per character in UTF-16
    }

    void Decode(Span<const byte> bytes, List<Rune> &out) const noexcept
    {
      auto it = bytes.begin();
      const auto end = bytes.end();

      while (it != end)
      {
        // Need at least one code unit
        if (static_cast<size_t>(end - it) < 2u)
        {
          out.push_back(Unicode::ReplacementCharacter);
          break;
        }

        auto *current = bytes.data() + static_cast<size_t>(it - bytes.begin());
        uint16 first = ByteUtils::AsNumeric<Endianness, Endian::System, uint16>(current);
        it += 2u;

        if (Unicode::IsValidCodepoint(first))
        {
          out.push_back(Rune(first));
          continue;
        }

        if (!Unicode::IsHighSurrogate(first)) // Lone low surrogate
        {
          out.push_back(Unicode::ReplacementCharacter);
          continue;
        }

        if (static_cast<size_t>(end - it) < 2u) // High surrogate not followed by low surrogate
        {
          out.push_back(Unicode::ReplacementCharacter); 
          break;
        }

        uint16 second = ByteUtils::AsNumeric<Endianness, Endian::System, uint16>(current);
        if (Unicode::IsLowSurrogate(second))
        {
          out.push_back(Unicode::DecodeSurrogatePair(first, second));
          it += 2u; // Only advance now we've consumed the low surrogate
        }
        else
        {
          out.push_back(Unicode::ReplacementCharacter);
        }
      }


    }
  
    KRYS_NODISCARD size_t GetMaxCharCount(size_t byteCount) const noexcept override
    {
      return byteCount / 2u; // At least 2 bytes per character in UTF-16
    }
  };

  /// @brief Represents a UTF-16 little-Endian::Type character encoding.
  using UTF16EncodingLE = UTF16Encoding<false>;

  /// @brief Represents a UTF-16 big-Endian::Type character encoding.
  using UTF16EncodingBE = UTF16Encoding<true>;
}