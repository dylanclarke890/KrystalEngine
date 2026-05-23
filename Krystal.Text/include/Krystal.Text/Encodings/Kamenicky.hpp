#pragma once

#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/kamenicky.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the KAMENICKY encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_kamenicky
      : public SingleByteLookupEncoding<basic_kamenicky<TCodeUnit, TCodePoint>,
                                        &EncodingTable::KamenickyIndexToCodePoint,
                                        &EncodingTable::KamenickyCodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = "KAMENICKY"_s;
  };

  /// @brief The encoding that matches the KAMENICKY encoding specification.
  constexpr inline basic_kamenicky<char> kamenicky = {};
}
