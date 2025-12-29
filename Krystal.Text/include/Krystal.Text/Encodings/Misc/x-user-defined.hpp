#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/ASCII.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"
#include "Krystal.Text/Unicode.hpp"

namespace Krys
{
  class XUserDefinedEncoding : public Encoding
  {
  private:
    KRYS_NODISCARD static EncodingInfo GetEncodingInfo() noexcept
    {
      static EncodingInfo info {
        .Name = u8"x-user-defined",
        .Aliases = {u8"x-user-defined"},
        .IsSingleByte = IsSingleByteEncoding {true},
      };

      return info;
    }

  public:
    static constexpr uint32 XUserDefinedStart = 0xF780u;
    static constexpr uint32 XUserDefinedEnd = 0xF7FFu;

  public:
    XUserDefinedEncoding() noexcept
        : Encoding(GetEncodingInfo(), EncoderFallback(EncodingReplacement_ASCII),
                   DecoderFallback(EncodingReplacement_UTF))
    {
    }

    virtual ~XUserDefinedEncoding() noexcept = default;

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
        if (wasInvalid || ch < XUserDefinedStart || ch > XUserDefinedEnd)
        {
          Encode(_encoderFallback.GetReplacementCharacter(), out);
        }
        else if (ASCII::IsASCII(ch))
        {
          out.push_back(static_cast<byte>(ch.Value));
        }
        else
        {
          out.push_back(byte {ch.Value - XUserDefinedStart + ASCII::ExtendedASCIIStart});
        }
      };

      Unicode::ForEachCodepoint(characters, EncodeCodepoint);
    }

    void Decode(Span<const byte> bytes, utf8_string &out) const noexcept override
    {
      Reserve(out, bytes.size());

      for (byte b : bytes)
      {
        if (ASCII::IsASCII(b))
        {
          out.push_back(static_cast<char8>(b));
        }
        else
        {
          Rune codepoint {XUserDefinedStart + (static_cast<uint8>(b) - ASCII::ExtendedASCIIStart)};
        }
      }
    }
  };
}