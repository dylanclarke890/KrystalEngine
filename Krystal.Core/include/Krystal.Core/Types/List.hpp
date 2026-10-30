#pragma once

#include <algorithm>
#include <vector>

namespace krys
{
  template <typename T>
  using List = std::vector<T>;

  /// @brief Removes the first element of @p list that matches @p predicate.
  /// @return Whether an element was removed.
  template <typename T, typename TPredicate>
  bool RemoveFirstMatching(List<T> &list, TPredicate &&predicate) noexcept
  {
    auto it = std::ranges::find_if(list, predicate);
    if (it == list.end())
    {
      return false;
    }
    list.erase(it);
    return true;
  }
}