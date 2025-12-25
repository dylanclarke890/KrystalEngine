#pragma once

#include "Krystal.Text/Encodings/Encoding.hpp"

namespace Krys::Text
{
  class Windows1257Encoding : public Encoding
  {
  public:
    static constexpr utf8_stringview IANA = u8"windows-1257";
    static constexpr EncodingInfo EncodingInformation = EncodingInfo {IANA, 1'257u};

  public:
    constexpr Windows1257Encoding() noexcept : Encoding(EncodingInformation)
    {
    }

    constexpr virtual ~Windows1257Encoding() noexcept = default;

    NO_DISCARD constexpr virtual bool IsSingleByte() const noexcept
    {
      return true;
    }

    NO_DISCARD constexpr virtual List<byte> Encode(utf8_stringview characters) const noexcept
    {
      List<byte> bytes;
      Encode(characters, bytes);
      return bytes;
    }

    constexpr virtual void Encode(utf8_stringview characters, List<byte> &out) const noexcept
    {
    }

    NO_DISCARD constexpr virtual utf8_string Decode(Span<const byte> bytes) const noexcept
    {
      utf8_string result;
      Decode(bytes, result);
      return result;
    }

    constexpr virtual void Decode(Span<const byte> bytes, utf8_string &out) const noexcept
    {
    }
  };
}