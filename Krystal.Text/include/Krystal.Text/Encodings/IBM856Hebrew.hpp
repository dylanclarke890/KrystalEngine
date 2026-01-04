#pragma once

#include "Krystal.Text/Impl/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"
#include "Krystal.Text/Encodings/EncodingTables/IBM856Hebrew.tables.hpp"

namespace Krys
{
  /// @brief The Hebrew encoding that matches IBM's Codepage 856.
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_ibm_856_hebrew
      : public Impl::SingleByteLookupEncoding<
          basic_ibm_856_hebrew<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::ibm_856_hebrew_index_to_code_point,
          &::Krys::Text::EncodingTable::ibm_856_hebrew_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief An instance of basic_ibm_856_hebrew for ease of use.
  constexpr inline basic_ibm_856_hebrew<char> ibm_856_hebrew = {};
}
