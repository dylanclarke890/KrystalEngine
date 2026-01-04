#pragma once

#include "Krystal.Text/Encodings/EncodingTables/MacCyrillic.tables.hpp"
#include "Krystal.Text/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The encoding that matches the Macintosh Cyrillic encoding specification.
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_mac_cyrillic
      : public Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_mac_cyrillic<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::mac_cyrillic_index_to_code_point,
          &::Krys::Text::EncodingTable::mac_cyrillic_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief An instance of basic_mac_cyrillic for ease of use.
  constexpr inline basic_mac_cyrillic<char> mac_cyrillic = {};
}
