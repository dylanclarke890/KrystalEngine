#pragma once

#include <deque>
#include <queue>

namespace krys
{
  template <typename T>
  using Queue = std::queue<T>;

  template <typename T>
  using Deque = std::deque<T>;
}