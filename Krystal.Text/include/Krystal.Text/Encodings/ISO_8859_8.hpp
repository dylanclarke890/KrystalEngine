#pragma once

#include "Krystal.Text/Encodings/EncodingTables/iso_8859_8.tables.hpp"
#include "Krystal.Text/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The encoding that matches the ISO/IEC 8859-8 encoding specification.
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_iso_8859_8
      : public Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_iso_8859_8<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::iso_8859_8_index_to_code_point,
          &::Krys::Text::EncodingTable::iso_8859_8_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief An instance of basic_iso_8859_8for ease of use.
  inline constexpr basic_iso_8859_8<char> iso_8859_8 = {};
}
