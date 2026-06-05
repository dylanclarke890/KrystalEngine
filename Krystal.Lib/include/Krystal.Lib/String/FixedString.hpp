#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <xutility>

namespace Krys
{
  /// @brief A fixed-size string that can be used in compile-time contexts (including as a NTTP). The size
  /// includes the null terminator.
  template <OneOf<char, char8, char16, char32> TChar, size_t N>
  struct FixedString
  {
    TChar Data[N] {};

    consteval FixedString(const TChar (&str)[N])
    {
      std::copy_n(str, N, Data);
    }

    KRYS_NODISCARD consteval bool operator==(const FixedString &other) const noexcept
    {
      return std::equal(std::begin(Data), std::end(Data), std::begin(other.Data));
    }

    KRYS_NODISCARD consteval bool operator!=(const FixedString &other) const noexcept
    {
      return !(*this == other);
    }

    template <size_t M>
    KRYS_NODISCARD consteval bool operator==(const FixedString<TChar, M> &other) const noexcept
    {
      if (N != M)
      {
        return false;
      }

      return std::equal(std::begin(Data), std::end(Data), std::begin(other.Data));
    }

    template <size_t M>
    KRYS_NODISCARD consteval bool operator!=(const FixedString<TChar, M> &other) const noexcept
    {
      return !(*this == other);
    }
  };
}