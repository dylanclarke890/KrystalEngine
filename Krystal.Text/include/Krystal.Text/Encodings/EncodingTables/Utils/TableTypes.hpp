#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

namespace Krys::Text::EncodingTable
{
  using index8 = uint_least8_t;
  using index8_code_point = index8[2];
  using index16 = uint_least16_t;
  using index16_code_point = index16[2];
  using index32 = uint_least32_t;
  using index32_code_point = index32[2];

  using basic_lookup_index_to_code_point_function = std::optional<uint_least32_t>(std::size_t);
  using basic_lookup_code_point_to_index_function = std::optional<std::size_t>(uint_least32_t);
}
