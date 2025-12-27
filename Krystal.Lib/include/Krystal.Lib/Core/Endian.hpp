#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <bit>

namespace Krys
{
  struct Endian : NonCopyMovable<Endian>
  {
    Endian() = delete;
    ~Endian() = delete;

    /// @brief Represents the endianness of a system or value.
    enum Type : uint8
    {
      Little = 0,
      Big = 1,
      System = 2
    };

    /// @brief Checks if the system is big endian.
    KRYS_NODISCARD static constexpr bool IsSystemBigEndian() noexcept
    {
      return std::endian::native == std::endian::big;
    }

    /// @brief Checks if the system is little endian.
    KRYS_NODISCARD static constexpr bool IsSystemLittleEndian() noexcept
    {
      return std::endian::native == std::endian::little;
    }

    /// @brief Swaps the byte order of an integral or floating-point value.
    /// @tparam T The type of the value to be swapped (integral or floating-point).
    /// @param value The value whose byte order is to be swapped.
    /// @return The value with its byte order swapped.
    template <Number T>
    KRYS_NODISCARD static constexpr T SwapEndian(T value)
    {
      if constexpr (Integral<T>)
      {
        return std::byteswap(value);
      }
      // For floating-point types, use std::bit_cast to swap bytes
      else if constexpr (FloatingPoint<T>)
      {
        using as_uint_t = std::conditional_t<sizeof(T) == 4, uint32, uint64>;
        as_uint_t temp = std::bit_cast<as_uint_t>(value);
        temp = std::byteswap(temp);
        return std::bit_cast<T>(temp);
      }
    }

    /// @brief Converts a value to big endian. If the underlying system is big endian, the value is assumed to
    /// already be represented as big endian and is returned unchanged.
    template <Number T>
    KRYS_NODISCARD static constexpr T ToBigEndian(T value) noexcept
    {
      if constexpr (IsSystemBigEndian())
      {
        return value;
      }
      else
      {
        return SwapEndian(value);
      }
    }

    /// @brief Converts a value to little endian. If the underlying system is little endian, the value is
    /// assumed to already be represented as little endian and is returned unchanged.
    template <Number T>
    KRYS_NODISCARD static constexpr T ToLittleEndian(T value) noexcept
    {
      if constexpr (IsSystemLittleEndian())
      {
        return value;
      }
      else
      {
        return SwapEndian(value);
      }
    }

    /// @brief Converts a value to the system's native endian.
    /// @tparam T The type of the value to be converted (integral or floating-point).
    /// @param value The value to convert.
    /// @param from The endian type the value is currently in (either Little or Big).
    /// @return The value converted to the system's native endianness.
    template <Endian::Type Src, Number T>
    KRYS_NODISCARD static constexpr T ToSystemEndian(T value) noexcept
    {
      if constexpr (Src == Little)
      {
        return IsSystemLittleEndian() ? value : SwapEndian(value);
      }
      else if constexpr (Src == Big)
      {
        return IsSystemBigEndian() ? value : SwapEndian(value);
      }
      else
      {
        return value;
      }
    }

    /// @brief Convert a value from one endian representation to another.
    template <Endian::Type Src, Endian::Type Dst, Number T>
    KRYS_NODISCARD static constexpr T Convert(T value) noexcept
    {
      if constexpr (Src == Dst)
      {
        return value;
      }
      else if constexpr (Dst == System)
      {
        return ToSystemEndian<Src, T>(value);
      }
      else if constexpr (Src == System)
      {
        if constexpr (Dst == Little)
        {
          return ToLittleEndian(value);
        }
        else if constexpr (Dst == Big)
        {
          return ToBigEndian(value);
        }
        else
        {
          return value;
        }
      }
      else
      {
        // from == Big && to == Little or vice versa.
        return SwapEndian(value);
      }
    }
  };
}