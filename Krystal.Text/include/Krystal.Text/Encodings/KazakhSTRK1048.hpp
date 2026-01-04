#pragma once

#include "Krystal.Text/Encodings/EncodingTables/KazakhSTRK1048.tables.hpp"
#include "Krystal.Text/Impl/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The encoding that matches the KZ1048 / Kazakh STRK-1048 encoding specification.
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_kazakh_strk1048
      : public Impl::SingleByteLookupEncoding<
          basic_kazakh_strk1048<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::kazakh_strk1048_index_to_code_point,
          &::Krys::Text::EncodingTable::kazakh_strk1048_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief An instance of basic_kazakh_strk1048 for ease of use.
  constexpr inline basic_kazakh_strk1048<char> kazakh_strk1048 = {};
}
