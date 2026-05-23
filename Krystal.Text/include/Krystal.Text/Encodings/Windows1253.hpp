#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/SingleASCIIByteHighBitLookupEncoding.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Windows1253.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"
namespace Krys::Text
{
  /// @brief The encoding that matches Microsoft Windows's Codepage 1253.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_windows_1253
      : public SingleASCIIByteHighBitLookupEncoding<
          basic_windows_1253<TCodeUnit, TCodePoint>, &EncodingTable::Windows1253IndexToCodePoint,
          &EncodingTable::Windows1253CodePointToIndex, TCodeUnit, TCodePoint>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"windows-1253"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 3> Aliases = {"windows-1253"_s,
                                                                                    "cp1253"_s, "x-cp1253"_s};
  };

  /// @brief The encoding that matches Microsoft Windows's Codepage 1253.
  constexpr inline basic_windows_1253<char> windows_1253 = {};
}
