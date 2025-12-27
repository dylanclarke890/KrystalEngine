#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <bit>
#include <utility>

namespace Krys
{
  template <IsEnum TEnum>
  KRYS_NODISCARD constexpr uint32 OrdinalCount() noexcept;

  template <typename TEnum>
  concept HasOrdinality = (OrdinalCount<TEnum>() > 0);

  template <typename T>
  KRYS_NODISCARD constexpr auto ToUnderlying(T value) noexcept
  {
    return std::to_underlying<T>(value);
  }

  /// @brief Count of bits needed to represent every ordinal.
  template <HasOrdinality TEnum>
  KRYS_NODISCARD constexpr uint32 BitCount() noexcept
  {
    return std::bit_width(static_cast<std::underlying_type_t<TEnum>>(OrdinalCount<TEnum>() - 1));
  }

  /// @brief Convenience function to iterate through every value in a Krys enum as part of
  /// a range-based for loop.
  template <HasOrdinality TEnum>
  KRYS_NODISCARD constexpr auto Ordinals() noexcept
  {
    struct Iterator
    {
      TEnum e {};

      KRYS_NODISCARD constexpr TEnum operator*() const noexcept
      {
        return e;
      }

      constexpr Iterator &operator++() noexcept
      {
        e = static_cast<TEnum>(std::to_underlying(e) + 1);
        return *this;
      }

      constexpr bool operator==(const Iterator &other) const noexcept = default;
      constexpr bool operator!=(const Iterator &other) const noexcept = default;
    };

    struct Range
    {
      KRYS_NODISCARD constexpr Iterator begin() const noexcept
      {
        return Iterator {};
      }
      KRYS_NODISCARD constexpr Iterator end() const noexcept
      {
        return Iterator {static_cast<TEnum>(OrdinalCount<TEnum>())};
      }
    };

    return Range {};
  }

/// @brief Defines bitwise operators for an enum class.
/// @param EnumType The enum class to define the operators for.
#define KRYS_ENUM_FLAG_OPERATORS(EnumType)                                                                   \
  KRYS_NODISCARD constexpr inline EnumType operator|(EnumType a, EnumType b) noexcept                            \
  {                                                                                                          \
    return static_cast<EnumType>(ToUnderlying(a) | ToUnderlying(b));                                         \
  }                                                                                                          \
                                                                                                             \
  KRYS_NODISCARD constexpr inline EnumType operator&(EnumType a, EnumType b) noexcept                            \
  {                                                                                                          \
    return static_cast<EnumType>(ToUnderlying(a) & ToUnderlying(b));                                         \
  }                                                                                                          \
                                                                                                             \
  KRYS_NODISCARD constexpr inline EnumType operator^(EnumType a, EnumType b) noexcept                            \
  {                                                                                                          \
    return static_cast<EnumType>(ToUnderlying(a) ^ ToUnderlying(b));                                         \
  }                                                                                                          \
                                                                                                             \
  KRYS_NODISCARD constexpr inline EnumType operator~(EnumType a) noexcept                                        \
  {                                                                                                          \
    return static_cast<EnumType>(~ToUnderlying(a));                                                          \
  }                                                                                                          \
                                                                                                             \
  constexpr inline EnumType &operator|=(EnumType &a, EnumType b) noexcept                                    \
  {                                                                                                          \
    return a = a | b;                                                                                        \
  }                                                                                                          \
                                                                                                             \
  constexpr inline EnumType &operator&=(EnumType &a, EnumType b) noexcept                                    \
  {                                                                                                          \
    return a = a & b;                                                                                        \
  }                                                                                                          \
                                                                                                             \
  constexpr inline EnumType &operator^=(EnumType &a, EnumType b) noexcept                                    \
  {                                                                                                          \
    return a = a ^ b;                                                                                        \
  }                                                                                                          \
                                                                                                             \
  KRYS_NODISCARD constexpr inline bool operator!(EnumType a) noexcept                                            \
  {                                                                                                          \
    return ToUnderlying(a) == 0;                                                                             \
  }
}