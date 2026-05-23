#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Windows1257.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 1257.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_1257
      : public SingleASCIIByteHighBitLookupEncoding<
          basic_windows_1257<TCodeUnit, TCodePoint>, &EncodingTable::Windows1257IndexToCodePoint,
          &EncodingTable::Windows1257CodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"windows-1257"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 3> Aliases = {"windows-1257"_s,
                                                                                    "cp1257"_s, "x-cp1257"_s};
  };

  /// @brief The encoding that matches Microsoft Windows's Codepage 1257.
  constexpr inline basic_windows_1257<char> windows_1257 = {};
}
