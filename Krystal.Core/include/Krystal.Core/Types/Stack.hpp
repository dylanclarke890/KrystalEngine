#pragma once

#include <deque>
#include <stack>

namespace krys
{
  template <typename T, typename Container = std::deque<T>>
  using Stack = std::stack<T, Container>;
}