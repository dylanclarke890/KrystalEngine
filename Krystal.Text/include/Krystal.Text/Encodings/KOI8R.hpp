#pragma once

#include "Krystal.Text/Encodings/EncodingTables/KOI8R.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the KOI-8-R (Russian) Cyrillic encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_koi8_r
      : public ::Krys::Text::Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_koi8_r<TCodeUnit, TCodePoint>, &::Krys::Text::EncodingTable::koi8_r_index_to_code_point,
          &::Krys::Text::EncodingTable::koi8_r_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief The encoding that matches the KOI-8-R (Russian) Cyrillic encoding specification.
  constexpr inline basic_koi8_r<char> koi8_r = {};
}
