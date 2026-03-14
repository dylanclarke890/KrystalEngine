#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Windows1256.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 1256.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_1256
      : public SingleASCIIByteHighBitLookupEncoding<
          basic_windows_1256<TCodeUnit, TCodePoint>, &EncodingTable::Windows1256IndexToCodePoint,
          &EncodingTable::Windows1256CodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"windows-1256"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 3> Aliases = {"windows-1256"_s,
                                                                                    "cp1256"_s, "x-cp1256"_s};
  };

  /// @brief The encoding that matches Microsoft Windows's Codepage 1256.
  constexpr inline basic_windows_1256<char> windows_1256 = {};
}
