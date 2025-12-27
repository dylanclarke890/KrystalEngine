#pragma once

#include <memory>
#include <type_traits>

namespace Krys
{
  /// @brief Owns the memory it points to.
  /// @tparam T The underlying type of the data being pointed to.
  template <typename T, typename TDeleter = std::default_delete<T>>
  using Unique = std::unique_ptr<T, TDeleter>;

  /// @brief Shares the memory it points to via reference counting.
  /// @tparam T The underlying type of the data being pointed to.
  template <typename T>
  using Ref = std::shared_ptr<T>;

  /// @brief Weakly references a resource.
  /// @tparam T The underlying type of the data being pointed to.
  template <typename T>
  using WeakRef = std::weak_ptr<T>;

  /// @brief Non-owning pointer to a resource.
  /// @tparam T The underlying type of the data being pointed to.
  template <typename T>
  using Ptr = T *;

  template <typename T, typename... Args>
  constexpr Unique<T> CreateUnique(Args &&...args)
  {
    return std::make_unique<T>(std::forward<Args>(args)...);
  }

  template <typename T, typename... Args>
  constexpr Ref<T> CreateRef(Args &&...args)
  {
    return std::make_shared<T>(std::forward<Args>(args)...);
  }
}