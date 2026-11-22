#pragma once

#include "Krystal.Core/Hashing/FastHash.hpp"
#include <ankerl/unordered_dense.h>
#include <functional>
#include <unordered_set>

namespace krys
{
  /// @brief The engine's hash set: open addressing over one contiguous array of elements, so lookups and
  /// iteration are cache-friendly.
  /// @note Inserting can move every element and erasing moves the last element into the gap, so references,
  /// pointers and iterators into it don't survive either. Use `NodeHashSet` where they must.
  template <typename T, typename THash = FastHash<T>, typename TKeyEqual = std::equal_to<T>>
  using HashSet = ankerl::unordered_dense::set<T, THash, TKeyEqual>;

  /// @brief A hash set that allocates each element on its own, so references and pointers to elements stay
  /// valid until that element is erased. Slower than `HashSet`; use it only where that stability is needed.
  template <typename T, typename THash = FastHash<T>, typename TKeyEqual = std::equal_to<T>>
  using NodeHashSet = std::unordered_set<T, THash, TKeyEqual>;
}
