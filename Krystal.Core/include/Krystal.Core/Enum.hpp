#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/TypeTraits.hpp"
#include <bit>

// An enum describes its range with hooks declared beside it, in its namespace (or as friends of the class
// it's nested in), which are found by argument-dependent lookup:
//
//   consteval E MaxEnumValue(E) noexcept;   // Its highest enumerator, for EnumSet and EnumBitWidth.
//   consteval E MinEnumValue(E) noexcept;   // Its lowest, where that isn't 0.
//
// Without MaxEnumValue an EnumSet takes 64 bits; without MinEnumValue the lowest value is taken to be 0.

namespace krys
{
  /// @brief Whether @p E declares its highest enumerator with a `MaxEnumValue` hook.
  template <typename E>
  concept HasMaxEnumValue = IsEnum<E> && requires {
    { MaxEnumValue(E {}) } -> SameType<E>;
  };

  namespace detail
  {
    template <typename E>
    concept HasMinEnumValue = IsEnum<E> && requires {
      { MinEnumValue(E {}) } -> SameType<E>;
    };

    /// @brief The lowest value of @p E, as an `int64`: its `MinEnumValue`, or 0.
    template <typename E>
    KRYS_NODISCARD consteval int64 MinEnumUnderlying() noexcept
    {
      if constexpr (HasMinEnumValue<E>)
      {
        return static_cast<int64>(ToUnderlying(MinEnumValue(E {})));
      }
      else
      {
        return 0;
      }
    }

    /// @brief The highest value of @p E, as an `int64`.
    template <HasMaxEnumValue E>
    KRYS_NODISCARD consteval int64 MaxEnumUnderlying() noexcept
    {
      constexpr auto max = static_cast<int64>(ToUnderlying(MaxEnumValue(E {})));
      static_assert(max >= MinEnumUnderlying<E>(),
                    "MaxEnumValue is below MinEnumValue (or below 0 without one)");
      return max;
    }

    template <HasMaxEnumValue E>
    KRYS_NODISCARD consteval uint8 EnumBitWidth() noexcept
    {
      constexpr int64 min = MinEnumUnderlying<E>();
      constexpr int64 max = MaxEnumUnderlying<E>();
      if constexpr (min >= 0)
      {
        return static_cast<uint8>(std::bit_width(static_cast<uint64>(max)));
      }
      else
      {
        static_assert(Signed<underlying_t<E>>, "An enum with negative values needs a signed underlying type");
        const auto magnitude = std::bit_width(static_cast<uint64>(max > 0 ? max : 0));
        const auto negativeMagnitude = std::bit_width(static_cast<uint64>(~min));
        return static_cast<uint8>(1 + (magnitude > negativeMagnitude ? magnitude : negativeMagnitude));
      }
    }
  }

  /// @brief The width a bit-field needs to hold any value of @p E.
  template <HasMaxEnumValue E>
  constexpr uint8 EnumBitWidth = detail::EnumBitWidth<E>();
}
