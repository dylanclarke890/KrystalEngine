#pragma once

#include "Krystal.Text/Encodings/EncodingTables/iso_8859_10.tables.hpp"
#include "Krystal.Text/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The encoding that matches the ISO/IEC 8859-10 encoding specification.
  ///
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_iso_8859_10
      : public Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_iso_8859_10<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::iso_8859_10_index_to_code_point,
          &::Krys::Text::EncodingTable::iso_8859_10_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief An instance of basic_iso_8859_10 for ease of use.
  constexpr inline basic_iso_8859_10<char> iso_8859_10 = {};
}
