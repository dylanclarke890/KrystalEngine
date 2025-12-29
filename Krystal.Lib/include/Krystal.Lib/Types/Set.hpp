#pragma once

#include <unordered_set>

namespace Krys
{
  template <typename T, typename Hash = std::hash<T>, typename KeyEqual = std::equal_to<T>>
  using Set = std::unordered_set<T, Hash, KeyEqual>;
}