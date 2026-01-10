#pragma once

#include "Krystal.Text/Encodings/EncodingTables/TatarASCII.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the TATAR encoding specification ("ASCII" variant, IBM Code Page 866).
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_tatar_ascii
      : public ::Krys::Text::Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_tatar_ascii<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::tatar_ascii_index_to_code_point,
          &::Krys::Text::EncodingTable::tatar_ascii_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief The encoding that matches the TATAR encoding specification ("ASCII" variant, IBM Code Page 866).
  constexpr inline basic_tatar_ascii<char> tatar_ascii = {};
}
