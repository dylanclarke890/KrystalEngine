#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Windows1258.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 1258.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_1258
      : public SingleASCIIByteHighBitLookupEncoding<
          basic_windows_1258<TCodeUnit, TCodePoint>, &EncodingTable::Windows1258IndexToCodePoint,
          &EncodingTable::windows_1258_code_point_to_index, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"windows-1258"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 3> Aliases = {"windows-1258"_s,
                                                                                    "cp1258"_s, "x-cp1258"_s};
  };

  /// @brief The encoding that matches Microsoft Windows's Codepage 1258.
  constexpr inline basic_windows_1258<char> windows_1258 = {};
}
