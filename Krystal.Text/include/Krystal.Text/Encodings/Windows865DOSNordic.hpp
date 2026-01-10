#pragma once

#include "Krystal.Text/Encodings/EncodingTables/Windows865DOSNordic.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 437 (DOS Latin, US).
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_865_dos_nordic
      : public ::Krys::Text::Impl::SingleByteLookupEncoding<
          basic_windows_865_dos_nordic<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::windows_865_dos_nordic_index_to_code_point,
          &::Krys::Text::EncodingTable::windows_865_dos_nordic_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief The encoding that matches Microsoft Windows's Codepage 437 (DOS Latin, US).
  constexpr inline basic_windows_865_dos_nordic<char> windows_865_dos_nordic = {};
}
