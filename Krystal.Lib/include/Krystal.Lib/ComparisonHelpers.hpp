#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>

namespace Krys
{
  constexpr bool IsUndefined(std::floating_point auto value) noexcept
  {
    return value != value;
  }

  constexpr bool IsDefined(std::floating_point auto value) noexcept
  {
    return !IsUndefined(value);
  }

  constexpr bool IsInf(auto value) noexcept
  {
    return value == +std::numeric_limits<decltype(value)>::infinity()
           || value == -std::numeric_limits<decltype(value)>::infinity();
  }

  constexpr auto MaxOrDefined(std::floating_point auto a, std::floating_point auto b) noexcept
  {
    if (IsDefined(a) && IsDefined(b))
    {
      return std::max(a, b);
    }
    return IsUndefined(a) ? b : a;
  }

  constexpr auto MinOrDefined(std::floating_point auto a, std::floating_point auto b) noexcept
  {
    if (IsDefined(a) && IsDefined(b))
    {
      return std::min(a, b);
    }

    return IsUndefined(a) ? b : a;
  }

  // Custom equality functions using a hardcoded epsilon of 0.0001f, or returning
  // true if both floats are NaN.
  inline bool InexactEquals(float a, float b) noexcept
  {
    if (IsDefined(a) && IsDefined(b))
    {
      return std::abs(a - b) < 0.0001f;
    }
    return IsUndefined(a) && IsUndefined(b);
  }

  inline bool InexactEquals(double a, double b) noexcept
  {
    if (IsDefined(a) && IsDefined(b))
    {
      return std::abs(a - b) < 0.0001;
    }
    return IsUndefined(a) && IsUndefined(b);
  }

  template <std::size_t Size, typename ElementT>
  bool InexactEquals(const std::array<ElementT, Size> &a, const std::array<ElementT, Size> &b) noexcept
  {
    bool areEqual = true;
    for (std::size_t i = 0; i < Size && areEqual; ++i)
    {
      areEqual = InexactEquals(a[i], b[i]);
    }
    return areEqual;
  }
}