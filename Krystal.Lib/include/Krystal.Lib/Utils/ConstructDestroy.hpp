#pragma once

#include <iterator>
#include <memory>
#include <utility>

namespace Krys
{
  namespace Impl
  {
    template <typename T, typename... TArgs>
    constexpr bool IsConstructAtNoexcept() noexcept
    {
      return noexcept(std::construct_at(std::declval<T *>(), std::declval<TArgs>()...));
    }

    template <typename T>
    constexpr bool IsDestroyAtNoexcept() noexcept
    {
      return noexcept(std::destroy_at(std::declval<T *>()));
    }
  }

  /// @brief Constructs an element of `T` and the given `ptr` location, using forwarded `args...`.
  /// @tparam T The type of the pointer to destroy.
  /// @tparam TArgs The argument types, if any, to use to construct the pointed-to type.
  /// @param[in] ptr Location for the value to be constructed.
  /// @param[in] args The arguments, if any, to use to cosntruct the pointed-to type.
  /// @remarks There is currently no way to specify default-init with this paradigm, potentially resulting in
  /// lost performace for niche use cases (such as indeterminate initialization and partial setting for
  /// integral types used for bit vector implementations or similar constructs.)
  template <typename T, typename... TArgs>
  constexpr T *construct_at(T *ptr, TArgs &&...args) noexcept(Impl::IsConstructAtNoexcept<T, TArgs...>())
  {
    return std::construct_at(ptr, std::forward<TArgs>(args)...);
  }

  /// @brief Destroys an object of type `T` at the given location `_ptr`.
  /// @tparam T The type of the pointer to destroy.
  /// @param[in] ptr Location for the value to be destroyed.
  /// @remarks For arrays, each element will be destroyed, including recursively into other C-array types.
  template <typename T>
  constexpr void destroy_at(T *ptr) noexcept(Impl::IsDestroyAtNoexcept<T>())
  {
    return std::destroy_at(ptr);
  }
}
