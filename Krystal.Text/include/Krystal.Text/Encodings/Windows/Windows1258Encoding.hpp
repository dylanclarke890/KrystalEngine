#pragma once

#include "Krystal.Text/Encodings/Encoding.hpp"

namespace Krys::Text
{
  /// @brief Represents a character encoding.
  class Windows1258Encoding : public Encoding
  {
  public:
    static constexpr utf8_stringview IANA = u8"windows-1258";
    static constexpr EncodingInfo EncodingInformation = EncodingInfo {IANA, 1'258u};

  public:
    constexpr Windows1258Encoding() noexcept : Encoding(EncodingInformation)
    {
    }

    constexpr virtual ~Windows1258Encoding() noexcept = default;

    /// @brief Indicates whether the encoding uses single-byte characters.
    NO_DISCARD constexpr virtual bool IsSingleByte() const noexcept
    {
      return true;
    }

    /// @brief Encodes a UTF-8 string into a sequence of bytes in the target encoding.
    NO_DISCARD constexpr virtual List<byte> Encode(utf8_stringview characters) const noexcept
    {
      List<byte> bytes;
      Encode(characters, bytes);
      return bytes;
    }

    /// @brief Encodes a UTF-8 string into a sequence of bytes in the target encoding.
    constexpr virtual void Encode(utf8_stringview characters, List<byte> &out) const noexcept
    {
    }

    /// @brief Decodes a sequence of bytes into a UTF-8 string.
    NO_DISCARD constexpr virtual utf8_string Decode(Span<const byte> bytes) const noexcept
    {
      utf8_string result;
      Decode(bytes, result);
      return result;
    }

    /// @brief Decodes a sequence of bytes into a UTF-8 string.
    constexpr virtual void Decode(Span<const byte> bytes, utf8_string &out) const noexcept
    {
    }
  };
}