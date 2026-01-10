#pragma once

#include "Krystal.Text/Encodings/EncodingTables/TatarANSI.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the TATAR encoding (ANSI variant, Based on Windows Code Page 1252.)
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_tatar_ansi
      : public ::Krys::Text::Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_tatar_ansi<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::tatar_ansi_index_to_code_point,
          &::Krys::Text::EncodingTable::tatar_ansi_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief The encoding that matches the TATAR encoding (ANSI variant, Based on Windows Code Page 1252.)
  constexpr inline basic_tatar_ansi<char> tatar_ansi = {};
}
