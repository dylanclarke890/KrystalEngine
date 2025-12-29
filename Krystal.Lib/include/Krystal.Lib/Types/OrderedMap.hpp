#pragma once

#include <map>

namespace Krys
{
  template <typename TKey, typename TValue, typename TComparator = std::less<TKey>>
  using OrderedMap = std::map<TKey, TValue, TComparator>;

  template <typename TKey, typename TValue, typename TComparator = std::less<TKey>>
  using OrderedMultiMap = std::multimap<TKey, TValue, TComparator>;
}