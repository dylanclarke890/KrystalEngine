#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"
#include "Krystal.Text/Unicode.hpp"

namespace Krys::Text
{
  class ASCIIEncoding : public Encoding
  {
  public:
    static constexpr utf8_stringview Name = u8"US-ASCII";
    static constexpr uint32 MIBenum = 3u;
    static constexpr uint32 WindowsCodePage = 20'127u;

  public:
    constexpr ASCIIEncoding() noexcept
        : Encoding({Name, MIBenum, WindowsCodePage}, EncoderFallback(EncodingReplacement_ASCII),
                   DecoderFallback(EncodingReplacement_UTF))
    {
    }

    constexpr virtual ~ASCIIEncoding() noexcept = default;

    NO_DISCARD constexpr bool IsSingleByte() const noexcept override
    {
      return true;
    }

    NO_DISCARD constexpr List<byte> Encode(utf8_stringview characters) const noexcept override
    {
      List<byte> bytes;
      Encode(characters, bytes);
      return bytes;
    }

    NO_DISCARD constexpr utf8_string Decode(Span<const byte> bytes) const noexcept override
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
          out += _decoderFallback.GetReplacementCharacter();
        }
      }
    }
  };
}