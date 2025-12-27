#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Maths/Traits.hpp"
#include "Krystal.Maths/Vector.hpp"
#include <cassert>
#include <type_traits>

#pragma region Helper Macros

#define MATRIX_TEMPLATE_PARAMS Krys::Number T, int C, int R
#define MATRIX_TEMPLATE_ARGS T, C, R
#define MATRIX_TYPE Krys::Maths::Matrix<MATRIX_TEMPLATE_ARGS>

#define MATRIX_STATIC_ASSERT(condition) static_assert(condition, "Unsupported matrix length.")

#define MATRIX_VALID_LENGTH_CONDITION(C, R) (C > 1 && C < 5 && R > 1 && R < 5)

#define MATRIX_TYPE_ALIASES(Columns, Rows)                                                                   \
  template <Number T>                                                                                        \
  using Matrix##Columns##x##Rows = Matrix<T, Columns, Rows>;                                                 \
  using Mat##Columns##x##Rows = Matrix<float, Columns, Rows>;                                                \
  using Mat##Columns##x##Rows##b = Matrix<bool, Columns, Rows>;                                              \
  using Mat##Columns##x##Rows##f = Matrix<float, Columns, Rows>;                                             \
  using Mat##Columns##x##Rows##d = Matrix<double, Columns, Rows>;                                            \
  using Mat##Columns##x##Rows##i = Matrix<int, Columns, Rows>;                                               \
  using Mat##Columns##x##Rows##u = Matrix<unsigned int, Columns, Rows>;

#define MATRIX_INTROSPECTION(columns, rows)                                                                  \
  static constexpr int Rows = rows;                                                                          \
  static constexpr int Columns = columns;                                                                    \
  using Type = Matrix<T, Columns, Rows>;                                                                     \
  using ComponentType = T;                                                                                   \
  using RowType = Vector<T, Columns>;                                                                        \
  using ColumnType = Vector<T, Rows>;                                                                        \
  using TransposeType = Matrix<T, Rows, Columns>;                                                            \
                                                                                                             \
private:                                                                                                     \
  ColumnType _data[Columns];

#define MATRIX_BINARY_OPERATOR(OP)                                                                           \
  template <MATRIX_TEMPLATE_PARAMS>                                                                          \
  KRYS_NODISCARD constexpr auto KRYS_CONCAT(operator, OP)(const MATRIX_TYPE &lhs,                            \
                                                          const MATRIX_TYPE &rhs) noexcept                   \
  {                                                                                                          \
    return Zip(lhs, rhs, [](auto l, auto r) { return l OP r; });                                             \
  }                                                                                                          \
  template <MATRIX_TEMPLATE_PARAMS>                                                                          \
  KRYS_NODISCARD constexpr auto KRYS_CONCAT(operator, OP)(const MATRIX_TYPE &lhs, T scalar) noexcept         \
  {                                                                                                          \
    return MapEach(lhs, [&](auto v) { return v OP scalar; });                                                \
  }

#define MATRIX_OPERATOR_ASSIGNMENT(OP)                                                                       \
  constexpr auto &KRYS_CONCAT(operator, KRYS_CONCAT(OP, =))(const Matrix &other) noexcept                    \
  {                                                                                                          \
    *this = *this OP other;                                                                                  \
    return *this;                                                                                            \
  }                                                                                                          \
  constexpr auto &KRYS_CONCAT(operator, KRYS_CONCAT(OP, =))(T scalar) noexcept                               \
  {                                                                                                          \
    *this = *this OP scalar;                                                                                 \
    return *this;                                                                                            \
  }

#define MATRIX_UNARY_OPERATOR(OP)                                                                            \
  template <MATRIX_TEMPLATE_PARAMS>                                                                          \
  KRYS_NODISCARD constexpr auto KRYS_CONCAT(operator, OP)(const MATRIX_TYPE &rhs) noexcept                   \
  {                                                                                                          \
    return MapEach(rhs, [](auto v) { return OP v; });                                                        \
  }

#define MATRIX_COMMON_MEMBER_FUNCTIONS()                                                                     \
  constexpr Matrix() noexcept = default;                                                                     \
  constexpr ~Matrix() noexcept = default;                                                                    \
  constexpr Matrix(const Matrix &) noexcept = default;                                                       \
  constexpr Matrix(Matrix &&) noexcept = default;                                                            \
  constexpr Matrix &operator=(const Matrix &) noexcept = default;                                            \
  constexpr Matrix &operator=(Matrix &&) noexcept = default;                                                 \
  constexpr auto operator<=>(const Matrix &) const noexcept = default;                                       \
  KRYS_NODISCARD constexpr ColumnType &operator[](int column) noexcept                                       \
  {                                                                                                          \
    assert(column >= 0 && column < Columns);                                                                 \
    return _data[column];                                                                                    \
  }                                                                                                          \
  KRYS_NODISCARD constexpr ColumnType const &operator[](int column) const noexcept                           \
  {                                                                                                          \
    assert(column >= 0 && column < Columns);                                                                 \
    return _data[column];                                                                                    \
  }                                                                                                          \
  MATRIX_OPERATOR_ASSIGNMENT(+)                                                                              \
  MATRIX_OPERATOR_ASSIGNMENT(-)                                                                              \
  MATRIX_OPERATOR_ASSIGNMENT(*)                                                                              \
  MATRIX_OPERATOR_ASSIGNMENT(/)                                                                              \
  MATRIX_OPERATOR_ASSIGNMENT(>>)                                                                             \
  MATRIX_OPERATOR_ASSIGNMENT(<<)                                                                             \
  MATRIX_OPERATOR_ASSIGNMENT(|)                                                                              \
  MATRIX_OPERATOR_ASSIGNMENT(&)                                                                              \
  MATRIX_OPERATOR_ASSIGNMENT(^)

