#pragma once

#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Maths
{
#pragma region Helper Macros

#define VECTOR_ROUND_FUNC(FuncName)                                                                          \
  template <FloatingPoint T, int N>                                                                          \
  KRYS_NODISCARD constexpr auto FuncName(const VECTOR_TYPE &x) noexcept                                          \
  {                                                                                                          \
    return MapEach(x, [](auto v) { return FuncName(v); });                                                   \
  }

#define MATRIX_ROUND_FUNC(FuncName)                                                                          \
  template <FloatingPoint T, int R, int C>                                                                   \
  KRYS_NODISCARD constexpr auto FuncName(const MATRIX_TYPE &x) noexcept                                          \
  {                                                                                                          \
    return MapEach(x, [](auto v) { return FuncName(v); });                                                   \
  }

#pragma endregion

  template <FloatingPoint T>
  KRYS_NODISCARD constexpr T Ceil(T x) noexcept
  {
    if (std::is_constant_evaluated())
    {
      if (static_cast<long long>(x) == x)
        return x;

      return static_cast<long long>(x) + ((x < T(0)) ? T(0) : T(1));
    }
    else
      return std::ceil(x);
  }

  template <FloatingPoint T>
  KRYS_NODISCARD constexpr T Floor(T x) noexcept
  {
    if (std::is_constant_evaluated())
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
  KRYS_NODISCARD constexpr T Round(T x) noexcept
  {
    if (std::is_constant_evaluated())
      return T(static_cast<long long>(x + (x > T(0) ? T(0.5) : T(-0.5))));
    else
      return std::round(x);
  }

  template <FloatingPoint T>
  KRYS_NODISCARD constexpr T Trunc(T x) noexcept
  {
    if (std::is_constant_evaluated())
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