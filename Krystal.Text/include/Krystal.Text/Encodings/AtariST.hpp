#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/AtariST.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_atari_st
      : public SingleByteLookupEncoding<basic_atari_st<TCodeUnit, TCodePoint>,
                                        &EncodingTable::AtariSTIndexToCodePoint,
                                        &EncodingTable::AtariSTCodePointToIndex, TCodeUnit, TCodePoint>
  {
    static_assert((sizeof(typename basic_atari_st::code_point) * CHAR_BIT) >= 22,
                  "The code point type for the Atari ST encoding must be at least 22 bits wide");

  public:
    constexpr static inline ASCIILiteral Name = {"AtariST"_s};
    constexpr static inline Array<ASCIILiteral, 3> Aliases = {"atari"_s, "AtariST"_s, "Atari_ST"_s};
  };

  /// @brief The Atari ST Encoding.
  constexpr inline basic_atari_st<char> atari_st = {};
}
