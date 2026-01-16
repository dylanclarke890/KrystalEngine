#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/EncodingTables/KOI8U.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the KOI-8 (Ukranian) encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_koi8_u
      : public ::Krys::Text::Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_koi8_u<TCodeUnit, TCodePoint>, &::Krys::Text::EncodingTable::koi8_u_index_to_code_point,
          &::Krys::Text::EncodingTable::koi8_u_code_point_to_index, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"KOI8-U"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 2> Aliases = {"koi8-u"_s, "koi8-ru"_s};
  };

  /// @brief The encoding that matches the KOI-8 (Ukranian) encoding specification.
  constexpr inline basic_koi8_u<char> koi8_u = {};
}
