#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Windows1254.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 1254.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_1254
      : public SingleASCIIByteHighBitLookupEncoding<
          basic_windows_1254<TCodeUnit, TCodePoint>, &EncodingTable::Windows1254IndexToCodePoint,
          &EncodingTable::Windows1254CodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"windows-1254"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 12> Aliases = {
      "x-cp1254"_s, "windows-1254"_s, "csisolatin5"_s,     "iso-8859-9"_s, "iso-ir-148"_s, "iso8859-9"_s,
      "iso88599"_s, "iso_8859-9"_s,   "iso_8859-9:1989"_s, "l5"_s,         "latin5"_s,     "cp1254"_s,
    };
  };

  /// @brief The encoding that matches Microsoft Windows's Codepage 1254.
  constexpr inline basic_windows_1254<char> windows_1254 = {};
}
