#pragma once

#include "Krystal.Core/Hashing/FastHash.hpp"
#include "Krystal.Core/Maths/Matrix.hpp"
#include "Krystal.Core/Maths/Vector.hpp"

namespace krys
{
  template <Number T, int N>
  struct FastHash<Vector<T, N>>
  {
    KRYS_NODISCARD size_t operator()(const Vector<T, N> &v) const noexcept
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

  template <Number T, int C, int R>
  struct FastHash<Matrix<T, C, R>>
  {
    KRYS_NODISCARD size_t operator()(const Matrix<T, C, R> &m) const noexcept
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
