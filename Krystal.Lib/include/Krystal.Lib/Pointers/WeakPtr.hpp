#pragma once

#include <memory>

namespace Krys
{
  /// @brief Weakly points to a resource.
  /// @tparam T The underlying type of the data being pointed to.
  template <typename T>
  using WeakPtr = std::weak_ptr<T>;
}