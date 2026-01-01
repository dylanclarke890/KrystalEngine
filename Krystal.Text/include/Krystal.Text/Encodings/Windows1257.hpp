#pragma once

#include "Krystal.Text/Encodings/EncodingTables/Windows1257.tables.hpp"
#include "Krystal.Text/Impl/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 1257.
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_1257
      : public Impl::SingleASCIIByteHighBitLookupEncoding<
          basic_windows_1257<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::windows_1257_index_to_code_point,
          &::Krys::Text::EncodingTable::windows_1257_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief An instance of basic_windows_1257 for ease of use.
  inline constexpr basic_windows_1257<char> windows_1257 = {};
}
