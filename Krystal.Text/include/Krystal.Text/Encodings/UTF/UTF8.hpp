#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"
#include "Krystal.Text/Unicode.hpp"

namespace Krys
{
  class UTF8Encoding : public Encoding
  {
  private:
    KRYS_NODISCARD static EncodingInfo GetEncodingInfo() noexcept
    {
      static EncodingInfo info {
        .Name = u8"UTF-8",
        .Aliases = {u8"utf-8", u8"unicode-1-1-utf-8", u8"unicode11utf8", u8"unicode20utf8", u8"utf8",
                    u8"x-unicode20utf8"},
        .MIBenum = MIBenum {106u},
        .Win32CodePage = Win32CodePage {65'001u},
        .IsSingleByte = IsSingleByteEncoding {false},
        .ByteOrderMark = {{byte {0xEFu}, byte {0xBBu}, byte {0xBFu}}, 3u},
      };

      return info;
    }

  public:
    UTF8Encoding() noexcept : Encoding(GetEncodingInfo())
    {
    }

    virtual ~UTF8Encoding() noexcept = default;

    void Encode(Span<const Rune> characters, List<byte> &out) const noexcept override
    {
      for (Rune ch : characters)
      {
        if (ASCII::IsASCII(ch))
        {
          out.push_back(ch.ToByte());
        }
        else if (ch <= 0x7FFu)
        {
          out.push_back(static_cast<byte>(0xC0 | (ch >> 6)));
          out.push_back(static_cast<byte>(0x80 | (ch & 0x3F)));
        }
        else if (ch <= Unicode::MaxBasicMultilingualPlaneValue)
        {
          out.push_back(static_cast<byte>(0xE0 | (ch >> 12)));
          out.push_back(static_cast<byte>(0x80 | ((ch >> 6) & 0x3F)));
          out.push_back(static_cast<byte>(0x80 | (ch & 0x3F)));
        }
        else if (ch <= Unicode::MaxSupplementaryPlaneValue)
        {
          out.push_back(static_cast<byte>(0xF0 | (ch >> 18)));
          out.push_back(static_cast<byte>(0x80 | ((ch >> 12) & 0x3F)));
          out.push_back(static_cast<byte>(0x80 | ((ch >> 6) & 0x3F)));
          out.push_back(static_cast<byte>(0x80 | (ch & 0x3F)));
        }
      }
    }

    KRYS_NODISCARD size_t GetMaxByteCount(size_t charCount) const noexcept override
    {
      return charCount * 4u; // Up to 4 bytes per character in UTF-8
    }

    void Decode(Span<const byte> bytes, List<Rune> &out) const noexcept override
    {
      auto it = bytes.begin();
      const auto end = bytes.end();
      while (it != end)
      {
        uint8 first = static_cast<uint8>(*it++);
        if (ASCII::IsASCII(first))
        {
          out.push_back(Rune(first));
          continue;
        }

        uint32 codepoint = 0;
        uint8 needed = 0;
        if ((first & 0xE0) == 0xC0)
        {
          codepoint = first & 0x1F;
          needed = 1;
        }
        else if ((first & 0xF0) == 0xE0)
        {
          codepoint = first & 0x0F;
          needed = 2;
        }
        else if ((first & 0xF8) == 0xF0)
        {
          codepoint = first & 0x07;
          needed = 3;
        }
        else
        {
          out.push_back(Unicode::ReplacementCharacter);
          continue;
        }

        if (static_cast<size_t>(end - it) < needed)
        {
          break;
        }

        for (uint8 i = 0; i < needed; i++)
        {
          uint8 b = static_cast<uint8>(*it);
          if ((b & 0xC0) != 0x80)
          {
            if (out.size() == 0u)
            {
              break;
            }

            it++; // Advance past the malformed byte
            out.push_back(Unicode::ReplacementCharacter);
            continue;
          }
          codepoint = (codepoint << 6) | (b & 0x3F);
          it++;
        }

        out.push_back(Rune(codepoint));
      }
    }

    KRYS_NODISCARD size_t GetMaxCharCount(size_t byteCount) const noexcept override
    {
      return byteCount; // Up to 1 character per byte in UTF-8
    }
  };
}