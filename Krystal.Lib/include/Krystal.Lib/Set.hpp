#pragma once

#include <set>

namespace Krys
{
  template <typename T, typename TComparer = std::less<T>>
  using Set = std::set<T, TComparer>;
}