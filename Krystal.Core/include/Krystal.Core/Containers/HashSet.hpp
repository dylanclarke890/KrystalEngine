#pragma once

#include "Krystal.Core/Hashing/FastHash.hpp"
#include <unordered_set>

namespace krys
{
  template <typename T, typename Hash = FastHash<T>, typename KeyEqual = std::equal_to<T>>
  using HashSet = std::unordered_set<T, Hash, KeyEqual>;
}
