#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/ATASCII.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The ATASCII (Atari ASCII) Encoding.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_atascii
      : public SingleByteLookupEncoding<
          basic_atascii<TCodeUnit, TCodePoint>, &EncodingTable::ATASCIIIndexToCodePoint,
          &EncodingTable::ATASCIICodePointToIndex, TCodeUnit, TCodePoint>
  {
    static_assert((sizeof(typename basic_atascii::code_point) * CHAR_BIT) >= 22,
                  "The code point type for atascii must be at least 22 bits wide");

  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"ATASCII"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 2> Aliases = {"ATASCII"_s,
                                                                                    "Atari-ASCII"_s};
  };

  /// @brief The ATASCII (Atari ASCII) Encoding.
  constexpr inline basic_atascii<char> atascii = {};
}
