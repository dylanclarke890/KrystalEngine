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

    void Encode(utf8_stringview characters, List<byte> &out) const noexcept override
    {
      Unicode::ForEachCodepoint(characters, [&](Rune ch) noexcept { Unicode::ToUTF8(ch, out); });
    }

    void Decode(Span<const byte> bytes, utf8_string &out) const noexcept override
    {
      Unicode::ForEachCodepoint(bytes, [&](Rune ch) noexcept { Unicode::ToUTF8(ch, out); });
    }
  };
}