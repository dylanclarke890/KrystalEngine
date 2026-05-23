#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Windows865DOSNordic.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 865 (DOS Nordic).
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_865_dos_nordic
      : public SingleByteLookupEncoding<basic_windows_865_dos_nordic<TCodeUnit, TCodePoint>,
                                        &EncodingTable::Windows865DOSNordicIndexToCodePoint,
                                        &EncodingTable::Windows865DOSNordicCodePointToIndex, TCodeUnit,
                                        TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"windows-865"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 2> Aliases = {"cp865"_s, "cp-865"_s};
  };

  /// @brief The encoding that matches Microsoft Windows's Codepage 865 (DOS Nordic).
  constexpr inline basic_windows_865_dos_nordic<char> windows_865_dos_nordic = {};
}
