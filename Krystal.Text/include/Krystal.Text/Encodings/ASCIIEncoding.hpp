#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"
#include "Krystal.Text/Unicode.hpp"

namespace Krys::Text
{
  class ASCIIEncoding : public Encoding
  {
  public:
    static constexpr utf8_stringview IANA = u8"US-ASCII";
    static constexpr EncodingInfo EncodingInformation {IANA, 20'127u};

  public:
    constexpr ASCIIEncoding() noexcept
        : Encoding(EncodingInformation, EncoderFallback(EncodingReplacement_ASCII),
                   DecoderFallback(EncodingReplacement_UTF))
    {
    }

    constexpr virtual ~ASCIIEncoding() noexcept = default;

    NO_DISCARD constexpr bool IsSingleByte() const noexcept override
    {
      return true;
    }

    constexpr void Encode(utf8_stringview characters, List<byte> &out) const noexcept override
    {
      Reserve(out, characters.size());

      const auto EncodeASCII = [&](UnicodeCodepoint ch, bool wasInvalid) noexcept
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

      Unicode::ForEachCodepoint(characters, EncodeASCII);
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