#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/EncodingTables/iso_8859_8.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the ISO/IEC 8859-8 encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_iso_8859_8
      : public ::Krys::Text::Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_iso_8859_8<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::iso_8859_8_index_to_code_point,
          &::Krys::Text::EncodingTable::iso_8859_8_code_point_to_index, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = "ISO-8859-8"_s;
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 12> Aliases = {
      "csiso88598e"_s,  "csisolatinhebrew"_s, "hebrew"_s,    "iso-8859-8"_s,
      "iso-8859-8-e"_s, "iso-ir-138"_s,       "iso8859-8"_s, "iso88598"_s,
      "iso_8859-8"_s,   "iso_8859-8:1988"_s,  "visual"_s};
  };

  /// @brief The encoding that matches the ISO/IEC 8859-8 encoding specification.
  constexpr inline basic_iso_8859_8<char> iso_8859_8 = {};
}
