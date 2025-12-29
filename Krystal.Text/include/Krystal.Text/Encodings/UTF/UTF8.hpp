#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"

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
        .ByteOrderMark = {Rune(0xEF'BB'BF'00u), 3u},
      };

      return info;
    }

  public:
    UTF8Encoding() noexcept
        : Encoding(GetEncodingInfo(), EncoderFallback(EncodingReplacement_UTF),
                   DecoderFallback(EncodingReplacement_UTF))
    {
    }

    virtual ~UTF8Encoding() noexcept = default;

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
      Reserve(out, characters.size());

      const auto EncodeCodepoint = [&](Rune ch, bool wasInvalid) noexcept
      {
        if (wasInvalid)
        {
          Encode(_encoderFallback.GetReplacementCharacter(), out);
        }
        else
        {
          Unicode::ToUTF8(ch, out);
        }
      };

      Unicode::ForEachCodepoint(characters, EncodeCodepoint);
    }

    void Decode(Span<const byte> bytes, utf8_string &out) const noexcept override
    {
      Reserve(out, bytes.size());

      const auto DecodeCodepoint = [&](Rune ch, bool wasInvalid) noexcept
      {
        if (wasInvalid)
        {
          out += _decoderFallback.GetReplacementCharacter();
        }
        else
        {
          Unicode::ToUTF8(ch, out);
        }
      };

      Unicode::ForEachCodepoint(bytes, DecodeCodepoint);
    }
  };
}