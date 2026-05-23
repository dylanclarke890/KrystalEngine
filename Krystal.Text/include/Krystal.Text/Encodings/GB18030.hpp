#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/detail/GBK_or_GB18030.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The GB18030 encoding, which can encode all of Unicode (and occasionally adds characters that
  /// exist outside of Unicode, but are still Unicode-compatible).
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_gb18030
      : public Basic_GBK_GB18030<basic_gb18030<TCodeUnit, TCodePoint>, TCodeUnit, TCodePoint, false>
  {
  public:
    constexpr static inline ASCIILiteral Name = {"gb18030"_s};
    constexpr static inline Array<ASCIILiteral, 1> Aliases = {"gb18030"_s};
  };

  /// @brief An instance of basic_gb18030 for ease of use.
  constexpr inline basic_gb18030<char> gb18030 = {};
}