#pragma endregion

namespace Krys::Maths
{
  template <MATRIX_TEMPLATE_PARAMS>
  struct Matrix;

  template <Number T>
  struct Matrix<T, 2, 2>
  {
    MATRIX_INTROSPECTION(2, 2)

  public:
    constexpr explicit Matrix(const T &value) noexcept : _data {ColumnType(value), ColumnType(value)}
    {
    }

    constexpr Matrix(const ColumnType &col0, const ColumnType &col1) noexcept : _data {col0, col1}
    {
    }

    constexpr Matrix(const T &m00, const T &m01, const T &m10, const T &m11) noexcept
        : _data {ColumnType(m00, m01), ColumnType(m10, m11)}
    {
    }

    MATRIX_COMMON_MEMBER_FUNCTIONS()
  };

  template <Number T>
  struct Matrix<T, 2, 3>
  {
    MATRIX_INTROSPECTION(2, 3)

  public:
    constexpr explicit Matrix(T value) noexcept : _data {ColumnType(value), ColumnType(value)}
    {
    }

    constexpr Matrix(const ColumnType &col0, const ColumnType &col1) noexcept : _data {col0, col1}
    {
    }

    constexpr Matrix(const T &m00, const T &m01, const T &m02, const T &m10, const T &m11,
                     const T &m12) noexcept
        : _data {ColumnType(m00, m01, m02), ColumnType(m10, m11, m12)}
    {
    }

    MATRIX_COMMON_MEMBER_FUNCTIONS()
  };

  template <Number T>
  struct Matrix<T, 2, 4>
  {
    MATRIX_INTROSPECTION(2, 4)

  public:
    constexpr explicit Matrix(T value) noexcept : _data {ColumnType(value), ColumnType(value)}
    {
    }

    constexpr Matrix(const ColumnType &col0, const ColumnType &col1) noexcept : _data {col0, col1}
    {
    }

    constexpr Matrix(const T &m00, const T &m01, const T &m02, const T &m03, const T &m10, const T &m11,
                     const T &m12, const T &m13) noexcept
        : _data {ColumnType(m00, m01, m02, m03), ColumnType(m10, m11, m12, m13)}
    {
    }

    MATRIX_COMMON_MEMBER_FUNCTIONS()
  };

  template <Number T>
  struct Matrix<T, 3, 2>
  {
    MATRIX_INTROSPECTION(3, 2)

  public:
    constexpr explicit Matrix(T value) noexcept
        : _data {ColumnType(value), ColumnType(value), ColumnType(value)}
    {
    }

    constexpr Matrix(const ColumnType &col0, const ColumnType &col1, const ColumnType &col2) noexcept
        : _data {col0, col1, col2}
    {
    }

    constexpr Matrix(const T &m00, const T &m01, const T &m10, const T &m11, const T &m20,
                     const T &m21) noexcept
        : _data {ColumnType(m00, m01), ColumnType(m10, m11), ColumnType(m20, m21)}
    {
    }

    MATRIX_COMMON_MEMBER_FUNCTIONS()
  };

  template <Number T>
  struct Matrix<T, 3, 3>
  {
    MATRIX_INTROSPECTION(3, 3)

  public:
    constexpr explicit Matrix(T value) noexcept
        : _data {ColumnType(value), ColumnType(value), ColumnType(value)}
    {
    }

    constexpr Matrix(const ColumnType &col0, const ColumnType &col1, const ColumnType &col2) noexcept
        : _data {col0, col1, col2}
    {
    }

    constexpr Matrix(const T &m00, const T &m01, const T &m02, const T &m10, const T &m11, const T &m12,
                     const T &m20, const T &m21, const T &m22) noexcept
        : _data {ColumnType(m00, m01, m02), ColumnType(m10, m11, m12), ColumnType(m20, m21, m22)}
    {
    }

    constexpr Matrix(const Matrix<T, 4, 4> &other) noexcept
        : _data {ColumnType(other[0]), ColumnType(other[1]), ColumnType(other[2])}
    {
    }

    MATRIX_COMMON_MEMBER_FUNCTIONS()
  };

  template <Number T>
  struct Matrix<T, 3, 4>
  {
    MATRIX_INTROSPECTION(3, 4)

  public:
    constexpr explicit Matrix(T value) noexcept
        : _data {ColumnType(value), ColumnType(value), ColumnType(value)}
    {
    }

    constexpr Matrix(const ColumnType &col0, const ColumnType &col1, const ColumnType &col2) noexcept
        : _data {col0, col1, col2}
    {
    }

    constexpr Matrix(const T &m00, const T &m01, const T &m02, const T &m03, const T &m10, const T &m11,
                     const T &m12, const T &m13, const T &m20, const T &m21, const T &m22,
                     const T &m23) noexcept
        : _data {ColumnType(m00, m01, m02, m03), ColumnType(m10, m11, m12, m13),
                 ColumnType(m20, m21, m22, m23)}
    {
    }

    MATRIX_COMMON_MEMBER_FUNCTIONS()
  };

  template <Number T>
  struct Matrix<T, 4, 2>
  {
    MATRIX_INTROSPECTION(4, 2)

  public:
    constexpr explicit Matrix(T value) noexcept
        : _data {ColumnType(value), ColumnType(value), ColumnType(value), ColumnType(value)}
    {
    }

    constexpr Matrix(const ColumnType &col0, const ColumnType &col1, const ColumnType &col2,
                     const ColumnType &col3) noexcept
        : _data {col0, col1, col2, col3}
    {
    }

