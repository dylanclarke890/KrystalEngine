#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/EncodingTables/KazakhSTRK1048.tables.hpp"
#include "Krystal.Text/Encodings/Impl/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the KZ1048 / Kazakh STRK-1048 encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_kazakh_strk1048
      : public ::Krys::Text::Impl::SingleByteLookupEncoding<
          basic_kazakh_strk1048<TCodeUnit, TCodePoint>,
          &::Krys::Text::EncodingTable::kazakh_strk1048_index_to_code_point,
          &::Krys::Text::EncodingTable::kazakh_strk1048_code_point_to_index, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = "KZ1048"_s;
  };

  /// @brief The encoding that matches the KZ1048 / Kazakh STRK-1048 encoding specification.
  constexpr inline basic_kazakh_strk1048<char> kazakh_strk1048 = {};
}
