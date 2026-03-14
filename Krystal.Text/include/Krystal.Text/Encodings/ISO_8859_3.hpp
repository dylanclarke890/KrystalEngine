#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/iso_8859_3.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the ISO/IEC 8859-3 encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_iso_8859_3
      : public SingleASCIIByteHighBitLookupEncoding<
          basic_iso_8859_3<TCodeUnit, TCodePoint>, &EncodingTable::ISO_8859_3IndexToCodePoint,
          &EncodingTable::ISO_8859_3CodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"ISO-8859-3"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 9> Aliases = {
      "csisolatin3"_s, "iso-8859-3"_s,      "iso-ir-109"_s, "iso8859-3"_s, "iso88593"_s,
      "iso_8859-3"_s,  "iso_8859-3:1988"_s, "l3"_s,         "latin3"_s};
  };

  /// @brief The encoding that matches the ISO/IEC 8859-3 encoding specification.
  constexpr inline basic_iso_8859_3<char> iso_8859_3 = {};
}