    constexpr Matrix(const T &m00, const T &m01, const T &m10, const T &m11, const T &m20, const T &m21,
                     const T &m30, const T &m31) noexcept
        : _data {ColumnType(m00, m01), ColumnType(m10, m11), ColumnType(m20, m21), ColumnType(m30, m31)}
    {
    }

    MATRIX_COMMON_MEMBER_FUNCTIONS()
  };

  template <Number T>
  struct Matrix<T, 4, 3>
  {
    MATRIX_INTROSPECTION(4, 3)

  public:
    constexpr explicit Matrix(T value) noexcept
        : _data {ColumnType(value), ColumnType(value), ColumnType(value), ColumnType(value)}
    {
    }

    constexpr Matrix(const ColumnType &col0, const ColumnType &col1, const ColumnType &col2,
                     const ColumnType &col3) noexcept
        : _data {col0, col1, col2, col3}
    {
    }

    constexpr Matrix(const T &m00, const T &m01, const T &m02, const T &m10, const T &m11, const T &m12,
                     const T &m20, const T &m21, const T &m22, const T &m30, const T &m31,
                     const T &m32) noexcept
        : _data {ColumnType(m00, m01, m02), ColumnType(m10, m11, m12), ColumnType(m20, m21, m22),
                 ColumnType(m30, m31, m32)}
    {
    }

    MATRIX_COMMON_MEMBER_FUNCTIONS()
  };

  template <Number T>
  struct Matrix<T, 4, 4>
  {
    MATRIX_INTROSPECTION(4, 4)

  public:
    constexpr explicit Matrix(T value) noexcept
        : _data {ColumnType(value), ColumnType(value), ColumnType(value), ColumnType(value)}
    {
    }

    constexpr Matrix(const ColumnType &col0, const ColumnType &col1, const ColumnType &col2,
                     const ColumnType &col3) noexcept
        : _data {col0, col1, col2, col3}
    {
    }

    constexpr Matrix(const T &m00, const T &m01, const T &m02, const T &m03, const T &m10, const T &m11,
                     const T &m12, const T &m13, const T &m20, const T &m21, const T &m22, const T &m23,
                     const T &m30, const T &m31, const T &m32, const T &m33) noexcept
        : _data {ColumnType(m00, m01, m02, m03), ColumnType(m10, m11, m12, m13),
                 ColumnType(m20, m21, m22, m23), ColumnType(m30, m31, m32, m33)}
    {
    }

    MATRIX_COMMON_MEMBER_FUNCTIONS()
  };

  template <MATRIX_TEMPLATE_PARAMS>
  struct Traits<MATRIX_TYPE>
  {
    using ComponentType = T;
    static constexpr bool IsVector = false;
    static constexpr bool IsMatrix = true;

    static constexpr int TotalComponents = C * R;
    static constexpr int Columns = C;
    static constexpr int Rows = R;
  };

  template <typename T>
  concept MatrixType = Traits<T>::IsMatrix;

  MATRIX_TYPE_ALIASES(2, 2);
  MATRIX_TYPE_ALIASES(2, 3);
  MATRIX_TYPE_ALIASES(2, 4);
  MATRIX_TYPE_ALIASES(3, 2);
  MATRIX_TYPE_ALIASES(3, 3);
  MATRIX_TYPE_ALIASES(3, 4);
  MATRIX_TYPE_ALIASES(4, 2);
  MATRIX_TYPE_ALIASES(4, 3);
  MATRIX_TYPE_ALIASES(4, 4);
  using Mat2 = Matrix<float, 2, 2>;
  using Mat3 = Matrix<float, 3, 3>;
  using Mat4 = Matrix<float, 4, 4>;

  /// @brief Creates an identity matrix of the specified size.
  template <MatrixType Mat>
  KRYS_NODISCARD constexpr auto Identity() noexcept
  {
    constexpr int C = Traits<Mat>::Columns;
    constexpr int R = Traits<Mat>::Rows;
    using T = typename Traits<Mat>::ComponentType;

    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(C, R));
    MATRIX_STATIC_ASSERT(C == R);

