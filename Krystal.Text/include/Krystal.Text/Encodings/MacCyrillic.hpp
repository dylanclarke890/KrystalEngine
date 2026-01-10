#pragma once

#include "Krystal.Text/Encodings/EncodingTables/MacCyrillic.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the Macintosh Cyrillic encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_mac_cyrillic
      : public ::Krys::Text::Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_mac_cyrillic<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::mac_cyrillic_index_to_code_point,
          &::Krys::Text::EncodingTable::mac_cyrillic_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief The encoding that matches the Macintosh Cyrillic encoding specification.
  constexpr inline basic_mac_cyrillic<char> mac_cyrillic = {};
}
