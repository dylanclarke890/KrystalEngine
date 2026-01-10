#pragma once

#include "Krystal.Text/Encodings/EncodingTables/IBM424HebrewBulletin.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The Hebrew encoding that matches IBM's Codepage 424.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_ibm_424_hebrew_bulletin
      : public ::Krys::Text::Impl::SingleByteLookupEncoding<
          basic_ibm_424_hebrew_bulletin<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::ibm_424_hebrew_bulletin_index_to_code_point,
          &::Krys::Text::EncodingTable::ibm_424_hebrew_bulletin_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief The Hebrew encoding that matches IBM's Codepage 424.
  constexpr inline basic_ibm_424_hebrew_bulletin<char> ibm_424_hebrew_bulletin = {};
}
