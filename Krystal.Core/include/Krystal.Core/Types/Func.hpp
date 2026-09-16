#pragma once

#include <functional>

namespace krys
{
  template <typename T>
  using Func = std::function<T>;
}