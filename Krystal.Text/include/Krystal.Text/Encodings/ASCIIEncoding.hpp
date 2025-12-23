#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"

namespace Krys::Text
{
  /// @brief Represents an ASCII character encoding.
  class ASCIIEncoding : public Encoding
  {
  public:
    ASCIIEncoding() noexcept
        : Encoding {"US-ASCII", EncoderFallback(EncodingReplacementCharacters::ASCII),
                    DecoderFallback(EncodingReplacementCharacters::UTF)}
    {
    }

    virtual ~ASCIIEncoding() noexcept = default;

    NO_DISCARD bool IsSingleByte() const noexcept override
    {
      return true;
    }

    NO_DISCARD List<byte> Encode(utf8_stringview characters) const noexcept override
    {
      List<byte> bytes;
      bytes.reserve(characters.size());

      const auto encodeASCII = [&](UnicodeScalar ch) noexcept
      {
        if (ch.Value > 127u)
        {
          // Use the encoder fallback replacement character
          for (char8_t rc : _encoderFallback.GetReplacementCharacter())
          {
            bytes.push_back(static_cast<byte>(rc));
          }
        }
        else
        {
          bytes.push_back(static_cast<byte>(ch.Value));
        }
      };

      ForEachUnicodeScalar(characters, encodeASCII);
      return bytes;
    }

    NO_DISCARD utf8_string Decode(Span<const byte> bytes) const noexcept override
    {
      utf8_string characters;
      characters.reserve(bytes.size());

      for (byte b : bytes)
      {
        uchar ch = static_cast<uchar>(b);
        if (ch > 127u)
        {
          characters += _decoderFallback.GetReplacementCharacter();
        }
        else
        {
          characters.push_back(ch);
        }
      }

      return characters;
    }
  };
}