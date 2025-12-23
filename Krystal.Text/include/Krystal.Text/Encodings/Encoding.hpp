#pragma once

#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Span.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Text/Encodings/EncodingFallback.hpp"
#include "Krystal.Text/Encodings/EncodingReplacementCharacters.hpp"
#include "Krystal.Text/UnicodeScalar.hpp"

namespace Krys::Text
{
  /// @brief Represents a character encoding.
  class Encoding
  {
  protected:
    string _ianaName;
    EncoderFallback _encoderFallback;
    DecoderFallback _decoderFallback;

    explicit Encoding(const string &name) noexcept
        : _ianaName(name), _encoderFallback(EncodingReplacementCharacters::UTF),
          _decoderFallback(EncodingReplacementCharacters::UTF)
    {
    }

    Encoding(const string &name, EncoderFallback encoderFallback, DecoderFallback decoderFallback) noexcept
        : _ianaName(name), _encoderFallback(encoderFallback), _decoderFallback(decoderFallback)
    {
    }

  public:
    virtual ~Encoding() noexcept = default;

    /// @brief Returns the IANA name of the encoding.
    NO_DISCARD const string &GetIANAName() const noexcept
    {
      return _ianaName;
    }

    /// @brief Returns the encoder fallback that should be used to replace invalid byte sequences.
    NO_DISCARD EncoderFallback GetEncoderReplacementFallback() const noexcept
    {
      return _encoderFallback;
    }

    /// @brief Returns a span of bytes that should be used to replace invalid character sequences.
    NO_DISCARD DecoderFallback GetDecoderReplacementFallback() const noexcept
    {
      return _decoderFallback;
    }

    /// @brief Returns a sequence of bytes that specifies the encoding used.
    NO_DISCARD virtual List<byte> GetPreamble() const noexcept
    {
      return {};
    }

    /// @brief Indicates whether the encoding uses single-byte characters.
    NO_DISCARD virtual bool IsSingleByte() const noexcept
    {
      return false;
    }

    /// @brief Encodes a string into a sequence of bytes.
    NO_DISCARD virtual List<byte> Encode(utf8_stringview characters) const noexcept = 0;

    /// @brief Decodes a sequence of bytes into a utf-8 string.
    NO_DISCARD virtual utf8_string Decode(Span<const byte> bytes) const noexcept = 0;
  };

  template <typename TFunc>
  void ForEachUnicodeScalar(utf8_stringview utf8, TFunc &&func) noexcept
  {
    const auto ASCII_MAX = 0x7F;
    const auto REPLACEMENT_CHARACTER = UnicodeScalar(0xFFFD);

    auto it = utf8.begin();
    const auto end = utf8.end();
    while (it != end)
    {
      uint8 first = static_cast<uint8>(*it++);
      uint32 codepoint = 0;
      size_t needed = 0;

      // Single-byte (ASCII)
      if (first <= ASCII_MAX)
      {
        func(UnicodeScalar(first));
        continue;
      }
      // Multi-byte sequence
      else if ((first & 0xE0) == 0xC0)
      {
        codepoint = first & 0x1F;
        needed = 1;
      }
      else if ((first & 0xF0) == 0xE0)
      {
        codepoint = first & 0x0F;
        needed = 2;
      }
      else if ((first & 0xF8) == 0xF0)
      {
        codepoint = first & 0x07;
        needed = 3;
      }
      // Invalid leading byte
      else
      {
        func(REPLACEMENT_CHARACTER);
        continue;
      }

      if (static_cast<size_t>(end - it) < needed)
      {
        func(REPLACEMENT_CHARACTER);
        break;
      }

      // Process continuation bytes
      for (size_t i = 0; i < needed; i++)
      {
        uint8 b = static_cast<uint8>(*it);
        if ((b & 0xC0) != 0x80)
        {
          func(REPLACEMENT_CHARACTER);
          // TODO: test this case
          it++; // sync to next byte
          continue;
        }

        // Append bits
        codepoint = (codepoint << 6) | (b & 0x3F);
        it++;
      }

      // Reject overlong encodings
      bool invalidTwoByte = (needed == 1 && codepoint < 0x80);
      bool invalidThreeByte = (needed == 2 && codepoint < 0x800);
      bool invalidFourByte = (needed == 3 && codepoint < 0x10000);
      bool invalidCodepoint = (codepoint > 0x10FFFF);
      bool invalidSurrogate = (codepoint >= 0xD800 && codepoint <= 0xDFFF);
      if (invalidTwoByte || invalidThreeByte || invalidFourByte || invalidCodepoint || invalidSurrogate)
      {
        func(REPLACEMENT_CHARACTER);
        continue;
      }

      // Valid codepoint
      func(UnicodeScalar(codepoint));
    }
  }
}