#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys
{
  KRYS_NODISCARD constexpr uint64 ipow_impl(uint64 base, uint64 exp, uint64 result) noexcept
  {
    return exp < 1u ? result : ipow_impl(base * base, exp / 2, (exp % 2) ? result * base : result);
  }

  KRYS_NODISCARD constexpr uint64 ipow(uint64 base, uint64 exp) noexcept
  {
    return ipow_impl(base, exp, 1);
  }
}