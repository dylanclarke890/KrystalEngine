#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Windows437DOSLatinUS.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 437 (DOS Latin, US).
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_437_dos_latin_us
      : public SingleByteLookupEncoding<basic_windows_437_dos_latin_us<TCodeUnit, TCodePoint>,
                                        &EncodingTable::Windows437DOSLatinUSIndexToCodePoint,
                                        &EncodingTable::Windows437DOSLatinUSCodePointToIndex, TCodeUnit,
                                        TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"windows-437"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 3> Aliases = {"cp437"_s, "cp-437"_s,
                                                                                    "ibm437"_s};
  };

  /// @brief The encoding that matches Microsoft Windows's Codepage 437 (DOS Latin, US).
  constexpr inline basic_windows_437_dos_latin_us<char> windows_437_dos_latin_us = {};
}
