#pragma once

#include <unordered_map>

namespace krys
{
  template <typename TKey, typename TValue, typename TKeyHasher = std::hash<TKey>>
  using HashMap = std::unordered_map<TKey, TValue, TKeyHasher>;

  template <typename TKey, typename TValue, typename TKeyHasher = std::hash<TKey>>
  using MultiHashMap = std::unordered_multimap<TKey, TValue, TKeyHasher>;
}
