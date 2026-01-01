#pragma once

#include "Krystal.Text/Encodings/EncodingTables/Windows865DOSNordic.tables.hpp"
#include "Krystal.Text/Impl/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 437 (DOS Latin, US).
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_865_dos_nordic
      : public Impl::SingleByteLookupEncoding<
          basic_windows_865_dos_nordic<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::windows_865_dos_nordic_index_to_code_point,
          &::Krys::Text::EncodingTable::windows_865_dos_nordic_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief An instance of basic_windows_865_dos_nordic for ease of use.
  inline constexpr basic_windows_865_dos_nordic<char> windows_865_dos_nordic = {};
}
