#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Maths/Traits.hpp"
#include "Krystal.Maths/Vector.hpp"

#include <cassert>
#include <type_traits>

#pragma region Helper Macros

#define MATRIX_TEMPLATE_PARAMS Krys::Arithmetic T, int R, int C
#define MATRIX_TEMPLATE_ARGS T, R, C
#define MATRIX_TYPE Krys::Maths::Matrix<MATRIX_TEMPLATE_ARGS>

#define MATRIX_STATIC_ASSERT(condition) static_assert(condition, "Unsupported matrix length.")

#define MATRIX_VALID_LENGTH_CONDITION(R, C) (R > 1 && R < 5 && C > 1 && C < 5)

#define MATRIX_TYPE_ALIASES(Rows, Columns)                                                                   \
  template <Arithmetic T>                                                                                    \
  using Matrix##Rows##x##Columns = Matrix<T, Rows, Columns>;                                                 \
  using Mat##Rows##x##Columns = Matrix<float, Rows, Columns>;                                                \
  using Mat##Rows##x##Columns##b = Matrix<bool, Rows, Columns>;                                              \
  using Mat##Rows##x##Columns##f = Matrix<float, Rows, Columns>;                                             \
  using Mat##Rows##x##Columns##d = Matrix<double, Rows, Columns>;                                            \
  using Mat##Rows##x##Columns##i = Matrix<int, Rows, Columns>;                                               \
  using Mat##Rows##x##Columns##u = Matrix<unsigned int, Rows, Columns>;

#define MATRIX_INTROSPECTION(rows, columns)                                                                  \
  static constexpr int Rows = rows;                                                                          \
  static constexpr int Columns = columns;                                                                    \
  using Type = Matrix<T, Rows, Columns>;                                                                     \
  using ComponentType = T;                                                                                   \
  using RowType = Vector<T, Columns>;                                                                        \
  using ColumnType = Vector<T, Rows>;                                                                        \
                                                                                                             \
private:                                                                                                     \
  RowType _data[Rows];

#define MATRIX_BINARY_OPERATOR(OP)                                                                           \
  template <MATRIX_TEMPLATE_PARAMS>                                                                          \
  NO_DISCARD constexpr auto CONCAT(operator, OP)(const Matrix<T, R, C> &lhs,                                 \
                                                 const Matrix<T, R, C> &rhs) noexcept                        \
  {                                                                                                          \
    return Zip(lhs, rhs, [](auto l, auto r) { return l OP r; });                                             \
  }                                                                                                          \
  template <MATRIX_TEMPLATE_PARAMS>                                                                          \
  NO_DISCARD constexpr auto CONCAT(operator, OP)(const Matrix<T, R, C> &lhs, T scalar) noexcept              \
  {                                                                                                          \
    return MapEach(lhs, [&](auto v) { return v OP scalar; });                                                \
  }

#define MATRIX_OPERATOR_ASSIGNMENT(OP)                                                                       \
  constexpr auto &CONCAT(operator, CONCAT(OP, =))(const Matrix &other) noexcept                              \
  {                                                                                                          \
    *this = *this OP other;                                                                                  \
    return *this;                                                                                            \
  }                                                                                                          \
  constexpr auto &CONCAT(operator, CONCAT(OP, =))(T scalar) noexcept                                         \
  {                                                                                                          \
    *this = *this OP scalar;                                                                                 \
    return *this;                                                                                            \
  }

