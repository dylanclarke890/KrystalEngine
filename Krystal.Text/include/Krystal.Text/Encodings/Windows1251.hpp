#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Windows1251.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"
namespace Krys::Text
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 1251.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_1251
      : public SingleASCIIByteHighBitLookupEncoding<
          basic_windows_1251<TCodeUnit, TCodePoint>, &EncodingTable::Windows1251IndexToCodePoint,
          &EncodingTable::Windows1251CodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"windows-1251"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 3> Aliases = {"windows-1251"_s,
                                                                                    "cp1251"_s, "x-cp1251"_s};
  };

  /// @brief The encoding that matches Microsoft Windows's Codepage 1251.
  constexpr inline basic_windows_1251<char> windows_1251 = {};
}
