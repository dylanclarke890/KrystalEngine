#pragma once

#include "Krystal.Text/Encodings/EncodingTables/KOI8R.tables.hpp"
#include "Krystal.Text/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The encoding that matches the KOI-8-R (Russian) Cyrillic encoding specification.
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_koi8_r
      : public Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_koi8_r<TCodeUnit, TCodePoint>, &::Krys::Text::EncodingTable::koi8_r_index_to_code_point,
          &::Krys::Text::EncodingTable::koi8_r_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief An instance of basic_koi8_r for ease of use.
  inline constexpr basic_koi8_r<char> koi8_r = {};
}
