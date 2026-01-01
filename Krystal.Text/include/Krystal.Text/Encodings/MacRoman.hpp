#pragma once

#include "Krystal.Text/Encodings/EncodingTables/MacRoman.tables.hpp"
#include "Krystal.Text/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The encoding that matches the Macintosh Roman encoding specification.
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_mac_roman
      : public Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_mac_roman<TCodeUnit, TCodePoint>, &::Krys::Text::EncodingTable::mac_roman_index_to_code_point,
          &::Krys::Text::EncodingTable::mac_roman_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief An instance of basic_mac_roman for ease of use.
  inline constexpr basic_mac_roman<char> mac_roman = {};
}