#define MATRIX_UNARY_OPERATOR(OP)                                                                            \
  template <MATRIX_TEMPLATE_PARAMS>                                                                          \
  NO_DISCARD constexpr auto CONCAT(operator, OP)(const Matrix<T, R, C> &rhs) noexcept                        \
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
  NO_DISCARD constexpr RowType &operator[](int row) noexcept                                                 \
  {                                                                                                          \
    assert(row >= 0 && row < Rows);                                                                          \
    return _data[row];                                                                                       \
  }                                                                                                          \
  NO_DISCARD constexpr RowType const &operator[](int row) const noexcept                                     \
  {                                                                                                          \
    assert(row >= 0 && row < Rows);                                                                          \
    return _data[row];                                                                                       \
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

  template <Arithmetic T>
  struct Matrix<T, 2, 2>
  {
    MATRIX_INTROSPECTION(2, 2)

  public:
    constexpr explicit Matrix(const T &value) noexcept : _data {RowType(value), RowType(value)}
    {
    }

    constexpr Matrix(const RowType &row1, const RowType &row2) noexcept : _data {row1, row2}
    {
    }

    constexpr Matrix(const T &m00, const T &m01, const T &m10, const T &m11) noexcept
        : _data {RowType(m00, m01), RowType(m10, m11)}
    {
    }

    MATRIX_COMMON_MEMBER_FUNCTIONS()
  };

  template <Arithmetic T>
  struct Matrix<T, 2, 3>
  {
    MATRIX_INTROSPECTION(2, 3)

  public:
    constexpr explicit Matrix(T value) noexcept : _data {RowType(value), RowType(value)}
    {
    }

    constexpr Matrix(const RowType &row1, const RowType &row2) noexcept : _data {row1, row2}
    {
    }

    constexpr Matrix(const T &m00, const T &m01, const T &m02, const T &m10, const T &m11,
                     const T &m12) noexcept
        : _data {RowType(m00, m01, m02), RowType(m10, m11, m12)}
    {
    }

    MATRIX_COMMON_MEMBER_FUNCTIONS()
  };

  template <Arithmetic T>
  struct Matrix<T, 2, 4>
  {
    MATRIX_INTROSPECTION(2, 4)

  public:
    constexpr explicit Matrix(T value) noexcept : _data {RowType(value), RowType(value)}
    {
    }

    constexpr Matrix(const RowType &row1, const RowType &row2) noexcept : _data {row1, row2}
    {
    }

    constexpr Matrix(const T &m00, const T &m01, const T &m02, const T &m03, const T &m10, const T &m11,
                     const T &m12, const T &m13) noexcept
        : _data {RowType(m00, m01, m02, m03), RowType(m10, m11, m12, m13)}
    {
    }

    MATRIX_COMMON_MEMBER_FUNCTIONS()
  };

  template <Arithmetic T>
  struct Matrix<T, 3, 2>
  {
    MATRIX_INTROSPECTION(3, 2)

  public:
    constexpr explicit Matrix(T value) noexcept : _data {RowType(value), RowType(value), RowType(value)}
    {
    }

    constexpr Matrix(const RowType &row1, const RowType &row2, const RowType &row3) noexcept
        : _data {row1, row2, row3}
    {
    }

    constexpr Matrix(const T &m00, const T &m01, const T &m10, const T &m11, const T &m20,
                     const T &m21) noexcept
        : _data {RowType(m00, m01), RowType(m10, m11), RowType(m20, m21)}
    {
    }

    MATRIX_COMMON_MEMBER_FUNCTIONS()
  };

  template <Arithmetic T>
  struct Matrix<T, 3, 3>
  {
    MATRIX_INTROSPECTION(3, 3)

  public:
    constexpr explicit Matrix(T value) noexcept : _data {RowType(value), RowType(value), RowType(value)}
    {
    }

    constexpr Matrix(const RowType &row1, const RowType &row2, const RowType &row3) noexcept
        : _data {row1, row2, row3}
    {
    }

    constexpr Matrix(const T &m00, const T &m01, const T &m02, const T &m10, const T &m11, const T &m12,
                     const T &m20, const T &m21, const T &m22) noexcept
        : _data {RowType(m00, m01, m02), RowType(m10, m11, m12), RowType(m20, m21, m22)}
    {
    }

    MATRIX_COMMON_MEMBER_FUNCTIONS()
  };

  template <Arithmetic T>
  struct Matrix<T, 3, 4>
  {
    MATRIX_INTROSPECTION(3, 4)

  public:
    constexpr explicit Matrix(T value) noexcept : _data {RowType(value), RowType(value), RowType(value)}
    {
    }

    constexpr Matrix(const RowType &row1, const RowType &row2, const RowType &row3) noexcept
        : _data {row1, row2, row3}
    {
    }

    constexpr Matrix(const T &m00, const T &m01, const T &m02, const T &m03, const T &m10, const T &m11,
                     const T &m12, const T &m13, const T &m20, const T &m21, const T &m22,
                     const T &m23) noexcept
        : _data {RowType(m00, m01, m02, m03), RowType(m10, m11, m12, m13), RowType(m20, m21, m22, m23)}
    {
    }

    MATRIX_COMMON_MEMBER_FUNCTIONS()
  };

  template <Arithmetic T>
  struct Matrix<T, 4, 2>
  {
    MATRIX_INTROSPECTION(4, 2)

  public:
    constexpr explicit Matrix(T value) noexcept
        : _data {RowType(value), RowType(value), RowType(value), RowType(value)}
    {
    }

    constexpr Matrix(const RowType &row1, const RowType &row2, const RowType &row3,
                     const RowType &row4) noexcept
        : _data {row1, row2, row3, row4}
    {
    }

    constexpr Matrix(const T &m00, const T &m01, const T &m10, const T &m11, const T &m20, const T &m21,
                     const T &m30, const T &m31) noexcept
        : _data {RowType(m00, m01), RowType(m10, m11), RowType(m20, m21), RowType(m30, m31)}
    {
    }

    MATRIX_COMMON_MEMBER_FUNCTIONS()
  };

  template <Arithmetic T>
  struct Matrix<T, 4, 3>
  {
    MATRIX_INTROSPECTION(4, 3)

  public:
    constexpr explicit Matrix(T value) noexcept
        : _data {RowType(value), RowType(value), RowType(value), RowType(value)}
    {
    }

    constexpr Matrix(const RowType &row1, const RowType &row2, const RowType &row3,
                     const RowType &row4) noexcept
        : _data {row1, row2, row3, row4}
    {
    }

    constexpr Matrix(const T &m00, const T &m01, const T &m02, const T &m10, const T &m11, const T &m12,
                     const T &m20, const T &m21, const T &m22, const T &m30, const T &m31,
                     const T &m32) noexcept
        : _data {RowType(m00, m01, m02), RowType(m10, m11, m12), RowType(m20, m21, m22),
                 RowType(m30, m31, m32)}
    {
    }

    MATRIX_COMMON_MEMBER_FUNCTIONS()
  };

  template <Arithmetic T>
  struct Matrix<T, 4, 4>
  {
    MATRIX_INTROSPECTION(4, 4)

  public:
    constexpr explicit Matrix(T value) noexcept
        : _data {RowType(value), RowType(value), RowType(value), RowType(value)}
    {
    }

    constexpr Matrix(const RowType &row1, const RowType &row2, const RowType &row3,
                     const RowType &row4) noexcept
        : _data {row1, row2, row3, row4}
    {
    }

    constexpr Matrix(const T &m00, const T &m01, const T &m02, const T &m03, const T &m10, const T &m11,
                     const T &m12, const T &m13, const T &m20, const T &m21, const T &m22, const T &m23,
                     const T &m30, const T &m31, const T &m32, const T &m33) noexcept
        : _data {RowType(m00, m01, m02, m03), RowType(m10, m11, m12, m13), RowType(m20, m21, m22, m23),
                 RowType(m30, m31, m32, m33)}
    {
    }

    MATRIX_COMMON_MEMBER_FUNCTIONS()
  };

  template <MATRIX_TEMPLATE_PARAMS>
  struct Traits<MATRIX_TYPE>
  {
    using ComponentType = T;
    static constexpr bool IsArithmetic = true;
    static constexpr bool IsVector = false;
    static constexpr bool IsMatrix = true;

    static constexpr int TotalComponents = R * C;
    static constexpr int Rows = R;
    static constexpr int Columns = C;
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
  NO_DISCARD constexpr auto Identity() noexcept
  {
    constexpr int R = Traits<Mat>::Rows;
    constexpr int C = Traits<Mat>::Columns;
    using T = typename Traits<Mat>::ComponentType;

    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(R, C));
    MATRIX_STATIC_ASSERT(R == C);

    if constexpr (R == 2 && C == 2)
    {
      return Mat(T(1), T(0), T(0), T(1));
    }
    else if constexpr (R == 3 && C == 3)
    {
      return Mat(T(1), T(0), T(0), T(0), T(1), T(0), T(0), T(0), T(1));
    }
    else if constexpr (R == 4 && C == 4)
    {
      return Mat(T(1), T(0), T(0), T(0), T(0), T(1), T(0), T(0), T(0), T(0), T(1), T(0), T(0), T(0), T(0),
                 T(1));
    }
  }

  /// @brief Map each component of the matrix using the provided function.
  template <MATRIX_TEMPLATE_PARAMS, RegularCallable<T> F>
  NO_DISCARD constexpr auto MapEach(const Matrix<T, R, C> &mat, const F &func) noexcept
  {
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(R, C));

    using U = std::invoke_result_t<F, T>;
    using Mat = Matrix<U, R, C>;

    if constexpr (R == 2)
    {
      auto row0 = MapEach(mat[0], func);
      auto row1 = MapEach(mat[1], func);
      return Mat(row0, row1);
    }
    else if constexpr (R == 3)
    {
      auto row0 = MapEach(mat[0], func);
      auto row1 = MapEach(mat[1], func);
      auto row2 = MapEach(mat[2], func);
      return Mat(row0, row1, row2);
    }
    else if constexpr (R == 4)
    {
      auto row0 = MapEach(mat[0], func);
      auto row1 = MapEach(mat[1], func);
      auto row2 = MapEach(mat[2], func);
      auto row3 = MapEach(mat[3], func);
      return Mat(row0, row1, row2, row3);
    }
    else
    {
      MATRIX_STATIC_ASSERT(false);
    }
  }

  /// @brief Apply a function to each component of the matrix.
  template <MATRIX_TEMPLATE_PARAMS, RegularCallable<T> F>
  NO_DISCARD constexpr void ForEach(const Matrix<T, R, C> &mat, const F &func) noexcept
  {
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(R, C));

    if constexpr (R == 2)
    {
      ForEach(mat[0], func);
      ForEach(mat[1], func);
    }
    else if constexpr (R == 3)
    {
      ForEach(mat[0], func);
      ForEach(mat[1], func);
      ForEach(mat[2], func);
    }
    else if constexpr (R == 4)
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
  NO_DISCARD constexpr auto Zip(const Matrix<T, R, C> &lhs, const Matrix<T, R, C> &rhs,
                                const F &func) noexcept
  {
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(R, C));

    using U = std::invoke_result_t<F, T, T>;
    using Mat = Matrix<U, R, C>;

    if constexpr (R == 2)
    {
      auto row0 = Zip(lhs[0], rhs[0], func);
      auto row1 = Zip(lhs[1], rhs[1], func);
      return Mat(row0, row1);
    }
    else if constexpr (R == 3)
    {
      auto row0 = Zip(lhs[0], rhs[0], func);
      auto row1 = Zip(lhs[1], rhs[1], func);
      auto row2 = Zip(lhs[2], rhs[2], func);
      return Mat(row0, row1, row2);
    }
    else if constexpr (R == 4)
    {
      auto row0 = Zip(lhs[0], rhs[0], func);
      auto row1 = Zip(lhs[1], rhs[1], func);
      auto row2 = Zip(lhs[2], rhs[2], func);
      auto row3 = Zip(lhs[3], rhs[3], func);
      return Mat(row0, row1, row2, row3);
    }
    else
    {
      MATRIX_STATIC_ASSERT(false);
    }
  }

  /// @brief Zip three matrices together using the provided function.
  template <MATRIX_TEMPLATE_PARAMS, RegularCallable<T, T, T> F>
  NO_DISCARD constexpr auto Zip(const Matrix<T, R, C> &a, const Matrix<T, R, C> &b, const Matrix<T, R, C> &c,
                                const F &func) noexcept
  {
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(R, C));

    using U = std::invoke_result_t<F, T, T, T>;
    using Mat = Matrix<U, R, C>;

    if constexpr (R == 2)
    {
      auto row0 = Zip(a[0], b[0], c[0], func);
      auto row1 = Zip(a[1], b[1], c[1], func);
      return Mat(row0, row1);
    }
    else if constexpr (R == 3)
    {
      auto row0 = Zip(a[0], b[0], c[0], func);
      auto row1 = Zip(a[1], b[1], c[1], func);
      auto row2 = Zip(a[2], b[2], c[2], func);
      return Mat(row0, row1, row2);
    }
    else if constexpr (R == 4)
    {
      auto row0 = Zip(a[0], b[0], c[0], func);
      auto row1 = Zip(a[1], b[1], c[1], func);
      auto row2 = Zip(a[2], b[2], c[2], func);
      auto row3 = Zip(a[3], b[3], c[3], func);
      return Mat(row0, row1, row2, row3);
    }
    else
    {
      MATRIX_STATIC_ASSERT(false);
    }
  }

  /// @brief Computes the sum of all components of the matrix.
  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr T Sum(const MATRIX_TYPE &m) noexcept
  {
    T sum = T(0);
    ForEach(m, [&sum](auto val) { sum += val; });
    return sum;
  }

  /// @brief Computes the sum of all components of the matrix after applying a function to each component.
  template <MATRIX_TEMPLATE_PARAMS, RegularCallable<T> F>
  NO_DISCARD constexpr auto Sum(const MATRIX_TYPE &m, const F &func) noexcept
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
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(R, C));

    if constexpr (R == 2 && C == 2)
    {
      return predicate(v[0].x) || predicate(v[0].y) || predicate(v[1].x) || predicate(v[1].y);
    }
    else if constexpr (R == 2 && C == 3)
    {
      return predicate(v[0].x) || predicate(v[0].y) || predicate(v[0].z) || predicate(v[1].x)
             || predicate(v[1].y) || predicate(v[1].z);
    }
    else if constexpr (R == 2 && C == 4)
    {
      return predicate(v[0].x) || predicate(v[0].y) || predicate(v[0].z) || predicate(v[0].w)
             || predicate(v[1].x) || predicate(v[1].y) || predicate(v[1].z) || predicate(v[1].w);
    }
    else if constexpr (R == 3 && C == 2)
    {
      return predicate(v[0].x) || predicate(v[0].y) || predicate(v[1].x) || predicate(v[1].y)
             || predicate(v[2].x) || predicate(v[2].y);
    }
    else if constexpr (R == 3 && C == 3)
    {
      return predicate(v[0].x) || predicate(v[0].y) || predicate(v[0].z) || predicate(v[1].x)
             || predicate(v[1].y) || predicate(v[1].z) || predicate(v[2].x) || predicate(v[2].y)
             || predicate(v[2].z);
    }
    else if constexpr (R == 3 && C == 4)
    {
      return predicate(v[0].x) || predicate(v[0].y) || predicate(v[0].z) || predicate(v[0].w)
             || predicate(v[1].x) || predicate(v[1].y) || predicate(v[1].z) || predicate(v[1].w)
             || predicate(v[2].x) || predicate(v[2].y) || predicate(v[2].z) || predicate(v[2].w);
    }
    else if constexpr (R == 4 && C == 2)
    {
      return predicate(v[0].x) || predicate(v[0].y) || predicate(v[1].x) || predicate(v[1].y)
             || predicate(v[2].x) || predicate(v[2].y) || predicate(v[3].x) || predicate(v[3].y);
    }
    else if constexpr (R == 4 && C == 3)
    {
      return predicate(v[0].x) || predicate(v[0].y) || predicate(v[0].z) || predicate(v[1].x)
             || predicate(v[1].y) || predicate(v[1].z) || predicate(v[2].x) || predicate(v[2].y)
             || predicate(v[2].z) || predicate(v[3].x) || predicate(v[3].y) || predicate(v[3].z);
    }
    else if constexpr (R == 4 && C == 4)
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
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(R, C));

    if constexpr (R == 2 && C == 2)
    {
      return predicate(v[0].x) && predicate(v[0].y) && predicate(v[1].x) && predicate(v[1].y);
    }
    else if constexpr (R == 2 && C == 3)
    {
      return predicate(v[0].x) && predicate(v[0].y) && predicate(v[0].z) && predicate(v[1].x)
             && predicate(v[1].y) && predicate(v[1].z);
    }
    else if constexpr (R == 2 && C == 4)
    {
      return predicate(v[0].x) && predicate(v[0].y) && predicate(v[0].z) && predicate(v[0].w)
             && predicate(v[1].x) && predicate(v[1].y) && predicate(v[1].z) && predicate(v[1].w);
    }
    else if constexpr (R == 3 && C == 2)
    {
      return predicate(v[0].x) && predicate(v[0].y) && predicate(v[1].x) && predicate(v[1].y)
             && predicate(v[2].x) && predicate(v[2].y);
    }
    else if constexpr (R == 3 && C == 3)
    {
      return predicate(v[0].x) && predicate(v[0].y) && predicate(v[0].z) && predicate(v[1].x)
             && predicate(v[1].y) && predicate(v[1].z) && predicate(v[2].x) && predicate(v[2].y)
             && predicate(v[2].z);
    }
    else if constexpr (R == 3 && C == 4)
    {
      return predicate(v[0].x) && predicate(v[0].y) && predicate(v[0].z) && predicate(v[0].w)
             && predicate(v[1].x) && predicate(v[1].y) && predicate(v[1].z) && predicate(v[1].w)
             && predicate(v[2].x) && predicate(v[2].y) && predicate(v[2].z) && predicate(v[2].w);
    }
    else if constexpr (R == 4 && C == 2)
    {
      return predicate(v[0].x) && predicate(v[0].y) && predicate(v[1].x) && predicate(v[1].y)
             && predicate(v[2].x) && predicate(v[2].y) && predicate(v[3].x) && predicate(v[3].y);
    }
    else if constexpr (R == 4 && C == 3)
    {
      return predicate(v[0].x) && predicate(v[0].y) && predicate(v[0].z) && predicate(v[1].x)
             && predicate(v[1].y) && predicate(v[1].z) && predicate(v[2].x) && predicate(v[2].y)
             && predicate(v[2].z) && predicate(v[3].x) && predicate(v[3].y) && predicate(v[3].z);
    }
    else if constexpr (R == 4 && C == 4)
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
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(R, C));

    if constexpr (R == 2 && C == 2)
    {
      return !predicate(v[0].x) && !predicate(v[0].y) && !predicate(v[1].x) && !predicate(v[1].y);
    }
    else if constexpr (R == 2 && C == 3)
    {
      return !predicate(v[0].x) && !predicate(v[0].y) && !predicate(v[0].z) && !predicate(v[1].x)
             && !predicate(v[1].y) && !predicate(v[1].z);
    }
    else if constexpr (R == 2 && C == 4)
    {
      return !predicate(v[0].x) && !predicate(v[0].y) && !predicate(v[0].z) && !predicate(v[0].w)
             && !predicate(v[1].x) && !predicate(v[1].y) && !predicate(v[1].z) && !predicate(v[1].w);
    }
    else if constexpr (R == 3 && C == 2)
    {
      return !predicate(v[0].x) && !predicate(v[0].y) && !predicate(v[1].x) && !predicate(v[1].y)
             && !predicate(v[2].x) && !predicate(v[2].y);
    }
    else if constexpr (R == 3 && C == 3)
    {
      return !predicate(v[0].x) && !predicate(v[0].y) && !predicate(v[0].z) && !predicate(v[1].x)
             && !predicate(v[1].y) && !predicate(v[1].z) && !predicate(v[2].x) && !predicate(v[2].y)
             && !predicate(v[2].z);
    }
    else if constexpr (R == 3 && C == 4)
    {
      return !predicate(v[0].x) && !predicate(v[0].y) && !predicate(v[0].z) && !predicate(v[0].w)
             && !predicate(v[1].x) && !predicate(v[1].y) && !predicate(v[1].z) && !predicate(v[1].w)
             && !predicate(v[2].x) && !predicate(v[2].y) && !predicate(v[2].z) && !predicate(v[2].w);
    }
    else if constexpr (R == 4 && C == 2)
    {
      return !predicate(v[0].x) && !predicate(v[0].y) && !predicate(v[1].x) && !predicate(v[1].y)
             && !predicate(v[2].x) && !predicate(v[2].y) && !predicate(v[3].x) && !predicate(v[3].y);
    }
    else if constexpr (R == 4 && C == 3)
    {
      return !predicate(v[0].x) && !predicate(v[0].y) && !predicate(v[0].z) && !predicate(v[1].x)
             && !predicate(v[1].y) && !predicate(v[1].z) && !predicate(v[2].x) && !predicate(v[2].y)
             && !predicate(v[2].z) && !predicate(v[3].x) && !predicate(v[3].y) && !predicate(v[3].z);
    }
    else if constexpr (R == 4 && C == 4)
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
  NO_DISCARD constexpr auto operator+(T scalar, const MATRIX_TYPE &mat) noexcept
  {
    return mat + scalar;
  }

  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto operator*(T scalar, const MATRIX_TYPE &mat) noexcept
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
  constexpr auto operator*(const Matrix<T, R, C> &lhs, const Matrix<T, C, P> &rhs) noexcept
  {
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(R, C));
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(C, P));

    Matrix<T, R, P> out {T(0)};

    for (int i = 0; i < R; ++i)     // row of lhs
      for (int j = 0; j < P; ++j)   // col of rhs
        for (int k = 0; k < C; ++k) // dot-product
          out[i][j] += lhs[i][k] * rhs[k][j];

    return out;
  }

  /// @brief Matrix multiplication with a vector.
  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto operator*(const MATRIX_TYPE &lhs, const Vector<T, C> &rhs) noexcept
  {
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(R, C));
    Vector<T, R> result {T(0)};
    for (int i = 0; i < R; ++i)
      for (int j = 0; j < C; ++j)
        result[i] += lhs[i][j] * rhs[j];
    return result;
  }

  /// @brief Component-wise multiplication (Hadamard product) of two matrices.
  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Hadamard(const MATRIX_TYPE &lhs, const MATRIX_TYPE &rhs) noexcept
  {
    return Zip(lhs, rhs, [](auto x, auto y) { return x * y; });
  }

  /// @brief Computes the transpose of `m`.
  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Transpose(const MATRIX_TYPE &m) noexcept
  {
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(R, C));

    using Mat = Matrix<T, C, R>;
    using RowType = Mat::RowType;

    if constexpr (R == 2 && C == 2)
    {
      return Mat(RowType {m[0].x, m[1].x}, RowType {m[0].y, m[1].y});
    }
    else if constexpr (R == 2 && C == 3)
    {
      return Mat(RowType {m[0].x, m[1].x}, RowType {m[0].y, m[1].y}, RowType {m[0].z, m[1].z});
    }
    else if constexpr (R == 2 && C == 4)
    {
      return Mat(RowType {m[0].x, m[1].x}, RowType {m[0].y, m[1].y}, RowType {m[0].z, m[1].z},
                 RowType {m[0].w, m[0].w});
    }
    else if constexpr (R == 3 && C == 2)
    {
      return Mat(RowType {m[0].x, m[1].x, m[2].x}, RowType {m[0].y, m[1].y, m[2].y});
    }
    else if constexpr (R == 3 && C == 3)
    {
      return Mat(RowType {m[0].x, m[1].x, m[2].x}, RowType {m[0].y, m[1].y, m[2].y},
                 RowType {m[0].z, m[1].z, m[2].z});
    }
    else if constexpr (R == 3 && C == 4)
    {
      return Mat(RowType {m[0].x, m[1].x, m[2].x}, RowType {m[0].y, m[1].y, m[2].y},
                 RowType {m[0].z, m[1].z, m[2].z}, RowType {m[0].w, m[1].w, m[2].w});
    }
    else if constexpr (R == 4 && C == 2)
    {
      return Mat(RowType {m[0].x, m[1].x, m[2].x, m[3].x}, RowType {m[0].y, m[1].y, m[2].y, m[3].y});
    }
    else if constexpr (R == 4 && C == 3)
    {
      return Mat(RowType {m[0].x, m[1].x, m[2].x, m[3].x}, RowType {m[0].y, m[1].y, m[2].y, m[3].y},
                 RowType {m[0].z, m[1].z, m[2].z, m[3].z});
    }
    else if constexpr (R == 4 && C == 4)
    {
      return Mat(RowType {m[0].x, m[1].x, m[2].x, m[3].x}, RowType {m[0].y, m[1].y, m[2].y, m[3].y},
                 RowType {m[0].z, m[1].z, m[2].z, m[3].z}, RowType {m[0].w, m[1].w, m[2].w, m[3].w});
    }
  }

  /// @brief Computes the determinant of `m`. `m` must be a square matrix.
  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr T Determinant(const MATRIX_TYPE &m) noexcept
  {
    MATRIX_STATIC_ASSERT(R == C);

    if constexpr (R == 2 && C == 2)
    {
      return m[0].x * m[1].y - m[0].y * m[1].x;
    }
    else if constexpr (R == 3 && C == 3)
    {
      return m[0].x * (m[1].y * m[2].z - m[2].y * m[1].z) - m[1].x * (m[0].y * m[2].z - m[2].y * m[0].z)
             + m[2].x * (m[0].y * m[1].z - m[0].z * m[1].y);
    }
    else if constexpr (R == 4 && C == 4)
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
  NO_DISCARD constexpr auto Inverse(const MATRIX_TYPE &m) noexcept
  {
    MATRIX_STATIC_ASSERT(R == C);

    if constexpr (R == 2 && C == 2)
    {
      T determinant = Determinant(m);
      if (determinant == T(0))
        determinant = T(1);

      T oneOverDeterminant = T(1) / determinant;
      return Matrix<T, 2, 2>(+m[1].y * oneOverDeterminant, -m[0].y * oneOverDeterminant,
                             -m[1].x * oneOverDeterminant, +m[0].x * oneOverDeterminant);
    }
    else if constexpr (R == 3 && C == 3)
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
    else if constexpr (R == 4 && C == 4)
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
  NO_DISCARD constexpr auto OuterProduct(const Vector<T, R> &a, const Vector<T, C> &b) noexcept
  {
    MATRIX_STATIC_ASSERT(MATRIX_VALID_LENGTH_CONDITION(R, C));

    MATRIX_TYPE mat;
    for (int i = 0; i < R; ++i)
    {
      for (int j = 0; j < C; ++j)
      {
        mat[i][j] = a[i] * b[j];
      }
    }

    return mat;
  }

#undef MATRIX_TYPE_ALIASES
#undef MATRIX_INTROSPECTION
#undef MATRIX_BINARY_OPERATOR
#undef MATRIX_UNARY_OPERATOR
#undef MATRIX_OPERATOR_ASSIGNMENT
#undef MATRIX_COMMON_MEMBER_FUNCTIONS
}