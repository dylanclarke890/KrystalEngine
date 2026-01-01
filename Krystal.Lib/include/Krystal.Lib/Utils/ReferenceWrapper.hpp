#pragma once

#include <type_traits>

namespace Krys
{
  /// @brief A type that wraps a reference to an object of type T.
  template <typename T>
  using ReferenceWrapper = std::reference_wrapper<T>;

  template <typename T>
  using reference_wrapper_t = typename ReferenceWrapper<T>::type;
}