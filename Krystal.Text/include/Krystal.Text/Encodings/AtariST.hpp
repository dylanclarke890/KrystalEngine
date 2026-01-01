#pragma once

#include "Krystal.Text/Encodings/EncodingTables/AtariST.hpp"
#include "Krystal.Text/Impl/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The Atari ST Encoding.
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_atari_st
      : public Impl::SingleByteLookupEncoding<
          basic_atari_st<TCodeUnit, TCodePoint>, &::Krys::Text::EncodingTable::atari_st_index_to_code_point,
          &::Krys::Text::EncodingTable::atari_st_code_point_to_index, TCodeUnit, TCodePoint>
  {
  private:
    static_assert((sizeof(typename basic_atari_st::code_point) * CHAR_BIT) > 21,
                  "The code point type for the Atari ST encoding must be at least 22 bits wide");
  };

  /// @brief An instance of basic_atari_st for ease of use.
  constexpr inline basic_atari_st<char> atari_st = {};
}
