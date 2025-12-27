#pragma once

#include "Krystal.Lib/Core/Hash.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Vector.hpp"
#include <functional>

namespace std
{
  template <VECTOR_TEMPLATE_PARAMS>
  struct hash<VECTOR_TYPE>
  {
    size_t operator()(const VECTOR_TYPE &v) const noexcept
    {
      if constexpr (N == 1)
      {
        return std::hash<T> {}(v.x);
      }
      else if constexpr (N == 2)
      {
        return Krys::Hash::Combine(v.x, v.y);
      }
      else if constexpr (N == 3)
      {
        return Krys::Hash::Combine(v.x, v.y, v.z);
      }
      else if constexpr (N == 4)
      {
        return Krys::Hash::Combine(v.x, v.y, v.z, v.w);
      }
    };
  };

  template <MATRIX_TEMPLATE_PARAMS>
  struct hash<MATRIX_TYPE>
  {
    size_t operator()(const MATRIX_TYPE &m) const noexcept
    {
      size_t hash = 0;
      for (size_t i = 0; i < R; i++)
        for (size_t j = 0; j < C; j++)
          hash = Krys::Hash::Combine(hash, m[i][j]);

      return hash;
    };
  };
}