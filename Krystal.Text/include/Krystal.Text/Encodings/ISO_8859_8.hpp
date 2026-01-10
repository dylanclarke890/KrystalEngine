#pragma once

#include "Krystal.Text/Encodings/EncodingTables/iso_8859_8.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the ISO/IEC 8859-8 encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_iso_8859_8
      : public ::Krys::Text::Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_iso_8859_8<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::iso_8859_8_index_to_code_point,
          &::Krys::Text::EncodingTable::iso_8859_8_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief The encoding that matches the ISO/IEC 8859-8 encoding specification.
  constexpr inline basic_iso_8859_8<char> iso_8859_8 = {};
}
