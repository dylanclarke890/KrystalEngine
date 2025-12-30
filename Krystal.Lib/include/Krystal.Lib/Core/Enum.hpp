#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <bit>
#include <utility>

namespace Krys
{
  template <IsEnum E>
  inline constexpr UnderlyingType<E> OrdinalCount = 0u;

  template <typename TEnum>
  concept HasOrdinality = (OrdinalCount<TEnum> > 0u);

  template <typename E>
  inline constexpr bool EnableEnumFlags = false;

  template <typename E>
  concept IsFlagsEnum = IsEnum<E> && EnableEnumFlags<E>;

  /// @brief Convenience function to iterate through every value in a Krys enum as part of
  /// a range-based for loop.
  template <HasOrdinality TEnum>
  KRYS_NODISCARD constexpr auto Ordinals() noexcept
  {
    struct Iterator
    {
      TEnum Value {};

      KRYS_NODISCARD constexpr TEnum operator*() const noexcept
      {
        return Value;
      }

      constexpr Iterator &operator++() noexcept
      {
        if constexpr (EnableEnumFlags<TEnum>)
        {
          if (ToUnderlying<TEnum>(Value) == 0u)
          {
            Value = static_cast<TEnum>(1u);
          }
          else
          {
            Value = static_cast<TEnum>(ToUnderlying<TEnum>(Value) << 1);
          }
        }
        else
        {
          Value = static_cast<TEnum>(ToUnderlying<TEnum>(Value) + 1);
        }

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
        return Iterator {static_cast<TEnum>(OrdinalCount<TEnum>)};
      }
    };

    return Range {};
  }

  /// @brief Count of bits needed to represent every ordinal.
  template <HasOrdinality TEnum>
  KRYS_NODISCARD constexpr uint32 BitCount() noexcept
  {
    return std::bit_width(static_cast<UnderlyingType<TEnum>>(OrdinalCount<TEnum> - 1u));
  }

  template <IsFlagsEnum TEnum>
  constexpr TEnum operator|(TEnum lhs, TEnum rhs)
  {
    using U = UnderlyingType<TEnum>;
    return static_cast<TEnum>(static_cast<U>(lhs) | static_cast<U>(rhs));
  }

  template <IsFlagsEnum TEnum>
  constexpr TEnum &operator|=(TEnum &lhs, TEnum rhs)
  {
    lhs = lhs | rhs;
    return lhs;
  }

  template <IsFlagsEnum TEnum>
  constexpr TEnum operator&(TEnum lhs, TEnum rhs)
  {
    using U = UnderlyingType<TEnum>;
    return static_cast<TEnum>(static_cast<U>(lhs) & static_cast<U>(rhs));
  }

  template <IsFlagsEnum TEnum>
  constexpr TEnum &operator&=(TEnum &lhs, TEnum rhs)
  {
    lhs = lhs & rhs;
    return lhs;
  }

  template <IsFlagsEnum TEnum>
  constexpr TEnum operator~(TEnum value)
  {
    using U = UnderlyingType<TEnum>;
    return static_cast<TEnum>(~static_cast<U>(value));
  }

  template <IsFlagsEnum TEnum>
  constexpr TEnum operator^(TEnum lhs, TEnum rhs)
  {
    using U = UnderlyingType<TEnum>;
    return static_cast<TEnum>(static_cast<U>(lhs) ^ static_cast<U>(rhs));
  }

  template <IsFlagsEnum TEnum>
  constexpr TEnum &operator^=(TEnum &lhs, TEnum rhs)
  {
    lhs = lhs ^ rhs;
    return lhs;
  }

  template <IsFlagsEnum TEnum>
  KRYS_NODISCARD constexpr bool operator!(TEnum value) noexcept
  {
    return ToUnderlying<TEnum>(value) == 0;
  }

  template <IsFlagsEnum TEnum>
  KRYS_NODISCARD constexpr bool HasFlag(TEnum value, TEnum flag) noexcept
  {
    using U = UnderlyingType<TEnum>;
    return (static_cast<U>(value) & static_cast<U>(flag)) == static_cast<U>(flag);
  }
}