#pragma once

#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/EncodingTables/kamenicky.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the KAMENICKY encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_kamenicky
      : public ::Krys::Text::Impl::SingleByteLookupEncoding<
          basic_kamenicky<TCodeUnit, TCodePoint>, &::Krys::Text::EncodingTable::kamenicky_index_to_code_point,
          &::Krys::Text::EncodingTable::kamenicky_code_point_to_index, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = "KAMENICKY"_s;
  };

  /// @brief The encoding that matches the KAMENICKY encoding specification.
  constexpr inline basic_kamenicky<char> kamenicky = {};
}
