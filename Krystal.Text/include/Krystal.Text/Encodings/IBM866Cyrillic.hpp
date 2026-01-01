#pragma once

#include "Krystal.Text/Encodings/EncodingTables/IBM866Cyrillic.tables.hpp"
#include "Krystal.Text/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The Cyrillic encoding that matches IBM's Codepage 866.
  ///
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_ibm_866_cyrillic
      : public Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_ibm_866_cyrillic<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::ibm_866_cyrillic_index_to_code_point,
          &::Krys::Text::EncodingTable::ibm_866_cyrillic_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief An instance of basic_ibm_866_cyrillic for ease of use.
  inline constexpr basic_ibm_866_cyrillic<char> ibm_866_cyrillic = {};
}
