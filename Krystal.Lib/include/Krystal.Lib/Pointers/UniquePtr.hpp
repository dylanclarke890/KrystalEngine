#pragma once

#include <memory>

namespace Krys
{
  /// @brief Owns the memory it points to.
  /// @tparam T The underlying type of the data being pointed to.
  template <typename T, typename TDeleter = std::default_delete<T>>
  using UniquePtr = std::unique_ptr<T, TDeleter>;

  template <typename T, typename... Args>
  constexpr UniquePtr<T> CreateUnique(Args &&...args)
  {
    return std::make_unique<T>(std::forward<Args>(args)...);
  }
}