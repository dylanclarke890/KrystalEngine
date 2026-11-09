#pragma once

#include <functional>

namespace krys
{
  template <typename T>
  using Func = std::function<T>;

  /// @brief A function that can only be moved, so it can own what can only be moved (a `UniquePtr`, a list
  /// moved into it), as a job or a task posted to another thread does.
  template <typename T>
  using MoveOnlyFunc = std::move_only_function<T>;
}