#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"
#include "Krystal.Text/Unicode.hpp"

namespace Krys
{
  class XUserDefinedEncoding : public Encoding
  {
  public:
    static constexpr utf8_stringview Name = u8"x-user-defined";
    static constexpr uint32 MIBenum = MIBenumUnknown;
    static constexpr uint32 WindowsCodePage = WindowsCodePageUnknown;

    static constexpr uint32 XUserDefinedStart = 0xF780u;
    static constexpr uint32 XUserDefinedEnd = 0xF7FFu;

  public:
    constexpr XUserDefinedEncoding() noexcept
        : Encoding({Name, MIBenum, WindowsCodePage}, EncoderFallback(EncodingReplacement_ASCII),
                   DecoderFallback(EncodingReplacement_UTF))
    {
    }

    constexpr virtual ~XUserDefinedEncoding() noexcept = default;

    KRYS_NODISCARD constexpr bool IsSingleByte() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD constexpr List<byte> Encode(utf8_stringview characters) const noexcept override
    {
      List<byte> bytes;
      Encode(characters, bytes);
      return bytes;
    }

    KRYS_NODISCARD constexpr utf8_string Decode(Span<const byte> bytes) const noexcept override
    {
      utf8_string characters;
      Decode(bytes, characters);
      return characters;
    }

    constexpr void Encode(utf8_stringview characters, List<byte> &out) const noexcept override
    {
      Reserve(out, characters.size());

      const auto EncodeCodepoint = [&](UnicodeCodepoint ch, bool wasInvalid) noexcept
      {
        if (wasInvalid || ch < XUserDefinedStart || ch > XUserDefinedEnd)
        {
          Encode(_encoderFallback.GetReplacementCharacter(), out);
        }
        else if (Unicode::IsASCIICharacter(ch))
        {
          out.push_back(static_cast<byte>(ch.Value));
        }
        else
        {
          out.push_back(byte {ch.Value - XUserDefinedStart + Unicode::ExtendedASCIIStart});
        }
      };

      Unicode::ForEachCodepoint(characters, EncodeCodepoint);
    }

    constexpr void Decode(Span<const byte> bytes, utf8_string &out) const noexcept override
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
          UnicodeCodepoint codepoint {XUserDefinedStart
                                      + (static_cast<uint8>(b) - Unicode::ExtendedASCIIStart)};
        }
      }
    }
  };
}