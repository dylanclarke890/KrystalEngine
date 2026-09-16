#pragma once

#include <unordered_set>

namespace krys
{
  template <typename T, typename Hash = std::hash<T>, typename KeyEqual = std::equal_to<T>>
  using HashSet = std::unordered_set<T, Hash, KeyEqual>;
}