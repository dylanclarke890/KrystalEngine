#pragma once

#include <map>

namespace Krys
{
  template <typename TKey, typename TValue, typename TKeyHasher = std::hash<TKey>>
  using OrderedMap = std::map<TKey, TValue, TKeyHasher>;

  template <typename TKey, typename TValue, typename TKeyHasher = std::hash<TKey>>
  using OrderedMultiMap = std::multimap<TKey, TValue, TKeyHasher>;
}