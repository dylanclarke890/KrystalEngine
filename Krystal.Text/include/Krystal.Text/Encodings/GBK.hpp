#pragma once

#include "Krystal.Text/Encodings/GB18030.hpp"

namespace Krys
{
  /// @brief The single-byte GBK encoding, primarily for use with Chinese text.
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_gbk : public Impl::BasicGB18030<basic_gbk<TCodeUnit, TCodePoint>, TCodeUnit, TCodePoint, true>
  {
  };

  /// @brief An instance of basic_gbk for ease of use.
  inline constexpr basic_gbk<char> gbk = {};
}
