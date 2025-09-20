#pragma once

#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Maths
{
#pragma region Helper Macros

#define VECTOR_ROUND_FUNC(FuncName)                                                                          \
  template <FloatingPoint T, int N>                                                                          \
  NO_DISCARD constexpr auto FuncName(const VECTOR_TYPE &x) noexcept                                          \
  {                                                                                                          \
    return MapEach(x, [](auto v) { return FuncName(v); });                                                   \
  }

#define MATRIX_ROUND_FUNC(FuncName)                                                                          \
  template <FloatingPoint T, int R, int C>                                                                   \
  NO_DISCARD constexpr auto FuncName(const MATRIX_TYPE &x) noexcept                                          \
  {                                                                                                          \
    return MapEach(x, [](auto v) { return FuncName(v); });                                                   \
  }

#pragma endregion

  template <FloatingPoint T>
  NO_DISCARD constexpr T Ceil(T x) noexcept
  {
    if (IS_COMPILE_TIME)
    {
      if (static_cast<long long>(x) == x)
        return x;

      return static_cast<long long>(x) + ((x < T(0)) ? T(0) : T(1));
    }
    else
      return std::ceil(x);
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T Floor(T x) noexcept
  {
    if (IS_COMPILE_TIME)
    {
      if (static_cast<long long>(x) == x)
        return x;
      else
        return static_cast<T>(static_cast<long long>(x) - ((x < T(0)) ? 1LL : 0LL));
    }
    else
      return std::floor(x);
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T Round(T x) noexcept
  {
    if (IS_COMPILE_TIME)
      return T(static_cast<long long>(x + (x > T(0) ? T(0.5) : T(-0.5))));
    else
      return std::round(x);
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr T Trunc(T x) noexcept
  {
    if (IS_COMPILE_TIME)
      return T(static_cast<long long>(x));
    else
      return std::trunc(x);
  }

  VECTOR_ROUND_FUNC(Ceil);
  VECTOR_ROUND_FUNC(Floor);
  VECTOR_ROUND_FUNC(Round);
  VECTOR_ROUND_FUNC(Trunc);

  MATRIX_ROUND_FUNC(Ceil);
  MATRIX_ROUND_FUNC(Floor);
  MATRIX_ROUND_FUNC(Round);
  MATRIX_ROUND_FUNC(Trunc);

#undef VECTOR_ROUND_FUNC
#undef MATRIX_ROUND_FUNC
}