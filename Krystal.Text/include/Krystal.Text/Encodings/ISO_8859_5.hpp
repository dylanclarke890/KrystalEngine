#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/iso_8859_5.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the ISO/IEC 8859-5 encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_iso_8859_5
      : public SingleASCIIByteHighBitLookupEncoding<
          basic_iso_8859_5<TCodeUnit, TCodePoint>, &EncodingTable::ISO_8859_5IndexToCodePoint,
          &EncodingTable::ISO_8859_5CodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"ISO-8859-5"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 8> Aliases = {
      "csisolatincyrillic"_s, "cyrillic"_s, "iso-8859-5"_s, "iso-ir-144"_s,
      "iso8859-5"_s,          "iso88595"_s, "iso_8859-5"_s, "iso_8859-5:1988"_s};
  };

  /// @brief The encoding that matches the ISO/IEC 8859-5 encoding specification.
  constexpr inline basic_iso_8859_5<char> iso_8859_5 = {};
}
