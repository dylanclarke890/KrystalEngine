#pragma once

#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::Text::EncodingTable
{
  using index8 = uint8;
  using index8_code_point = index8[2];
  using index16 = uint16;
  using index16_code_point = index16[2];
  using index32 = uint32;
  using index32_code_point = index32[2];

  using basic_lookup_index_to_code_point_function = Maybe<uint32>(size_t);
  using basic_lookup_code_point_to_index_function = Maybe<size_t>(uint32);
}