    if constexpr (C == 2 && R == 2)
    {
      return Mat(T(1), T(0), T(0), T(1));
    }
    else if constexpr (C == 3 && R == 3)
    {
      return Mat(T(1), T(0), T(0), T(0), T(1), T(0), T(0), T(0), T(1));
    }
    else if constexpr (C == 4 && R == 4)
    {
      return Mat(T(1), T(0), T(0), T(0), T(0), T(1), T(0), T(0), T(0), T(0), T(1), T(0), T(0), T(0), T(0),
                 T(1));
    }
  }

  /// @brief Map each component of the matrix using the provided function.
  template <MATRIX_TEMPLATE_PARAMS, RegularCallable<T> F>
  KRYS_NODISCARD constexpr auto MapEach(const MATRIX_TYPE &mat, const F &func) noexcept
  {
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(C, R));

    using U = std::invoke_result_t<F, T>;
    using Mat = Matrix<U, C, R>;

    if constexpr (C == 2)
    {
      auto col0 = MapEach(mat[0], func);
      auto col1 = MapEach(mat[1], func);
      return Mat(col0, col1);
    }
    else if constexpr (C == 3)
    {
      auto col0 = MapEach(mat[0], func);
      auto col1 = MapEach(mat[1], func);
      auto col2 = MapEach(mat[2], func);
      return Mat(col0, col1, col2);
    }
    else if constexpr (C == 4)
    {
      auto col0 = MapEach(mat[0], func);
      auto col1 = MapEach(mat[1], func);
      auto col2 = MapEach(mat[2], func);
      auto col3 = MapEach(mat[3], func);
      return Mat(col0, col1, col2, col3);
    }
    else
    {
      MATRIX_STATIC_ASSERT(false);
    }
  }

  /// @brief Apply a function to each component of the matrix.
  template <MATRIX_TEMPLATE_PARAMS, RegularCallable<T> F>
  KRYS_NODISCARD constexpr void ForEach(const MATRIX_TYPE &mat, const F &func) noexcept
  {
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(C, R));

    if constexpr (C == 2)
    {
      ForEach(mat[0], func);
      ForEach(mat[1], func);
    }
    else if constexpr (C == 3)
    {
      ForEach(mat[0], func);
      ForEach(mat[1], func);
      ForEach(mat[2], func);
    }
    else if constexpr (C == 4)
    {
      ForEach(mat[0], func);
      ForEach(mat[1], func);
      ForEach(mat[2], func);
      ForEach(mat[3], func);
    }
    else
    {
      MATRIX_STATIC_ASSERT(false);
    }
  }

  /// @brief Zip two matrices together using the provided function.
  template <MATRIX_TEMPLATE_PARAMS, RegularCallable<T, T> F>
  KRYS_NODISCARD constexpr auto Zip(const MATRIX_TYPE &lhs, const MATRIX_TYPE &rhs, const F &func) noexcept
  {
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(C, R));

    using U = std::invoke_result_t<F, T, T>;
    using Mat = Matrix<U, C, R>;

    if constexpr (C == 2)
    {
      auto col0 = Zip(lhs[0], rhs[0], func);
      auto col1 = Zip(lhs[1], rhs[1], func);
      return Mat(col0, col1);
    }
    else if constexpr (C == 3)
    {
      auto col0 = Zip(lhs[0], rhs[0], func);
      auto col1 = Zip(lhs[1], rhs[1], func);
      auto col2 = Zip(lhs[2], rhs[2], func);
      return Mat(col0, col1, col2);
    }
    else if constexpr (C == 4)
    {
      auto col0 = Zip(lhs[0], rhs[0], func);
      auto col1 = Zip(lhs[1], rhs[1], func);
      auto col2 = Zip(lhs[2], rhs[2], func);
      auto col3 = Zip(lhs[3], rhs[3], func);
      return Mat(col0, col1, col2, col3);
    }
    else
    {
      MATRIX_STATIC_ASSERT(false);
    }
  }

  /// @brief Zip three matrices together using the provided function.
  template <MATRIX_TEMPLATE_PARAMS, RegularCallable<T, T, T> F>
  KRYS_NODISCARD constexpr auto Zip(const MATRIX_TYPE &a, const MATRIX_TYPE &b, const MATRIX_TYPE &c,
                                    const F &func) noexcept
  {
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(C, R));

    using U = std::invoke_result_t<F, T, T, T>;
    using Mat = Matrix<U, C, R>;

    if constexpr (C == 2)
    {
      auto col0 = Zip(a[0], b[0], c[0], func);
      auto col1 = Zip(a[1], b[1], c[1], func);
      return Mat(col0, col1);
    }
    else if constexpr (C == 3)
    {
      auto col0 = Zip(a[0], b[0], c[0], func);
      auto col1 = Zip(a[1], b[1], c[1], func);
      auto col2 = Zip(a[2], b[2], c[2], func);
      return Mat(col0, col1, col2);
    }
    else if constexpr (C == 4)
    {
      auto col0 = Zip(a[0], b[0], c[0], func);
      auto col1 = Zip(a[1], b[1], c[1], func);
      auto col2 = Zip(a[2], b[2], c[2], func);
      auto col3 = Zip(a[3], b[3], c[3], func);
      return Mat(col0, col1, col2, col3);
    }
    else
    {
      MATRIX_STATIC_ASSERT(false);
    }
  }

  /// @brief Computes the sum of all components of the matrix.
  template <MATRIX_TEMPLATE_PARAMS>
  KRYS_NODISCARD constexpr T Sum(const MATRIX_TYPE &m) noexcept
  {
    T sum = T(0);
    ForEach(m, [&sum](auto val) { sum += val; });
    return sum;
  }

  /// @brief Computes the sum of all components of the matrix after applying a function to each component.
  template <MATRIX_TEMPLATE_PARAMS, RegularCallable<T> F>
  KRYS_NODISCARD constexpr auto Sum(const MATRIX_TYPE &m, const F &func) noexcept
  {
    using U = std::invoke_result_t<F, T>;

    U sum = U(0);
    ForEach(m, [&sum, &func](auto val) { sum += func(val); });
    return sum;
  }

  /// @brief Checks if any of the components in `v` satisfy the predicate.
  template <MATRIX_TEMPLATE_PARAMS, Predicate<T> Func>
  constexpr bool AnyOf(const MATRIX_TYPE &v, const Func &predicate) noexcept
  {
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(C, R));

    if constexpr (C == 2 && R == 2)
    {
      return predicate(v[0].x) || predicate(v[0].y) || predicate(v[1].x) || predicate(v[1].y);
    }
    else if constexpr (C == 2 && R == 3)
    {
      return predicate(v[0].x) || predicate(v[0].y) || predicate(v[0].z) || predicate(v[1].x)
             || predicate(v[1].y) || predicate(v[1].z);
    }
    else if constexpr (C == 2 && R == 4)
    {
      return predicate(v[0].x) || predicate(v[0].y) || predicate(v[0].z) || predicate(v[0].w)
             || predicate(v[1].x) || predicate(v[1].y) || predicate(v[1].z) || predicate(v[1].w);
    }
    else if constexpr (C == 3 && R == 2)
    {
      return predicate(v[0].x) || predicate(v[0].y) || predicate(v[1].x) || predicate(v[1].y)
             || predicate(v[2].x) || predicate(v[2].y);
    }
    else if constexpr (C == 3 && R == 3)
    {
      return predicate(v[0].x) || predicate(v[0].y) || predicate(v[0].z) || predicate(v[1].x)
             || predicate(v[1].y) || predicate(v[1].z) || predicate(v[2].x) || predicate(v[2].y)
             || predicate(v[2].z);
    }
    else if constexpr (C == 3 && R == 4)
    {
      return predicate(v[0].x) || predicate(v[0].y) || predicate(v[0].z) || predicate(v[0].w)
             || predicate(v[1].x) || predicate(v[1].y) || predicate(v[1].z) || predicate(v[1].w)
             || predicate(v[2].x) || predicate(v[2].y) || predicate(v[2].z) || predicate(v[2].w);
    }
    else if constexpr (C == 4 && R == 2)
    {
      return predicate(v[0].x) || predicate(v[0].y) || predicate(v[1].x) || predicate(v[1].y)
             || predicate(v[2].x) || predicate(v[2].y) || predicate(v[3].x) || predicate(v[3].y);
    }
    else if constexpr (C == 4 && R == 3)
    {
      return predicate(v[0].x) || predicate(v[0].y) || predicate(v[0].z) || predicate(v[1].x)
             || predicate(v[1].y) || predicate(v[1].z) || predicate(v[2].x) || predicate(v[2].y)
             || predicate(v[2].z) || predicate(v[3].x) || predicate(v[3].y) || predicate(v[3].z);
    }
    else if constexpr (C == 4 && R == 4)
    {
      return predicate(v[0].x) || predicate(v[0].y) || predicate(v[0].z) || predicate(v[0].w)
             || predicate(v[1].x) || predicate(v[1].y) || predicate(v[1].z) || predicate(v[1].w)
             || predicate(v[2].x) || predicate(v[2].y) || predicate(v[2].z) || predicate(v[2].w)
             || predicate(v[3].x) || predicate(v[3].y) || predicate(v[3].z) || predicate(v[3].w);
    }
  }

  /// @brief Checks if all of the components in `v` satisfy the predicate.
  template <MATRIX_TEMPLATE_PARAMS, Predicate<T> Func>
  constexpr bool AllOf(const MATRIX_TYPE &v, const Func &predicate) noexcept
  {
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(C, R));

    if constexpr (C == 2 && R == 2)
    {
      return predicate(v[0].x) && predicate(v[0].y) && predicate(v[1].x) && predicate(v[1].y);
    }
    else if constexpr (C == 2 && R == 3)
    {
      return predicate(v[0].x) && predicate(v[0].y) && predicate(v[0].z) && predicate(v[1].x)
             && predicate(v[1].y) && predicate(v[1].z);
    }
    else if constexpr (C == 2 && R == 4)
    {
      return predicate(v[0].x) && predicate(v[0].y) && predicate(v[0].z) && predicate(v[0].w)
             && predicate(v[1].x) && predicate(v[1].y) && predicate(v[1].z) && predicate(v[1].w);
    }
    else if constexpr (C == 3 && R == 2)
    {
      return predicate(v[0].x) && predicate(v[0].y) && predicate(v[1].x) && predicate(v[1].y)
             && predicate(v[2].x) && predicate(v[2].y);
    }
    else if constexpr (C == 3 && R == 3)
    {
      return predicate(v[0].x) && predicate(v[0].y) && predicate(v[0].z) && predicate(v[1].x)
             && predicate(v[1].y) && predicate(v[1].z) && predicate(v[2].x) && predicate(v[2].y)
             && predicate(v[2].z);
    }
    else if constexpr (C == 3 && R == 4)
    {
      return predicate(v[0].x) && predicate(v[0].y) && predicate(v[0].z) && predicate(v[0].w)
             && predicate(v[1].x) && predicate(v[1].y) && predicate(v[1].z) && predicate(v[1].w)
             && predicate(v[2].x) && predicate(v[2].y) && predicate(v[2].z) && predicate(v[2].w);
    }
    else if constexpr (C == 4 && R == 2)
    {
      return predicate(v[0].x) && predicate(v[0].y) && predicate(v[1].x) && predicate(v[1].y)
             && predicate(v[2].x) && predicate(v[2].y) && predicate(v[3].x) && predicate(v[3].y);
    }
    else if constexpr (C == 4 && R == 3)
    {
      return predicate(v[0].x) && predicate(v[0].y) && predicate(v[0].z) && predicate(v[1].x)
             && predicate(v[1].y) && predicate(v[1].z) && predicate(v[2].x) && predicate(v[2].y)
             && predicate(v[2].z) && predicate(v[3].x) && predicate(v[3].y) && predicate(v[3].z);
    }
    else if constexpr (C == 4 && R == 4)
    {
      return predicate(v[0].x) && predicate(v[0].y) && predicate(v[0].z) && predicate(v[0].w)
             && predicate(v[1].x) && predicate(v[1].y) && predicate(v[1].z) && predicate(v[1].w)
             && predicate(v[2].x) && predicate(v[2].y) && predicate(v[2].z) && predicate(v[2].w)
             && predicate(v[3].x) && predicate(v[3].y) && predicate(v[3].z) && predicate(v[3].w);
    }
  }

  /// @brief Checks if none of the components in `v` satisfy the predicate.
  template <MATRIX_TEMPLATE_PARAMS, Predicate<T> Func>
  constexpr bool NoneOf(const MATRIX_TYPE &v, const Func &predicate) noexcept
  {
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(C, R));

    if constexpr (C == 2 && R == 2)
    {
      return !predicate(v[0].x) && !predicate(v[0].y) && !predicate(v[1].x) && !predicate(v[1].y);
    }
    else if constexpr (C == 2 && R == 3)
    {
      return !predicate(v[0].x) && !predicate(v[0].y) && !predicate(v[0].z) && !predicate(v[1].x)
             && !predicate(v[1].y) && !predicate(v[1].z);
    }
    else if constexpr (C == 2 && R == 4)
    {
      return !predicate(v[0].x) && !predicate(v[0].y) && !predicate(v[0].z) && !predicate(v[0].w)
             && !predicate(v[1].x) && !predicate(v[1].y) && !predicate(v[1].z) && !predicate(v[1].w);
    }
    else if constexpr (C == 3 && R == 2)
    {
      return !predicate(v[0].x) && !predicate(v[0].y) && !predicate(v[1].x) && !predicate(v[1].y)
             && !predicate(v[2].x) && !predicate(v[2].y);
    }
    else if constexpr (C == 3 && R == 3)
    {
      return !predicate(v[0].x) && !predicate(v[0].y) && !predicate(v[0].z) && !predicate(v[1].x)
             && !predicate(v[1].y) && !predicate(v[1].z) && !predicate(v[2].x) && !predicate(v[2].y)
             && !predicate(v[2].z);
    }
    else if constexpr (C == 3 && R == 4)
    {
      return !predicate(v[0].x) && !predicate(v[0].y) && !predicate(v[0].z) && !predicate(v[0].w)
             && !predicate(v[1].x) && !predicate(v[1].y) && !predicate(v[1].z) && !predicate(v[1].w)
             && !predicate(v[2].x) && !predicate(v[2].y) && !predicate(v[2].z) && !predicate(v[2].w);
    }
    else if constexpr (C == 4 && R == 2)
    {
      return !predicate(v[0].x) && !predicate(v[0].y) && !predicate(v[1].x) && !predicate(v[1].y)
             && !predicate(v[2].x) && !predicate(v[2].y) && !predicate(v[3].x) && !predicate(v[3].y);
    }
    else if constexpr (C == 4 && R == 3)
    {
      return !predicate(v[0].x) && !predicate(v[0].y) && !predicate(v[0].z) && !predicate(v[1].x)
             && !predicate(v[1].y) && !predicate(v[1].z) && !predicate(v[2].x) && !predicate(v[2].y)
             && !predicate(v[2].z) && !predicate(v[3].x) && !predicate(v[3].y) && !predicate(v[3].z);
    }
    else if constexpr (C == 4 && R == 4)
    {
      return !predicate(v[0].x) && !predicate(v[0].y) && !predicate(v[0].z) && !predicate(v[0].w)
             && !predicate(v[1].x) && !predicate(v[1].y) && !predicate(v[1].z) && !predicate(v[1].w)
             && !predicate(v[2].x) && !predicate(v[2].y) && !predicate(v[2].z) && !predicate(v[2].w)
             && !predicate(v[3].x) && !predicate(v[3].y) && !predicate(v[3].z) && !predicate(v[3].w);
    }
  }

  MATRIX_BINARY_OPERATOR(+);
  MATRIX_BINARY_OPERATOR(-);
  MATRIX_BINARY_OPERATOR(/);
  MATRIX_BINARY_OPERATOR(>>);
  MATRIX_BINARY_OPERATOR(<<);
  MATRIX_BINARY_OPERATOR(|);
  MATRIX_BINARY_OPERATOR(&);
  MATRIX_BINARY_OPERATOR(^);

  MATRIX_UNARY_OPERATOR(-);
  MATRIX_UNARY_OPERATOR(+);
  MATRIX_UNARY_OPERATOR(~);

  template <MATRIX_TEMPLATE_PARAMS>
  KRYS_NODISCARD constexpr auto operator+(T scalar, const MATRIX_TYPE &mat) noexcept
  {
    return mat + scalar;
  }

  template <MATRIX_TEMPLATE_PARAMS>
  KRYS_NODISCARD constexpr auto operator*(T scalar, const MATRIX_TYPE &mat) noexcept
  {
    return mat * scalar;
  }

  template <MATRIX_TEMPLATE_PARAMS>
  constexpr auto operator*(const MATRIX_TYPE &lhs, const T &scalar) noexcept
  {
    return MapEach(lhs, [&](auto v) { return v * scalar; });
  }

  /// @brief Matrix multiplication. For component-wise multiplication, call Hadamard(lhs, rhs) instead.
  template <MATRIX_TEMPLATE_PARAMS, int P>
  constexpr auto operator*(const Matrix<T, C, R> &lhs, const Matrix<T, P, C> &rhs) noexcept
  {
    // Result has P columns and R rows (R x P).
    Matrix<T, P, R> out {T(0)};

    for (int col = 0; col < P; ++col)   // column of result / rhs
      for (int row = 0; row < R; ++row) // row of result / lhs
        for (int k = 0; k < C; ++k)     // inner dimension
          out[col][row] += lhs[k][row] * rhs[col][k];

    return out;
  }

  /// @brief Matrix multiplication with a vector.
  template <MATRIX_TEMPLATE_PARAMS>
  KRYS_NODISCARD constexpr auto operator*(const Matrix<T, C, R> &lhs, const Vector<T, C> &rhs) noexcept
  {
    Vector<T, R> result {T(0)};
    for (int row = 0; row < R; ++row)
      for (int k = 0; k < C; ++k)
        result[row] += lhs[k][row] * rhs[k];
    return result;
  }

  /// @brief Component-wise multiplication (Hadamard product) of two matrices.
  template <MATRIX_TEMPLATE_PARAMS>
  KRYS_NODISCARD constexpr auto Hadamard(const MATRIX_TYPE &lhs, const MATRIX_TYPE &rhs) noexcept
  {
    return Zip(lhs, rhs, [](auto x, auto y) { return x * y; });
  }

  /// @brief Computes the transpose of `m`.
  template <MATRIX_TEMPLATE_PARAMS>
  KRYS_NODISCARD constexpr auto Transpose(const MATRIX_TYPE &m) noexcept
  {
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(C, R));

    using Mat = typename Matrix<T, C, R>::TransposeType;
    using ColumnType = Mat::ColumnType;

    if constexpr (C == 2 && R == 2)
    {
      return Mat(ColumnType {m[0].x, m[1].x}, ColumnType {m[0].y, m[1].y});
    }
    else if constexpr (C == 2 && R == 3)
    {
      return Mat(ColumnType {m[0].x, m[1].x}, ColumnType {m[0].y, m[1].y}, ColumnType {m[0].z, m[1].z});
    }
    else if constexpr (C == 2 && R == 4)
    {
      return Mat(ColumnType {m[0].x, m[1].x}, ColumnType {m[0].y, m[1].y}, ColumnType {m[0].z, m[1].z},
                 ColumnType {m[0].w, m[1].w});
    }
    else if constexpr (C == 3 && R == 2)
    {
      return Mat(ColumnType {m[0].x, m[1].x, m[2].x}, ColumnType {m[0].y, m[1].y, m[2].y});
    }
    else if constexpr (C == 3 && R == 3)
    {
      return Mat(ColumnType {m[0].x, m[1].x, m[2].x}, ColumnType {m[0].y, m[1].y, m[2].y},
                 ColumnType {m[0].z, m[1].z, m[2].z});
    }
    else if constexpr (C == 3 && R == 4)
    {
      return Mat(ColumnType {m[0].x, m[1].x, m[2].x}, ColumnType {m[0].y, m[1].y, m[2].y},
                 ColumnType {m[0].z, m[1].z, m[2].z}, ColumnType {m[0].w, m[1].w, m[2].w});
    }
    else if constexpr (C == 4 && R == 2)
    {
      return Mat(ColumnType {m[0].x, m[1].x, m[2].x, m[3].x}, ColumnType {m[0].y, m[1].y, m[2].y, m[3].y});
    }
    else if constexpr (C == 4 && R == 3)
    {
      return Mat(ColumnType {m[0].x, m[1].x, m[2].x, m[3].x}, ColumnType {m[0].y, m[1].y, m[2].y, m[3].y},
                 ColumnType {m[0].z, m[1].z, m[2].z, m[3].z});
    }
    else if constexpr (C == 4 && R == 4)
    {
      return Mat(ColumnType {m[0].x, m[1].x, m[2].x, m[3].x}, ColumnType {m[0].y, m[1].y, m[2].y, m[3].y},
                 ColumnType {m[0].z, m[1].z, m[2].z, m[3].z}, ColumnType {m[0].w, m[1].w, m[2].w, m[3].w});
    }
  }

  /// @brief Computes the determinant of `m`. `m` must be a square matrix.
  template <MATRIX_TEMPLATE_PARAMS>
  KRYS_NODISCARD constexpr T Determinant(const MATRIX_TYPE &m) noexcept
  {
    MATRIX_STATIC_ASSERT(C == R);

    if constexpr (C == 2 && R == 2)
    {
      return m[0].x * m[1].y - m[0].y * m[1].x;
    }
    else if constexpr (C == 3 && R == 3)
    {
      return m[0].x * (m[1].y * m[2].z - m[2].y * m[1].z) - m[1].x * (m[0].y * m[2].z - m[2].y * m[0].z)
             + m[2].x * (m[0].y * m[1].z - m[0].z * m[1].y);
    }
    else if constexpr (C == 4 && R == 4)
    {
      return m[0].x
               * Determinant(Matrix<T, 3, 3> {
                 {m[1].y, m[1].z, m[1].w}, {m[2].y, m[2].z, m[2].w}, {m[3].y, m[3].z, m[3].w}})
             - m[1].x
                 * Determinant(Matrix<T, 3, 3> {
                   {m[0].y, m[0].z, m[0].w}, {m[2].y, m[2].z, m[2].w}, {m[3].y, m[3].z, m[3].w}})
             + m[2].x
                 * Determinant(Matrix<T, 3, 3> {
                   {m[0].y, m[0].z, m[0].w}, {m[1].y, m[1].z, m[1].w}, {m[3].y, m[3].z, m[3].w}})
             - m[3].x
                 * Determinant(Matrix<T, 3, 3> {
                   {m[0].y, m[0].z, m[0].w}, {m[1].y, m[1].z, m[1].w}, {m[2].y, m[2].z, m[2].w}});
    }
  }

  /// @brief Computes the inverse of `m`.
  template <MATRIX_TEMPLATE_PARAMS>
  KRYS_NODISCARD constexpr auto Inverse(const MATRIX_TYPE &m) noexcept
  {
    MATRIX_STATIC_ASSERT(R == C);

    if constexpr (C == 2 && R == 2)
    {
      T determinant = Determinant(m);
      if (determinant == T(0))
        determinant = T(1);

      T oneOverDeterminant = T(1) / determinant;
      return Matrix<T, 2, 2>(+m[1].y * oneOverDeterminant, -m[0].y * oneOverDeterminant,
                             -m[1].x * oneOverDeterminant, +m[0].x * oneOverDeterminant);
    }
    else if constexpr (C == 3 && R == 3)
    {
      T determinant = Determinant(m);
      if (determinant == T(0))
        determinant = T(1);
      T oneOverDet = T(1) / determinant;

      return Matrix<T, 3, 3>(+((m[1].y * m[2].z - m[2].y * m[1].z) * oneOverDet),
                             -((m[0].y * m[2].z - m[2].y * m[0].z) * oneOverDet),
                             +((m[0].y * m[1].z - m[1].y * m[0].z) * oneOverDet),

                             -((m[1].x * m[2].z - m[2].x * m[1].z) * oneOverDet),
                             +((m[0].x * m[2].z - m[2].x * m[0].z) * oneOverDet),
                             -((m[0].x * m[1].z - m[1].x * m[0].z) * oneOverDet),

                             +((m[1].x * m[2].y - m[2].x * m[1].y) * oneOverDet),
                             -((m[0].x * m[2].y - m[2].x * m[0].y) * oneOverDet),
                             +((m[0].x * m[1].y - m[1].x * m[0].y) * oneOverDet));
    }
    else if constexpr (C == 4 && R == 4)
    {
      T A2323 = m[2].z * m[3].w - m[2].w * m[3].z;
      T A1323 = m[2].y * m[3].w - m[2].w * m[3].y;
      T A1223 = m[2].y * m[3].z - m[2].z * m[3].y;
      T A0323 = m[2].x * m[3].w - m[2].w * m[3].x;
      T A0223 = m[2].x * m[3].z - m[2].z * m[3].x;
      T A0123 = m[2].x * m[3].y - m[2].y * m[3].x;
      T A2313 = m[1].z * m[3].w - m[1].w * m[3].z;
      T A1313 = m[1].y * m[3].w - m[1].w * m[3].y;
      T A1213 = m[1].y * m[3].z - m[1].z * m[3].y;
      T A2312 = m[1].z * m[2].w - m[1].w * m[2].z;
      T A1312 = m[1].y * m[2].w - m[1].w * m[2].y;
      T A1212 = m[1].y * m[2].z - m[1].z * m[2].y;
      T A0313 = m[1].x * m[3].w - m[1].w * m[3].x;
      T A0213 = m[1].x * m[3].z - m[1].z * m[3].x;
      T A0312 = m[1].x * m[2].w - m[1].w * m[2].x;
      T A0212 = m[1].x * m[2].z - m[1].z * m[2].x;
      T A0113 = m[1].x * m[3].y - m[1].y * m[3].x;
      T A0112 = m[1].x * m[2].y - m[1].y * m[2].x;

      T det = m[0].x * (m[1].y * A2323 - m[1].z * A1323 + m[1].w * A1223)
              - m[0].y * (m[1].x * A2323 - m[1].z * A0323 + m[1].w * A0223)
              + m[0].z * (m[1].x * A1323 - m[1].y * A0323 + m[1].w * A0123)
              - m[0].w * (m[1].x * A1223 - m[1].y * A0223 + m[1].z * A0123);

      if (det == T(0))
        det = 1;
      det = T(1) / det;

      return Matrix<T, 4, 4>({det * (m[1].y * A2323 - m[1].z * A1323 + m[1].w * A1223),
                              det * -(m[0].y * A2323 - m[0].z * A1323 + m[0].w * A1223),
                              det * (m[0].y * A2313 - m[0].z * A1313 + m[0].w * A1213),
                              det * -(m[0].y * A2312 - m[0].z * A1312 + m[0].w * A1212)},
                             {det * -(m[1].x * A2323 - m[1].z * A0323 + m[1].w * A0223),
                              det * (m[0].x * A2323 - m[0].z * A0323 + m[0].w * A0223),
                              det * -(m[0].x * A2313 - m[0].z * A0313 + m[0].w * A0213),
                              det * (m[0].x * A2312 - m[0].z * A0312 + m[0].w * A0212)},
                             {det * (m[1].x * A1323 - m[1].y * A0323 + m[1].w * A0123),
                              det * -(m[0].x * A1323 - m[0].y * A0323 + m[0].w * A0123),
                              det * (m[0].x * A1313 - m[0].y * A0313 + m[0].w * A0113),
                              det * -(m[0].x * A1312 - m[0].y * A0312 + m[0].w * A0112)},
                             {det * -(m[1].x * A1223 - m[1].y * A0223 + m[1].z * A0123),
                              det * (m[0].x * A1223 - m[0].y * A0223 + m[0].z * A0123),
                              det * -(m[0].x * A1213 - m[0].y * A0213 + m[0].z * A0113),
                              det * (m[0].x * A1212 - m[0].y * A0212 + m[0].z * A0112)});
    }
  }

  /// @brief Computes the outer product of two vectors, resulting in a matrix.
  template <MATRIX_TEMPLATE_PARAMS>
  constexpr auto OuterProduct(const Vector<T, R> &a, const Vector<T, C> &b) noexcept
  {
    Matrix<T, C, R> mat;
    for (int col = 0; col < C; ++col)
      for (int row = 0; row < R; ++row)
        mat[col][row] = a[row] * b[col];
    return mat;
  }

#undef MATRIX_TYPE_ALIASES
#undef MATRIX_INTROSPECTION
#undef MATRIX_BINARY_OPERATOR
#undef MATRIX_UNARY_OPERATOR
#undef MATRIX_OPERATOR_ASSIGNMENT
#undef MATRIX_COMMON_MEMBER_FUNCTIONS
}