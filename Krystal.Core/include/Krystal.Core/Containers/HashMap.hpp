#pragma once

#include "Krystal.Core/Hashing/FastHash.hpp"
#include <ankerl/unordered_dense.h>
#include <functional>
#include <unordered_map>

namespace krys
{
  /// @brief The engine's hash map: open addressing over one contiguous array of key-value pairs, so lookups
  /// and iteration are cache-friendly.
  /// @note Inserting can move every element and erasing moves the last element into the gap, so references,
  /// pointers and iterators into it don't survive either. Use `NodeHashMap` where they must.
  template <typename TKey, typename TValue, typename THash = FastHash<TKey>,
            typename TKeyEqual = std::equal_to<TKey>>
  using HashMap = ankerl::unordered_dense::map<TKey, TValue, THash, TKeyEqual>;

  /// @brief A hash map that allocates each element on its own, so references and pointers to elements stay
  /// valid until that element is erased. Slower than `HashMap`; use it only where that stability is needed.
  template <typename TKey, typename TValue, typename THash = FastHash<TKey>,
            typename TKeyEqual = std::equal_to<TKey>>
  using NodeHashMap = std::unordered_map<TKey, TValue, THash, TKeyEqual>;
}
