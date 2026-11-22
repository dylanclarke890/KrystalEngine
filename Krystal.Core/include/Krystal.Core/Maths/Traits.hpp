#pragma once

#include "Krystal.Core/Concepts.hpp"

namespace krys
{
  template <typename T>
  struct Traits;

  template <typename T>
  struct Traits
  {
    using ComponentType = T;

    constexpr static bool IsVector = false;
    constexpr static bool IsMatrix = false;

    constexpr static size_t TotalComponents = 0;
    constexpr static size_t Rows = 0;
    constexpr static size_t Columns = 0;
  };

  /// @brief A vector or matrix: a type whose components the component-wise algorithms (`MapEach`, `Zip`,
  /// `AllOf` and so on) work over.
  template <typename T>
  concept VectorOrMatrix = Traits<T>::IsVector || Traits<T>::IsMatrix;
}
