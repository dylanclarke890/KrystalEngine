#pragma once

#include "Krystal.Text/Encodings/EncodingTables/iso_8859_7.tables.hpp"
#include "Krystal.Text/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The encoding that matches the ISO/IEC 8859-7 encoding specification.
  ///
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_iso_8859_7
      : public Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_iso_8859_7<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::iso_8859_7_index_to_code_point,
          &::Krys::Text::EncodingTable::iso_8859_7_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief An instance of basic_iso_8859_7 for ease of use.
  inline constexpr basic_iso_8859_7<char> iso_8859_7 = {};
}
