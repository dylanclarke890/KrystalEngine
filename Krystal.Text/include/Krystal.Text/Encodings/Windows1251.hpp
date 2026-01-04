#pragma once

#include "Krystal.Text/Encodings/EncodingTables/Windows1251.tables.hpp"
#include "Krystal.Text/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 1251.
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_1251
      : public Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_windows_1251<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::windows_1251_index_to_code_point,
          &::Krys::Text::EncodingTable::windows_1251_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief An instance of windows_1251 for ease of use.
  constexpr inline basic_windows_1251<char> windows_1251 = {};
}
