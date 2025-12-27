#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include <bit>
#include <utility>

namespace Krys
{
  template <IsEnum TEnum>
  NO_DISCARD constexpr uint32 OrdinalCount() noexcept;

  template <typename TEnum>
  concept HasOrdinality = (OrdinalCount<TEnum>() > 0);

  template <typename T>
  NO_DISCARD constexpr auto ToUnderlying(T value) noexcept
  {
    return std::to_underlying<T>(value);
  }

  /// @brief Count of bits needed to represent every ordinal.
  template <HasOrdinality TEnum>
  NO_DISCARD constexpr uint32 BitCount() noexcept
  {
    return std::bit_width(static_cast<std::underlying_type_t<TEnum>>(OrdinalCount<TEnum>() - 1));
  }

  /// @brief Convenience function to iterate through every value in a Krys enum as part of
  /// a range-based for loop.
  template <HasOrdinality TEnum>
  NO_DISCARD constexpr auto Ordinals() noexcept
  {
    struct Iterator
    {
      TEnum e {};

      NO_DISCARD constexpr TEnum operator*() const noexcept
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
      NO_DISCARD constexpr Iterator begin() const noexcept
      {
        return Iterator {};
      }
      NO_DISCARD constexpr Iterator end() const noexcept
      {
        return Iterator {static_cast<TEnum>(OrdinalCount<TEnum>())};
      }
    };

    return Range {};
  }
}