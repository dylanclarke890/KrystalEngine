#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Pair.hpp"
#include <algorithm>
#include <assert.h>
#include <utility>

/// NOTE: Credit to Dan Israel Malta:
/// https://github.com/DanIsraelMalta/BiDirectional-Map/blob/master/BiDirectionalMap.h
namespace Krys
{
  template <typename T>
  concept ImmutableBimapItem = EqualityComparable<T> && GreaterThanComparable<T>;

  /// TODO: this is a bit of a naive implementation in terms of performance.

  /// @brief Fixed-size bi-directional flat map which is immutable after creation.
  /// @tparam N number of items/entrys in dictionary
  /// @tparam TKey key type
  /// @tparam TValue value type
  template <ImmutableBimapItem TKey, ImmutableBimapItem TValue, size_t N>
  class ImmutableBimap
  {
    using Item = Pair<TKey, TValue>;
    using Map = Array<Item, N>;

  private:
    alignas(typename std::aligned_storage<sizeof(Item), alignof(Item)>::type) Map _map;

  public:
    explicit constexpr ImmutableBimap(const Map &map) noexcept : _map(map)
    {
      Sort();
    }

    explicit constexpr ImmutableBimap(std::initializer_list<Item> &&map) noexcept
        : _map(reinterpret_cast<const Map &>(*map.begin()))
    {
      Sort();
    }

  public:
    /// @brief return the value of a given key (O(log2(index of n))) using exponential search
    KRYS_NODISCARD constexpr TValue GetValueOfKey(const TKey key) noexcept
    {
      if (_map[0].first == key)
      {
        return _map[0].second;
      }

      // "exponential search"
      std::size_t i {1};
      while ((i < N) && (_map[i].first < key))
      {
        i *= 2;
      }

      // apply binary search
      return BinarySearch(key, i / 2, (i + 1 < N) ? (i + 1) : N);
    }

    /// @brief return the key of a given value (O(n)) if there are several identical value's, return the key
    /// of the first value according to dictionary construction order
    KRYS_NODISCARD constexpr TKey GetKeyOfValue(const TValue value) noexcept
    {
      bool found {false};
      size_t i = 0u;

      while (!found && (i < N))
      {
        found = _map[i].second == value;
        i++;
      }

      if (found)
      {
        return _map[i - 1].first;
      }

      return TKey {};
    }

  private:
    /// @brief sort (according to keys) and assert if there are duplicate keys
    void Sort() noexcept
    {
      std::sort(_map.begin(), _map.end(),
                [](const Item &left, const Item &right) { return (left.first < right.first); });

#ifdef KRYS_ENV(DEV)
      // alert for duplicate keys
      auto it = std::adjacent_find(_map.begin(), _map.end(), [](const Item &left, const Item &right)
                                   { return (left.first == right.first); });

      assert(it == _map.end() && "BiMap was constructed with a duplicate key!");
#endif
    }

    /// @brief recursive binary search
    constexpr inline TValue BinarySearch(const TKey key, std::size_t left, std::size_t right) noexcept
    {
      // region center point
      std::size_t middle {};

      while (left != right)
      {
        // update range center
        middle = left + (right - left) / 2;

        // update regions
        if (_map[middle].first < key)
        {
          left = middle;
        }
        else
        {
          right = middle - 1;
        }
      }

      if (left == right)
      {
        return _map[middle].second;
      }

      // key not found
      return TValue {};
    }
  };
}