#pragma once

#include <functional>

namespace Krys
{
  template <typename T>
  using Func = std::function<T>;
}