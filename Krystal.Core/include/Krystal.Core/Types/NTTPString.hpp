#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <xutility>

namespace krys
{
  /// @brief A fixed-size string that can be used in compile-time contexts (such as as a NTTP). The size
  /// includes the null terminator.
  template <IsChar TChar, size_t N>
  struct NTTPString
  {
    TChar Data[N] {};
    size_t Size {N};

    consteval NTTPString(const TChar (&str)[N]) noexcept
    {
      std::copy_n(str, N, Data);
    }

    KRYS_NODISCARD consteval bool operator==(const NTTPString &other) const noexcept
    {
      return std::equal(std::begin(Data), std::end(Data), std::begin(other.Data));
    }

    KRYS_NODISCARD consteval bool operator!=(const NTTPString &other) const noexcept
    {
      return !(*this == other);
    }

    template <size_t M>
    KRYS_NODISCARD consteval bool operator==(const NTTPString<TChar, M> &other) const noexcept
    {
      if (N != M)
      {
        return false;
      }

      return std::equal(std::begin(Data), std::end(Data), std::begin(other.Data));
    }

    template <size_t M>
    KRYS_NODISCARD consteval bool operator!=(const NTTPString<TChar, M> &other) const noexcept
    {
      return !(*this == other);
    }
  };
}