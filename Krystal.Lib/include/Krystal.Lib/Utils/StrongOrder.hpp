#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include <compare>

namespace Krys
{
  struct StrongOrder
  {
    KRYS_NODISCARD constexpr static std::strong_ordering Reverse(std::strong_ordering order) noexcept
    {
      return 0 <=> order;
    }

    KRYS_NODISCARD constexpr static bool IsGreaterThan(std::strong_ordering order) noexcept
    {
      return order > 0;
    }

    KRYS_NODISCARD constexpr static bool IsLessThan(std::strong_ordering order) noexcept
    {
      return order < 0;
    }

    KRYS_NODISCARD constexpr static bool IsEqual(std::strong_ordering order) noexcept
    {
      return order == 0;
    }
  };
}