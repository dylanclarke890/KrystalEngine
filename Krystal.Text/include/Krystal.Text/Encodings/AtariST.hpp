#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/EncodingTables/AtariST.hpp"
#include "Krystal.Text/Encodings/Impl/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The Atari ST Encoding.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_atari_st
      : public ::Krys::Text::Impl::SingleByteLookupEncoding<
          basic_atari_st<TCodeUnit, TCodePoint>, &::Krys::Text::EncodingTable::atari_st_index_to_code_point,
          &::Krys::Text::EncodingTable::atari_st_code_point_to_index, TCodeUnit, TCodePoint>
  {
    static_assert((sizeof(typename basic_atari_st::code_point) * CHAR_BIT) >= 22,
                  "The code point type for the Atari ST encoding must be at least 22 bits wide");

  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"AtariST"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 3> Aliases = {"atari"_s, "AtariST"_s,
                                                                                    "Atari_ST"_s};
  };

  /// @brief The Atari ST Encoding.
  constexpr inline basic_atari_st<char> atari_st = {};
}
