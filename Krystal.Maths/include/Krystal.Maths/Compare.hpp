#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Maths
{

#pragma region Helper Macros

#define COMPARISON_FUNC(Type, FuncName, OP, TemplateParams)                                                  \
  template <TemplateParams>                                                                                  \
  KRYS_NODISCARD constexpr auto FuncName(const Type &a, const Type &b) noexcept                                  \
  {                                                                                                          \
    return Zip(a, b, [](auto x, auto y) -> bool { return x OP y; });                                         \
  }                                                                                                          \
  template <TemplateParams>                                                                                  \
  KRYS_NODISCARD constexpr bool FuncName(const Type &a, T b) noexcept                                            \
  {                                                                                                          \
    return AllOf(a, [&b](auto v) { return v OP b; });                                                        \
  }

#define MATRIX_COMPARISON_FUNC(FuncName, OP)                                                                 \
  COMPARISON_FUNC(MATRIX_TYPE, FuncName, OP, MATRIX_TEMPLATE_PARAMS)

#define VECTOR_COMPARISON_FUNC(FuncName, OP)                                                                 \
  COMPARISON_FUNC(VECTOR_TYPE, FuncName, OP, VECTOR_TEMPLATE_PARAMS)

#pragma endregion

  VECTOR_COMPARISON_FUNC(LessThan, <);
  VECTOR_COMPARISON_FUNC(LessThanOrEqual, <=);
  VECTOR_COMPARISON_FUNC(GreaterThan, >);
  VECTOR_COMPARISON_FUNC(GreaterThanOrEqual, >=);
  VECTOR_COMPARISON_FUNC(Equal, ==);
  VECTOR_COMPARISON_FUNC(NotEqual, !=);

  MATRIX_COMPARISON_FUNC(LessThan, <);
  MATRIX_COMPARISON_FUNC(LessThanOrEqual, <=);
  MATRIX_COMPARISON_FUNC(GreaterThan, >);
  MATRIX_COMPARISON_FUNC(GreaterThanOrEqual, >=);
  MATRIX_COMPARISON_FUNC(Equal, ==);
  MATRIX_COMPARISON_FUNC(NotEqual, !=);

#undef COMPARISON_FUNC
#undef MATRIX_COMPARISON_FUNC
#undef VECTOR_COMPARISON_FUNC
}