#pragma once

#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys
{
  struct Bit
  {
    STATIC_CLASS(Bit)

    template <Integral T>
    KRYS_NODISCARD static constexpr T Mask(T bits) noexcept
    {
      return bits >= sizeof(T) * 8 ? ~T(0) : (T(1) << bits) - 1;
    }

    template <Integral T>
    KRYS_NODISCARD static constexpr T Set(T value, uint first, uint count = 1) noexcept
    {
      return value | T(Mask(count) << first);
    }

    template <Integral T>
    KRYS_NODISCARD static constexpr T Unset(T value, uint first, uint count = 1) noexcept
    {
      return value & ~T(Mask(count) << first);
    }

    template <Integral T>
    KRYS_NODISCARD static constexpr uint32 Count(T value) noexcept
    {
      uint count;
      for (count = 0; value; count++)
        value &= value - 1; // clear the least significant bit set
      return count;
    }

    template <Integral T>
    KRYS_NODISCARD static constexpr T RotateRight(T value, uint count) noexcept
    {
      const int size = sizeof(T) * 8;
      count %= size;
      return (value >> T(count)) | (value << T(size - count));
    }

    template <Integral T>
    KRYS_NODISCARD static constexpr T RotateLeft(T value, uint count) noexcept
    {
      const int size = sizeof(T) * 8;
      return (value << T(count)) | (value >> T(size - count));
    }
  };
}
