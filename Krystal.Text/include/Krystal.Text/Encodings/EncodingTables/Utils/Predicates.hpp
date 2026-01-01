#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Utils/TableTypes.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>

namespace Krys::Text::EncodingTable
{
  template <typename TIndexCodePoint, typename TIndex = decltype(std::declval<TIndexCodePoint>()[0])>
  KRYS_NODISCARD constexpr inline bool LessThanIndexTarget(const TIndexCodePoint &value, TIndex target) noexcept
  {
    return value[0] < target;
  }

  template <typename TIndexCodePoint, typename TIndex = decltype(std::declval<TIndexCodePoint>()[0])>
  KRYS_NODISCARD constexpr inline bool LessThanCodePointTarget(const TIndexCodePoint &value,
                                                               TIndex target) noexcept
  {
    return value[1] < target;
  }

  KRYS_NODISCARD constexpr inline bool LessThanIndex8Target(const index8_code_point &value,
                                                            index16 target) noexcept
  {
    return LessThanIndexTarget<index8_code_point>(value, target);
  }

  KRYS_NODISCARD constexpr inline bool LessThanIndex16Target(const index16_code_point &value,
                                                             index16 target) noexcept
  {
    return LessThanIndexTarget<index16_code_point>(value, target);
  }

  KRYS_NODISCARD constexpr inline bool LessThanIndex32Target(const index32_code_point &value,
                                                             index32 target) noexcept
  {
    return LessThanIndexTarget<index32_code_point>(value, target);
  }

  KRYS_NODISCARD constexpr inline bool LessThanCodePoint16Target(const index16_code_point &value,
                                                      index16 target) noexcept
  {
    return LessThanCodePointTarget<index16_code_point>(value, target);
  }

  KRYS_NODISCARD constexpr inline bool LessThanCodePoint32Target(const index32_code_point &value,
                                                      index32 target) noexcept
  {
    return LessThanCodePointTarget<index32_code_point>(value, target);
  }
}
