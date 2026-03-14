#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/KazakhSTRK1048.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches the KZ1048 / Kazakh STRK-1048 encoding specification.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_kazakh_strk1048
      : public SingleByteLookupEncoding<basic_kazakh_strk1048<TCodeUnit, TCodePoint>,
                                        &EncodingTable::KazakhSTRK1048IndexToCodePoint,
                                        &EncodingTable::KazakhSTRK1048CodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = "KZ1048"_s;
  };

  /// @brief The encoding that matches the KZ1048 / Kazakh STRK-1048 encoding specification.
  constexpr inline basic_kazakh_strk1048<char> kazakh_strk1048 = {};
}
