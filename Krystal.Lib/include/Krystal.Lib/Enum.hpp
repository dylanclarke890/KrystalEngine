#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Types.hpp"
#include <bit>
#include <utility>

namespace Krys
{
  template <typename T>
  NO_DISCARD constexpr auto ToUnderlying(T value) noexcept
  {
    return std::to_underlying<T>(value);
  }

  /// @brief Concept that checks if a type is an enumeration.
  template <typename TEnum>
  concept Enumeration = std::is_enum_v<TEnum>;

  template <Enumeration TEnum>
  constexpr uint32 OrdinalCount();

  template <typename TEnum>
  concept HasOrdinality = (OrdinalCount<TEnum>() > 0);

  /// @brief Count of bits needed to represent every ordinal.
  template <HasOrdinality TEnum>
  constexpr uint32 BitCount()
  {
    return std::bit_width(static_cast<std::underlying_type_t<TEnum>>(OrdinalCount<TEnum>() - 1));
  }

  /// @brief Convenience function to iterate through every value in a Krys enum as part of
  /// a range-based for loop.
  template <HasOrdinality TEnum>
  auto Ordinals()
  {
    struct Iterator
    {
      TEnum e {};

      TEnum operator*() const
      {
        return e;
      }

      Iterator &operator++()
      {
        e = static_cast<TEnum>(std::to_underlying(e) + 1);
        return *this;
      }

      bool operator==(const Iterator &other) const = default;
      bool operator!=(const Iterator &other) const = default;
    };

    struct Range
    {
      Iterator begin() const
      {
        return Iterator {};
      }
      Iterator end() const
      {
        return Iterator {static_cast<TEnum>(OrdinalCount<TEnum>())};
      }
    };

    return Range {};
  }
}