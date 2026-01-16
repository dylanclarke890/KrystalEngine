#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Encodings/Impl/GBK_or_GB18030.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Text
{
  /// @brief The single-byte GBK encoding, primarily for use with Chinese text.
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_gbk : public ::Krys::Text::Encodings::Impl::BasicGB18030<basic_gbk<TCodeUnit, TCodePoint>,
                                                                       TCodeUnit, TCodePoint, true>
  {
  public:
    constexpr static inline ::Krys::Text::ASCIILiteral Name = {"GBK"_s};
    constexpr static inline ::Krys::Array<::Krys::Text::ASCIILiteral, 9> Aliases = {
      "chinese"_s,    "csgb2312"_s, "csiso58gb231280"_s, "gb2312"_s, "gb_2312"_s,
      "gb_2312-80"_s, "gbk"_s,      "iso-ir-58"_s,       "x-gbk"_s};
  };

  /// @brief An instance of basic_gbk for ease of use.
  constexpr inline basic_gbk<char> gbk = {};
}
