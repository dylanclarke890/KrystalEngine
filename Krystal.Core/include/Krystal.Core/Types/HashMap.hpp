#pragma once

#include "Krystal.Core/Hashing/FastHash.hpp"
#include <unordered_map>

namespace krys
{
  template <typename TKey, typename TValue, typename TKeyHasher = FastHash<TKey>>
  using HashMap = std::unordered_map<TKey, TValue, TKeyHasher>;

  template <typename TKey, typename TValue, typename TKeyHasher = FastHash<TKey>>
  using MultiHashMap = std::unordered_multimap<TKey, TValue, TKeyHasher>;
}
