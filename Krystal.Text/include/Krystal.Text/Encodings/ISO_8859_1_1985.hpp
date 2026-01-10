#pragma once

#include "Krystal.Text/Encodings/EncodingTables/iso_8859_1_1985.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches ISO/IEC 8859-1 specification published in 1985.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_iso_8859_1_1985
      : public ::Krys::Text::Impl::SingleByteLookupEncoding<
          basic_iso_8859_1_1985<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::iso_8859_1_1985_index_to_code_point,
          &::Krys::Text::EncodingTable::iso_8859_1_1985_code_point_to_index, TCodeUnit, TCodePoint>
  {
  private:
    static_assert(((sizeof(TCodePoint) * CHAR_BIT) > 15),
                  "The code point type for ISO 8859-1 (1985) must be at least 16 bits wide");
  };

  /// @brief The encoding that matches ISO/IEC 8859-1 specification published in 1985.
  constexpr inline basic_iso_8859_1_1985<char> iso_8859_1_1985 = {};
}
