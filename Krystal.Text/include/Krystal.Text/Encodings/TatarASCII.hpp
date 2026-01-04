#pragma once

#include "Krystal.Text/Encodings/EncodingTables/TatarASCII.tables.hpp"
#include "Krystal.Text/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The encoding that matches the TATAR encoding specification (based on its "ASCII" variant, IBM
  /// Code Page 866).
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_tatar_ascii
      : public Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_tatar_ascii<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::tatar_ascii_index_to_code_point,
          &::Krys::Text::EncodingTable::tatar_ascii_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief An instance of basic_tatar_ascii for ease of use.
  constexpr inline basic_tatar_ascii<char> tatar_ascii = {};
}
