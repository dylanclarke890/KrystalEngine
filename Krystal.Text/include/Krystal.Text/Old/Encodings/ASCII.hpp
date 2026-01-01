#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/Old/ASCII.hpp"
#include "Krystal.Text/Old/Encodings/Encoding.hpp"
#include "Krystal.Text/Old/Unicode.hpp"

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

    void Encode(Span<const Rune> characters, List<byte> &out) const noexcept override
    {
      for (Rune ch : characters)
      {
        if (ASCII::IsASCII(ch))
        {
          out.push_back(ch.ToByte());
        }
        else
        {
          out.push_back(ASCII::ReplacementCharacter.ToByte());
        }
      }
    }

    KRYS_NODISCARD size_t GetMaxByteCount(size_t charCount) const noexcept override
    {
      return charCount; // 1 byte per character in ASCII
    }

    void Decode(Span<const byte> bytes, List<Rune> &out) const noexcept override
    {
      for (byte b : bytes)
      {
        if (ASCII::IsASCII(b))
        {
          out.push_back(Rune(b));
        }
        else
        {
          out.push_back(ASCII::ReplacementCharacter);
        }
      }
    }

    KRYS_NODISCARD size_t GetMaxCharCount(size_t byteCount) const noexcept override
    {
      return byteCount; // 1 character per byte in ASCII
    }
  };
}