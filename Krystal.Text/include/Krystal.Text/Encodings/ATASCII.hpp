#pragma once

#include "Krystal.Text/Encodings/EncodingTables/ATASCII.hpp"
#include "Krystal.Text/Encodings/Impl/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The ATASCII (Atari ASCII) Encoding.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_atascii
      : public ::Krys::Text::Impl::SingleByteLookupEncoding<
          basic_atascii<TCodeUnit, TCodePoint>, &::Krys::Text::EncodingTable::atascii_index_to_code_point,
          &::Krys::Text::EncodingTable::atascii_code_point_to_index, TCodeUnit, TCodePoint>
  {
    static_assert((sizeof(typename basic_atascii::code_point) * CHAR_BIT) >= 22,
                  "The code point type for atascii must be at least 22 bits wide");
  };

  /// @brief The ATASCII (Atari ASCII) Encoding.
  constexpr inline basic_atascii<char> atascii = {};
}
