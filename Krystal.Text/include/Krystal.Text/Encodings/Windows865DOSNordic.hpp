#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Windows865DOSNordic.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 865 (DOS Nordic).
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_865_dos_nordic
      : public ::Krys::Text::Impl::SingleByteLookupEncoding<
          basic_windows_865_dos_nordic<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::windows_865_dos_nordic_index_to_code_point,
          &::Krys::Text::EncodingTable::windows_865_dos_nordic_code_point_to_index, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"windows-865"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 2> Aliases = {"cp865"_s, "cp-865"_s};
  };

  /// @brief The encoding that matches Microsoft Windows's Codepage 865 (DOS Nordic).
  constexpr inline basic_windows_865_dos_nordic<char> windows_865_dos_nordic = {};
}
