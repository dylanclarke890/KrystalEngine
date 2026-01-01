#pragma once

#include "Krystal.Text/Encodings/EncodingTables/kamenicky.tables.hpp"
#include "Krystal.Text/Impl/SingleByteLookupEncoding.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys
{
  /// @brief The encoding that matches the KAMENICKY encoding specification.
  /// @tparam TCodeUnit The default code unit type to use when outputting encoded units.
  /// @tparam TCodePoint The code point type to use when outputting decoded units.
  template <typename TCodeUnit = char, typename TCodePoint = UnicodeCodePoint>
  class basic_kamenicky
      : public Impl::SingleByteLookupEncoding<
          basic_kamenicky<TCodeUnit, TCodePoint>, &::Krys::Text::EncodingTable::kamenicky_index_to_code_point,
          &::Krys::Text::EncodingTable::kamenicky_code_point_to_index, TCodeUnit, TCodePoint>
  {
  };

  /// @brief An instance of basic_kamenicky for ease of use.
  inline constexpr basic_kamenicky<char> kamenicky = {};
}
