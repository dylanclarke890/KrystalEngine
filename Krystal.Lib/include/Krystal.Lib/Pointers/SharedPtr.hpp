#pragma once

#include <memory>

namespace Krys
{
  /// @brief Shares the memory it points to via reference counting.
  /// @tparam T The underlying type of the data being pointed to.
  template <typename T>
  using SharedPtr = std::shared_ptr<T>;

  /// @brief Weakly points to a shared ptr resource.
  /// @tparam T The underlying type of the data being pointed to.
  template <typename T>
  using WeakSharedPtr = std::weak_ptr<T>;

  template <typename T, typename... Args>
  constexpr SharedPtr<T> CreateSharedPtr(Args &&...args)
  {
    return std::make_shared<T>(std::forward<Args>(args)...);
  }
}