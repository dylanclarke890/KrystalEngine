#pragma once

#include "Krystal.Core/Text/Encodings/Base.hpp"

namespace krys::text
{
  /// @brief The American Standard Code for Information Exchange (ASCII) Encoding.
  struct ASCII : public DefaultEncodingTraits
  {
    constexpr static EncodingId Id = EncodingId::ASCII;
    constexpr static utf8_stringview Name = u8"US-ASCII";
    constexpr static Array<utf8_stringview, 10uz> Aliases = {
      u8"ASCII",    u8"ANSI_X3.4-1968", u8"ANSI_X3.4-1986", u8"ISO_646.irv:1991", u8"ISO646-US", u8"us",
      u8"iso-ir-6", u8"IBM367",         u8"cp367",          u8"csASCII"};

    template <CodeUnitInputIterator<ASCII> TIterator>
    KRYS_NODISCARD constexpr static DecodeOneResult<ASCII> DecodeOne(TIterator current,
                                                                     TIterator end) noexcept
    {
      krys_debug_assert(current != end);

      char32 codeUnit = static_cast<char32>(*current);
      if (codeUnit > MaxASCIIChar)
      {
        return DecodeOneInvalidSequenceError<ASCII>();
      }

      return DecodeOneSuccess<ASCII>(codeUnit);
    }

    KRYS_NODISCARD constexpr static EncodeOneResult<ASCII> EncodeOne(char32 codePoint) noexcept
    {
      if (codePoint > MaxASCIIChar)
      {
        return EncodeOneUnmappableCodePointError<ASCII>();
      }

      return EncodeOneSuccess<ASCII>(static_cast<code_unit>(codePoint));
    }
  };

  static_assert(Encoding<ASCII>);
}