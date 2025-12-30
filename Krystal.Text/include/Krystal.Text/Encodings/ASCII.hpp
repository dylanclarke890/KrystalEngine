#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Detection/CPU.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/ASCII.hpp"
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
    ASCIIEncoding() noexcept : Encoding(GetEncodingInfo())
    {
    }

    virtual ~ASCIIEncoding() noexcept = default;

    void Encode(utf8_stringview characters, List<byte> &out) const noexcept override
    {
      const auto EncodeCodepoint = [&](Rune ch, bool replaced) noexcept
      {
        if (replaced || !ASCII::IsASCII(ch))
        {
          out.push_back(static_cast<byte>(ASCII::ReplacementCharacter));
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
      for (byte b : bytes)
      {
        if (ASCII::IsASCII(b))
        {
          out.push_back(static_cast<char8>(b));
        }
        else
        {
          out.push_back(static_cast<char8>(ASCII::ReplacementCharacter));
        }
      }
    }
  };
}