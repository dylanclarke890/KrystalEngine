#pragma once

#include "Krystal.Text/Encodings/EncodingTables/TatarANSI.tables.hpp"
#include "Krystal.Text/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The encoding that matches the TATAR encoding (it's ANSI variant, based on Windows Code Page
  /// 1252.)
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_tatar_ansi
      : public Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_tatar_ansi<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::tatar_ansi_index_to_code_point,
          &::Krys::Text::EncodingTable::tatar_ansi_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief An instance of basic_tatar_ansi for ease of use.
  inline constexpr basic_tatar_ansi<char> tatar_ansi = {};
}
