#pragma once

#include "Krystal.Text/Encodings/EncodingTables/IBM424HebrewBulletin.tables.hpp"
#include "Krystal.Text/Impl/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The Hebrew encoding that matches IBM's Codepage 424.
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_ibm_424_hebrew_bulletin
      : public Impl::SingleByteLookupEncoding<
          basic_ibm_424_hebrew_bulletin<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::ibm_424_hebrew_bulletin_index_to_code_point,
          &::Krys::Text::EncodingTable::ibm_424_hebrew_bulletin_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief An instance of basic_ibm_8_424ebrew for ease of use.
  inline constexpr basic_ibm_424_hebrew_bulletin<char> ibm_424_hebrew_bulletin = {};
}
