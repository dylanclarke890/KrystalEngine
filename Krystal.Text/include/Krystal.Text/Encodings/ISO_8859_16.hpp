#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/EncodingTables/iso_8859_16.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the ISO/IEC 8859-116 encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_iso_8859_16
      : public ::Krys::Text::Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_iso_8859_16<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::iso_8859_16_index_to_code_point,
          &::Krys::Text::EncodingTable::iso_8859_16_code_point_to_index, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"ISO-8859-16"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 1> Aliases = {"iso-8859-16"_s};
  };

  /// @brief The encoding that matches the ISO/IEC 8859-116 encoding specification.
  constexpr inline basic_iso_8859_16<char> iso_8859_16 = {};
}
