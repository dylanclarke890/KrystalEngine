#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"
#include "Krystal.Text/Unicode.hpp"

namespace Krys
{
  class ASCIIEncoding : public Encoding
  {
  private:
    KRYS_NODISCARD static EncodingInfo GetEncodingInfo() noexcept
    {
      static EncodingInfo info {
        .Name = u8"US-ASCII",
        .Aliases = {u8"us-ascii", u8"ansi_x3.4-1968", u8"ascii", u8"iso-ir-6", u8"iso646-us", u8"us",
                    u8"x-ascii"},
        .MIBenum = MIBenum {3u},
        .Win32CodePage = Win32CodePage {20'127u},
        .IsSingleByte = IsSingleByteEncoding {true},
      };

      return info;
    }

  public:
    ASCIIEncoding() noexcept
        : Encoding(GetEncodingInfo(), EncoderFallback(EncodingReplacement_ASCII),
                   DecoderFallback(EncodingReplacement_UTF))
    {
    }

    virtual ~ASCIIEncoding() noexcept = default;

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

      const auto EncodeCodepoint = [&](UnicodeCodepoint ch, bool wasInvalid) noexcept
      {
        if (wasInvalid || !Unicode::IsASCIICharacter(ch))
        {
          Encode(_encoderFallback.GetReplacementCharacter(), out);
        }
        else
        {
          out.push_back(static_cast<byte>(ch.Value));
        }
      };

      Unicode::ForEachCodepoint(characters, EncodeCodepoint);
    }

    void Decode(Span<const byte> bytes, utf8_string &out) const noexcept override
    {
      Reserve(out, bytes.size());

      for (byte b : bytes)
      {
        if (Unicode::IsASCIICharacter(b))
        {
          out.push_back(static_cast<char8_t>(b));
        }
        else
        {
          out += _decoderFallback.GetReplacementCharacter();
        }
      }
    }
  };
}