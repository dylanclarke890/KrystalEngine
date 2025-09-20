#pragma once

#include <unordered_map>

namespace Krys
{
  template <typename TKey, typename TValue, typename TKeyHasher = std::hash<TKey>>
  using Map = std::unordered_map<TKey, TValue, TKeyHasher>;

  template <typename TKey, typename TValue, typename TKeyHasher = std::hash<TKey>>
  using MultiMap = std::unordered_multimap<TKey, TValue, TKeyHasher>;
}
