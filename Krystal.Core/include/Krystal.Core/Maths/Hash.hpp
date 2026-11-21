#pragma once

#include "Krystal.Core/Hashing/FastHash.hpp"
#include "Krystal.Core/Maths/Matrix.hpp"
#include "Krystal.Core/Maths/Vector.hpp"

namespace krys
{
  template <VECTOR_TEMPLATE_PARAMS>
  struct FastHash<VECTOR_TYPE>
  {
    KRYS_NODISCARD size_t operator()(const VECTOR_TYPE &v) const noexcept
    {
      if constexpr (N == 1)
      {
        return FastHash<T> {}(v.x);
      }
      else if constexpr (N == 2)
      {
        return CombineFastHashes(v.x, v.y);
      }
      else if constexpr (N == 3)
      {
        return CombineFastHashes(v.x, v.y, v.z);
      }
      else if constexpr (N == 4)
      {
        return CombineFastHashes(v.x, v.y, v.z, v.w);
      }
    }
  };

  template <MATRIX_TEMPLATE_PARAMS>
  struct FastHash<MATRIX_TYPE>
  {
    KRYS_NODISCARD size_t operator()(const MATRIX_TYPE &m) const noexcept
    {
      size_t hash = 0;
      for (size_t i = 0; i < R; i++)
      {
        for (size_t j = 0; j < C; j++)
        {
          hash = CombineFastHashes(hash, m[i][j]);
        }
      }

      return hash;
    }
  };
}
